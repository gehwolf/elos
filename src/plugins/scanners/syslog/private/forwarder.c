#include "forwarder.h"

#include <safu/common.h>
#include <safu/log.h>
#include <safu/result.h>
#include <samconf/samconf_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

static void _connectForward(elosForward_t *forward) {
    safuLogDebugF("Setup forward for %s", forward->addr.sun_path);
    forward->fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (forward->fd == -1) {
        safuLogWarnF("failed to setup forward for %s", forward->addr.sun_path);
    } else {
        int retVal = connect(forward->fd, (const struct sockaddr *)&forward->addr, sizeof(forward->addr));
        if (retVal == -1) {
            safuLogWarnF("failed to connect forward for %s", forward->addr.sun_path);
            forward->state = ELOS_FORWARD_NOT_CONNECTED;
        } else {
            forward->state = ELOS_FORWARD_CONNECTED;
            safuLogDebugF("connected forward for %s", forward->addr.sun_path);
        }
    }
}

safuResultE_t elosSyslogForwarderInitialze(elosForwarder_t *forwarder, samconfConfig_t const *forwards) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    forwarder->forwardsCount = 0;
    forwarder->forwards = NULL;

    if (forwards != NULL && forwards->type == SAMCONF_CONFIG_VALUE_ARRAY && forwards->childCount > 0) {
        forwarder->forwards = safuAllocMem(NULL, sizeof(elosForward_t) * forwards->childCount);
        if (forwarder->forwards == NULL) {
            safuLogErr("Failed to allocate forwards list");
        } else {
            forwarder->forwardsCount = forwards->childCount;
            memset(forwarder->forwards, 0, sizeof(elosForward_t) * forwarder->forwardsCount);
            result = SAFU_RESULT_OK;

            for (size_t i = 0; i < forwards->childCount; i++) {
                elosForward_t *forward = &forwarder->forwards[i];
                forward->state = ELOS_FORWARD_NOT_INITIALIZED;
                if (forwards->children[i]->type == SAMCONF_CONFIG_VALUE_STRING) {
                    const char *forwardAddr = forwards->children[i]->value.string;
                    forward->addr.sun_family = AF_UNIX;
                    strncpy(forward->addr.sun_path, forwardAddr, sizeof(forward->addr.sun_path) - 1);
                    _connectForward(forward);
                } else {
                    safuLogWarnF("Invalid value for forward %zu", i);
                }
            }
        }
    }

    return result;
}

safuResultE_t elosSyslogForwarderForwardMessage(elosForwarder_t *forwarder, char const *buffer, size_t length) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    for (size_t i = 0; i < forwarder->forwardsCount; i++) {
        safuLogDebugF("send forward %zu", i);
        elosForward_t *forward = &forwarder->forwards[i];
        if (forward->state == ELOS_FORWARD_NOT_INITIALIZED) {
            safuLogDebugF("Skip forward to %s", forward->addr.sun_path);
            continue;
        }
        if (forward->state == ELOS_FORWARD_NOT_CONNECTED) {
            safuLogDebugF(" reconnect to %s", forward->addr.sun_path);
            _connectForward(forward);
        }
        if (forward->state == ELOS_FORWARD_CONNECTED) {
            size_t transferred = 0;
            result = safuSendExactly(forward->fd, buffer, length, &transferred);
            if (result != SAFU_RESULT_OK) {
                safuLogErrF("failed to forward message, transferred bytes was %zu", transferred);
                forward->state = ELOS_FORWARD_NOT_CONNECTED;
            } else {
                result = SAFU_RESULT_OK;
            }
        }
    }

    return result;
}

safuResultE_t elosSyslogForwarderDeleteMembers(elosForwarder_t *forwarder) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (forwarder != NULL) {
        for (size_t i = 0; i < forwarder->forwardsCount; i++) {
            elosForward_t *forward = &forwarder->forwards[i];
            if (forward->state == ELOS_FORWARD_CONNECTED) {
                shutdown(forward->fd, SHUT_RDWR);
                close(forward->fd);
                forward->state = ELOS_FORWARD_NOT_INITIALIZED;
            }
        }
        free(forwarder->forwards);
        result = SAFU_RESULT_OK;
    }

    return result;
}

#pragma once

#include <safu/result.h>
#include <samconf/samconf_types.h>
#include <sys/un.h>

typedef enum elosForwardE {
    ELOS_FORWARD_NOT_INITIALIZED = 0,
    ELOS_FORWARD_NOT_CONNECTED,
    ELOS_FORWARD_CONNECTED,
} elosForwardE_t;

typedef struct elosForward {
    struct sockaddr_un addr;
    int fd;
    elosForwardE_t state;
} elosForward_t;

typedef struct elosForwarder {
    size_t forwardsCount;
    elosForward_t *forwards;
} elosForwarder_t;

safuResultE_t elosSyslogForwarderInitialze(elosForwarder_t *forwarder, samconfConfig_t const *forwards);
safuResultE_t elosSyslogForwarderForwardMessage(elosForwarder_t *forwarder, char const *buffer, size_t length);
safuResultE_t elosSyslogForwarderDeleteMembers(elosForwarder_t *forwarder);

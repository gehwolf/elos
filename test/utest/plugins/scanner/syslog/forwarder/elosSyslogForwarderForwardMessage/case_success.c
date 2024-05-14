// SPDX-License-Identifier: MIT
#include <cmocka_extensions/cmocka_extensions.h>
#include <safu/common.h>
#include <safu/result.h>
#include <samconf/samconf_types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "elosSyslogForwarderForwardMessage_utest.h"
#include "forwarder.h"

int elosTestElosSyslogForwarderForwardMessageSuccessSetup(void **state) {
    unlink("/tmp/forward1.sock");
    unlink("/tmp/forward2.sock");
    unlink("/tmp/forward3.sock");
    return 0;
}

int elosTestElosSyslogForwarderForwardMessageSuccessTeardown(void **state) {
    unlink("/tmp/forward1.sock");
    unlink("/tmp/forward2.sock");
    unlink("/tmp/forward3.sock");
    return 0;
}

int _createForwardSocket(const char *socketPath) {
    int fd = -1;
    struct sockaddr_un name = {.sun_family = AF_UNIX};

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    assert_int_not_equal(fd, -1);

    strncpy(name.sun_path, socketPath, strlen(socketPath));
    int result = bind(fd, (const struct sockaddr *)&name, sizeof(name));
    assert_int_not_equal(result, -1);

    return fd;
}

void elosTestElosSyslogForwarderForwardMessageSuccess(void **state) {
    int forwardTargetFds[3] = {0};
    elosForwarder_t forwarder = {0};

    samconfConfig_t forward3 = {.type = SAMCONF_CONFIG_VALUE_STRING, .value.string = "/tmp/forward3.sock"};
    samconfConfig_t forward1 = {.type = SAMCONF_CONFIG_VALUE_STRING, .value.string = "/tmp/forward1.sock"};
    samconfConfig_t forward2 = {.type = SAMCONF_CONFIG_VALUE_STRING, .value.string = "/tmp/forward2.sock"};
    samconfConfig_t *forwards[] = {
        &forward1,
        &forward2,
        &forward3,
    };
    samconfConfig_t forwardConfig = {
        .type = SAMCONF_CONFIG_VALUE_ARRAY,
        .key = "forwards",
        .children = forwards,
        .childCount = ARRAY_SIZE(forwards),
    };
    char const message[] = "Hugo hat husten";
    char forwardedMessage[ARRAY_SIZE(message)] = {0};

    forwardTargetFds[0] = _createForwardSocket(forward1.value.string);
    forwardTargetFds[1] = _createForwardSocket(forward2.value.string);
    forwardTargetFds[2] = _createForwardSocket(forward3.value.string);

    safuResultE_t status = elosSyslogForwarderInitialze(&forwarder, &forwardConfig);
    assert_int_equal(SAFU_RESULT_OK, status);
    assert_int_equal(forwarder.forwardsCount, 3);
    assert_non_null_msg(forwarder.forwards, "forwards shall be a allocated list");

    status = elosSyslogForwarderForwardMessage(&forwarder, message, strlen(message));
    assert_int_equal(SAFU_RESULT_OK, status);

    for (size_t i = 0; i < ARRAY_SIZE(forwardTargetFds); i++) {
        size_t received = 0;
        status = safuRecvExactly(forwardTargetFds[i], forwardedMessage, strlen(message), &received);
        assert_int_equal(SAFU_RESULT_OK, status);
        assert_int_equal(strlen(message), received);
        assert_string_equal(message, forwardedMessage);
    }

    status = elosSyslogForwarderDeleteMembers(&forwarder);
    assert_int_equal(SAFU_RESULT_OK, status);
}

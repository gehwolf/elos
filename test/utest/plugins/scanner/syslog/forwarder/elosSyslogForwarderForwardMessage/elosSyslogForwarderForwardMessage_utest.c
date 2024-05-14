// SPDX-License-Identifier: MIT
#include "elosSyslogForwarderForwardMessage_utest.h"
#include <safu/common.h>

TEST_SUITE_FUNC_PROTOTYPES(elosSyslogForwarderForwardMessageUtest)

int main(void) {
    const struct CMUnitTest tests[] = {
        TEST_CASE(elosTestElosSyslogForwarderForwardMessageSuccess),
    };

    return RUN_TEST_SUITE(tests, elosSyslogForwarderForwardMessageUtest);
}

int elosSyslogForwarderForwardMessageUtestInit(void **state) {
    elosTestState_t *testState = safuAllocMem(NULL, sizeof(*testState));

    *state = testState;

    return 0;
}

int elosSyslogForwarderForwardMessageUtestCleanUp(void **state) {
    free(*state);
    return 0;
}

static int elosSyslogForwarderForwardMessageUtestSetup(UNUSED void **state) {
    return 0;
}

static int elosSyslogForwarderForwardMessageUtestTeardown(UNUSED void **state) {
    return 0;
}

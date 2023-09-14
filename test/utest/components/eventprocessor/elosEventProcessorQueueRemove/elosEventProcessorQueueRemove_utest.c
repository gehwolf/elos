// SPDX-License-Identifier: MIT
#include "elosEventProcessorQueueRemove_utest.h"

TEST_SUITE_FUNC_PROTOTYPES(_testSuite)

int main() {
    const struct CMUnitTest tests[] = {
        TEST_CASE(elosTestEloEventProcessorQueueRemoveSuccess),
        TEST_CASE(elosTestEloEventProcessorQueueRemoveErrParam),
        TEST_CASE(elosTestEloEventProcessorQueueRemoveDeleteFailed),
    };

    return RUN_TEST_SUITE(tests, _testSuite);
}

static int _testSuiteSetup(void **state) {
    *state = safuAllocMem(NULL, sizeof(elosUnitTestState_t));
    return 0;
}

static int _testSuiteTeardown(void **state) {
    free(*state);
    return 0;
}

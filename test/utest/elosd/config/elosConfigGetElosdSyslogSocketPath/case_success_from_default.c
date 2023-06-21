// SPDX-License-Identifier: MIT

#include "elosConfigGetElosdSyslogSocketPath_utest.h"

int elosTestElosConfigGetElosdSyslogSocketPathSuccessFromDefaultSetup(UNUSED void **state) {
    return 0;
}

int elosTestElosConfigGetElosdSyslogSocketPathSuccessFromDefaultTeardown(UNUSED void **state) {
    return 0;
}

void elosTestElosConfigGetElosdSyslogSocketPathSuccessFromDefault(UNUSED void **state) {
    const char *expectedValue = "/dev/log";
    samconfConfig_t mockConfig = elosGetMockConfig();

    TEST("elosConfigGetElosdSyslogSocketPath");
    SHOULD("%s", "get the elos syslog socket path default option");

    MOCK_FUNC_AFTER_CALL(samconfConfigGetBool, 0);
    expect_value(__wrap_samconfConfigGetBool, root, &mockConfig);
    expect_string(__wrap_samconfConfigGetBool, path, ELOS_CONFIG_ROOT "UseEnv");
    expect_any(__wrap_samconfConfigGetBool, result);
    will_set_parameter(__wrap_samconfConfigGetBool, result, false);
    will_return(__wrap_samconfConfigGetBool, SAMCONF_CONFIG_OK);

    MOCK_FUNC_AFTER_CALL(samconfConfigGetString, 0);
    expect_value(__wrap_samconfConfigGetString, root, &mockConfig);
    expect_string(__wrap_samconfConfigGetString, path, ELOS_CONFIG_SCANNER "SyslogScanner/SyslogPath");
    expect_any(__wrap_samconfConfigGetString, result);
    will_set_parameter(__wrap_samconfConfigGetString, result, expectedValue);
    will_return(__wrap_samconfConfigGetString, SAMCONF_CONFIG_OK);

    const char *returnValue = elosConfigGetElosdSyslogSocketPath(&mockConfig);
    assert_string_equal(returnValue, expectedValue);
}

// SPDX-License-Identifier: MIT

#include "elosEventToJsonObject_utest.h"

int elosTestElosEventToJsonObjectPayloadErrorSetup(void **state) {
    static elosTestState_t test = {0};
    test.result = json_object_new_object();
    assert_non_null(test.result);
    *state = &test;
    return 0;
}

int elosTestElosEventToJsonObjectPayloadErrorTeardown(void **state) {
    elosTestState_t *test = *(elosTestState_t **)state;
    json_object_put(test->result);
    return 0;
}

void elosTestElosEventToJsonObjectPayloadError(void **state) {
    safuResultE_t retval = SAFU_RESULT_OK;
    elosTestState_t *test = *(elosTestState_t **)state;
    const elosEvent_t testEvent = {.payload = "payload"};

    TEST("elosEventToJsonObject");
    SHOULD("%s", "return null as addition of new payload object fails");

    MOCK_FUNC_AFTER_CALL(safuJsonAddNewString, 0);
    expect_not_value(__wrap_safuJsonAddNewString, jobj, NULL);
    expect_string(__wrap_safuJsonAddNewString, name, "payload");
    expect_value(__wrap_safuJsonAddNewString, val, testEvent.payload);
    will_return(__wrap_safuJsonAddNewString, NULL);

    retval = elosEventToJsonObject(test->result, &testEvent);
    assert_int_equal(retval, SAFU_RESULT_FAILED);
}

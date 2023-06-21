// SPDX-License-Identifier: MIT

#include "elosClientManagerThreadListen_utest.h"

int elosTestElosClientManagerThreadSuccessOnesSetup(void **state) {
    elosClientManagerContext_t *context = *state;
    memset(context, '\0', sizeof(elosClientManagerContext_t));
    context->status |= CLIENT_MANAGER_LISTEN_ACTIVE;
    int retval = pthread_mutex_init(&context->lock, NULL);
    assert_int_equal(retval, 0);
    return 0;
}

int elosTestElosClientManagerThreadSuccessOnesTeardown(UNUSED void **state) {
    return 0;
}

void elosTestElosClientManagerThreadSuccessOnes(void **state) {
    elosClientManagerContext_t *context = *state;
    elosClientManagerConnection_t *connection = &context->connection[MOCK_SLOT];

    MOCK_FUNC_ALWAYS(elosClientManagerGetStatus);
    MOCK_FUNC_ALWAYS(pthread_mutex_lock);
    MOCK_FUNC_ALWAYS(pthread_mutex_unlock);

    TEST("elosClientManagerThreadListen");
    SHOULD("%s", "successfully create a connection");

    expect_value(elosClientManagerGetStatus, context, context);
    expect_not_value(elosClientManagerGetStatus, status, NULL);
    will_set_parameter(elosClientManagerGetStatus, status, context->status);
    will_return(elosClientManagerGetStatus, SAFU_RESULT_OK);

    MOCK_FUNC_AFTER_CALL(elosClientManagerThreadGetFreeConnectionSlot, 0);
    expect_value(elosClientManagerThreadGetFreeConnectionSlot, ctx, context);
    expect_not_value(elosClientManagerThreadGetFreeConnectionSlot, slot, NULL);
    will_set_parameter(elosClientManagerThreadGetFreeConnectionSlot, slot, MOCK_SLOT);
    will_return(elosClientManagerThreadGetFreeConnectionSlot, SAFU_RESULT_OK);

    expect_value(__wrap_pthread_mutex_lock, __mutex, &connection->lock);
    will_return(__wrap_pthread_mutex_lock, 0);

    MOCK_FUNC_AFTER_CALL(elosClientManagerThreadWaitForIncomingConnection, 0);
    expect_value(elosClientManagerThreadWaitForIncomingConnection, ctx, context);
    expect_value(elosClientManagerThreadWaitForIncomingConnection, slot, MOCK_SLOT);
    will_return(elosClientManagerThreadWaitForIncomingConnection, SAFU_RESULT_OK);

    MOCK_FUNC_AFTER_CALL(pthread_create, 0);
    expect_value(__wrap_pthread_create, __newthread, &connection->thread);
    expect_value(__wrap_pthread_create, __attr, 0);
    expect_any(__wrap_pthread_create, __start_routine);
    expect_value(__wrap_pthread_create, __arg, (void *)connection);
    will_return(__wrap_pthread_create, 0);

    expect_value(__wrap_pthread_mutex_unlock, __mutex, &connection->lock);
    will_return(__wrap_pthread_mutex_unlock, 0);

    // connection created, break up to leave loop

    expect_value(elosClientManagerGetStatus, context, context);
    expect_not_value(elosClientManagerGetStatus, status, NULL);
    will_set_parameter(elosClientManagerGetStatus, status, 0);
    will_return(elosClientManagerGetStatus, SAFU_RESULT_OK);

    expect_value(__wrap_pthread_mutex_lock, __mutex, &context->lock);
    will_return(__wrap_pthread_mutex_lock, 0);

    expect_value(__wrap_pthread_mutex_unlock, __mutex, &context->lock);
    will_return(__wrap_pthread_mutex_unlock, 0);

    elosClientManagerThreadListen(context);

    MOCK_FUNC_NEVER(elosClientManagerGetStatus);
    MOCK_FUNC_NEVER(pthread_mutex_lock);
    MOCK_FUNC_NEVER(pthread_mutex_unlock);
}

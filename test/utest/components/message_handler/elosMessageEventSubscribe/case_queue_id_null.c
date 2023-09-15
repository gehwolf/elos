// SPDX-License-Identifier: MIT
#include <stdlib.h>

#include "cmocka_mocks/mock_jsonc.h"
#include "elosMessageEventSubscribe_utest.h"
#include "mock_event.h"
#include "mock_eventfiltermanager.h"
#include "mock_eventprocessor.h"
#include "mock_message_handler.h"
#include "safu/common.h"
#include "safu/mock_log.h"
#include "safu/mock_safu.h"
#include "safu/mock_vector.h"

int elosTestElosMessageEventSubscribeQueueIdNullSetup(void **state) {
    elosUtestState_t *data = safuAllocMem(NULL, sizeof(elosUtestState_t));
    memset(data, 0, sizeof(elosUtestState_t));
    data->conn = safuAllocMem(NULL, sizeof(elosClientConnection_t));
    __real_safuVecCreate(&data->conn->data.eventFilterNodeIdVector, 1, sizeof(int));
    __real_safuVecCreate(&data->conn->data.eventQueueIdVector, 1, sizeof(int));
    data->conn->sharedData = safuAllocMem(NULL, sizeof(elosClientConnectionSharedData_t));
    data->conn->sharedData->eventProcessor = safuAllocMem(NULL, sizeof(elosEventProcessor_t));
    *state = (void *)data;
    return 0;
}

int elosTestElosMessageEventSubscribeQueueIdNullTeardown(void **state) {
    elosUtestState_t *data = (elosUtestState_t *)*state;
    __real_json_object_put(data->response);
    free(data->conn->sharedData->eventProcessor);
    free(data->conn->sharedData);
    __real_safuVecFree(&data->conn->data.eventFilterNodeIdVector);
    __real_safuVecFree(&data->conn->data.eventQueueIdVector);
    free(data->conn);
    free(data->msg);
    free(data);
    return 0;
}

void elosTestElosMessageEventSubscribeQueueIdNull(void **state) {
    safuResultE_t result;
    const char *msg =
        "{\"filter\":["
        "\"event.source.appName,'myApp',eq\","
        "\"event.source.appName,'linux',eq\","
        "\"event.source.appName,'openssh',eq\""
        "]}";
    const char *filterStrings[] = {"event.source.appName,'myApp',eq", "event.source.appName,'linux',eq",
                                   "event.source.appName,'openssh',eq"};
    elosUtestState_t *data;
    elosEventFilterNodeId_t nodeId = 1;
    elosEventQueueId_t queueId = ELOS_ID_INVALID;
    elosArrayStringTestData_t arrayStringTestData = {filterStrings, 3};

    TEST("elosMessageEventSubscribe");
    SHOULD("%s", "fail as elosEventQueueManagerEntryNew returns an invalid queue id");

    data = (elosUtestState_t *)*state;
    assert_non_null(data);

    data->msg = safuAllocMem(NULL, sizeof(elosMessage_t *) + strlen(msg) + 1);
    memcpy(data->msg->json, msg, strlen(msg) + 1);
    elosEventProcessor_t *eventProcessor = data->conn->sharedData->eventProcessor;

    data->response = elosMessageHandlerResponseCreate("failed to initialize eventFilterNode/eventQueue");
    assert_non_null(data->response);

    MOCK_FUNC_ENABLE(elosMessageHandlerSendJson);
    MOCK_FUNC_ENABLE(elosEventProcessorFilterNodeCreateWithQueue);
    MOCK_FUNC_ENABLE(elosEventFilterManagerNodeRemove);

    expect_value(elosEventProcessorFilterNodeCreateWithQueue, eventProcessor, eventProcessor);
    expect_check(elosEventProcessorFilterNodeCreateWithQueue, filterStrings, elosAssertStringArraysEqual,
                 &arrayStringTestData);
    expect_value(elosEventProcessorFilterNodeCreateWithQueue, filterStringCount, 3);
    expect_any(elosEventProcessorFilterNodeCreateWithQueue, eventQueueId);
    expect_any(elosEventProcessorFilterNodeCreateWithQueue, eventFilterNodeId);
    will_set_parameter(elosEventProcessorFilterNodeCreateWithQueue, eventQueueId, queueId);
    will_set_parameter(elosEventProcessorFilterNodeCreateWithQueue, eventFilterNodeId, nodeId);
    will_return(elosEventProcessorFilterNodeCreateWithQueue, SAFU_RESULT_FAILED);

    expect_value(elosMessageHandlerSendJson, conn, data->conn);
    expect_value(elosMessageHandlerSendJson, messageId, ELOS_MESSAGE_RESPONSE_EVENT_SUBSCRIBE);
    expect_check(elosMessageHandlerSendJson, jobj, elosCheckJsonObject, data->response);
    will_return(elosMessageHandlerSendJson, SAFU_RESULT_OK);

    expect_value(elosEventFilterManagerNodeRemove, efm, &eventProcessor->eventFilterManager);
    expect_value(elosEventFilterManagerNodeRemove, efnId, nodeId);
    will_return(elosEventFilterManagerNodeRemove, SAFU_RESULT_OK);

    result = elosMessageEventSubscribe(data->conn, data->msg);

    MOCK_FUNC_NEVER(safuVecPush);

    assert_int_equal(result, SAFU_RESULT_FAILED);
}

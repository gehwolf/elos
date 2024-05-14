// SPDX-License-Identifier: MIT
#pragma once

#include <cmocka_extensions/cmocka_extensions.h>
#include "forwarder.h"

typedef struct elosTestState {
    char *timezone;
    samconfConfig_t *childrenData;
    elosForwarder_t forwarder;
} elosTestState_t;

int elosSyslogForwarderForwardMessageUtestInit(void **state);
int elosSyslogForwarderForwardMessageUtestCleanUp(void **state);

TEST_CASE_FUNC_PROTOTYPES(elosTestElosSyslogForwarderForwardMessageSuccess)

// SPDX-License-Identifier: MIT
#pragma once

#include <samconf/samconf.h>

#include <cmocka_extensions/cmocka_extensions.h>
#include <forwarder.h>

typedef struct {
    samconfConfig_t *childrenData;
} elosUteststateT_t;

int elosLoglineMapperInitUtestCleanUp(void **state);
int elosLoglineMapperInitUtestInit(void **state);

TEST_CASE_FUNC_PROTOTYPES(elosTestElosLoglineMapperInitSuccess)
TEST_CASE_FUNC_PROTOTYPES(elosTestElosLoglineMapperInitSectionedMessageCodes)
TEST_CASE_FUNC_PROTOTYPES(elosTestElosLoglineMapperInitErrDeepNestedMessageCodes)
TEST_CASE_FUNC_PROTOTYPES(elosTestElosLoglineMapperInitNullConfig)
#endif

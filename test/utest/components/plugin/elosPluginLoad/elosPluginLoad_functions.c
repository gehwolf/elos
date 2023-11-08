// SPDX-License-Identifier: MIT

#include <safu/log.h>
#include <sys/eventfd.h>

#include "elosPluginLoad_utest.h"

char const *elosPluginFuncCustomName[ELOS_PLUGIN_FUNC_COUNT] = {
    [ELOS_PLUGIN_FUNC_LOAD] = "elosCustomLoad",
    [ELOS_PLUGIN_FUNC_START] = "elosCustomStart",
    [ELOS_PLUGIN_FUNC_STOP] = "elosCustomStop",
    [ELOS_PLUGIN_FUNC_UNLOAD] = "elosCustomUnload",
};

safuResultE_t elosCustomLoad(elosPluginContext_t *plugin) {
    elosPluginTestData_t *data = (elosPluginTestData_t *)plugin->data;
    safuResultE_t result = SAFU_RESULT_OK;

    data->customFuncBits |= _CUSTOM_LOAD_BIT;

    return result;
}

safuResultE_t elosCustomStart(elosPluginContext_t *plugin) {
    elosPluginTestData_t *data = (elosPluginTestData_t *)plugin->data;
    safuResultE_t result = SAFU_RESULT_OK;

    data->customFuncBits |= _CUSTOM_START_BIT;

    return result;
}

safuResultE_t elosCustomStop(elosPluginContext_t *plugin) {
    elosPluginTestData_t *data = (elosPluginTestData_t *)plugin->data;
    safuResultE_t result = SAFU_RESULT_OK;

    data->customFuncBits |= _CUSTOM_STOP_BIT;

    return result;
}

safuResultE_t elosCustomUnload(elosPluginContext_t *plugin) {
    elosPluginTestData_t *data = (elosPluginTestData_t *)plugin->data;
    safuResultE_t result = SAFU_RESULT_OK;

    data->customFuncBits |= _CUSTOM_UNLOAD_BIT;

    return result;
}
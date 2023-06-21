// SPDX-License-Identifier: MIT

#include "elos/plugin/plugin.h"

#include <dlfcn.h>
#include <safu/common.h>
#include <safu/log.h>
#include <safu/mutex.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define _LOG_ERR_PLUGIN_WRONG_STATE(__plugin, __state) \
    safuLogErrF("Plugin id=%d is not in state '%s' (state=%d)", (__plugin)->id, __state, (__plugin)->state);

#define _LOG_ERR_PLUGIN_UNKNOWN_STATE(__plugin) \
    safuLogErrF("Plugin id=%d is in unknown state=%d", (__plugin)->id, (__plugin)->state);

safuResultE_t elosPluginInitialize(elosPlugin_t *plugin, elosPluginParam_t const *param) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if ((plugin == NULL) || (param == NULL)) {
        safuLogErr("Invalid parameter");
    } else if (plugin->state != PLUGIN_STATE_INVALID) {
        safuLogErr("The given plugin struct is not in state 'PLUGIN_STATE_INVALID'");
    } else {
        int eventFdWorker;
        int eventFdSync;
        int eventFdStop;

        eventFdStop = eventfd(0, 0);
        if (eventFdStop < 0) {
            safuLogErrErrno("eventfd failed");
        } else {
            eventFdSync = eventfd(0, 0);
            if (eventFdSync < 0) {
                safuLogErrErrno("eventfd failed");
            } else {
                eventFdWorker = eventfd(0, 0);
                if (eventFdWorker < 0) {
                    safuLogErrErrno("eventfd (worker) failed");
                } else {
                    plugin->id = param->id;
                    plugin->path = param->path;
                    plugin->config = param->config;
                    plugin->data = param->data;
                    plugin->stop = eventFdStop;
                    plugin->sync = eventFdSync;
                    plugin->worker.sync = eventFdWorker;
                    plugin->worker.isThreadRunning = false;
                    plugin->state = PLUGIN_STATE_INITIALIZED;
                    result = SAFU_RESULT_OK;
                }
            }
        }
    }

    return result;
}

safuResultE_t elosPluginNew(elosPlugin_t **plugin, elosPluginParam_t const *param) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if ((plugin == NULL) || (param == NULL)) {
        safuLogErr("Invalid parameter");
    } else {
        void *newPlugin;
        newPlugin = safuAllocMem(NULL, sizeof(elosPlugin_t));
        if (newPlugin == NULL) {
            safuLogErr("Memory allocation failed");
        } else {
            memset(newPlugin, 0, sizeof(elosPlugin_t));

            result = elosPluginInitialize(newPlugin, param);
            if (result != SAFU_RESULT_OK) {
                safuLogErr("elosPluginInitialize failed");
            } else {
                *plugin = newPlugin;
            }
        }
    }

    return result;
}

safuResultE_t elosPluginStart(elosPlugin_t *plugin) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (plugin == NULL) {
        safuLogErr("NULL parameter");
    } else {
        bool startNeeded = false;

        switch (plugin->state) {
            case PLUGIN_STATE_LOADED:
                startNeeded = true;
                break;
            case PLUGIN_STATE_INVALID:
            case PLUGIN_STATE_INITIALIZED:
            case PLUGIN_STATE_STARTED:
            case PLUGIN_STATE_STOPPED:
            case PLUGIN_STATE_UNLOADED:
            case PLUGIN_STATE_ERROR:
                _LOG_ERR_PLUGIN_WRONG_STATE(plugin, "LOADED");
                break;
            default:
                _LOG_ERR_PLUGIN_UNKNOWN_STATE(plugin);
                break;
        }

        if (startNeeded == true) {
            int retVal;

            retVal = eventfd_write(plugin->sync, 1);
            if (retVal < 0) {
                safuLogErrErrno("eventfd_write (sync) failed");
            } else {
                eventfd_t efdVal = 0;

                retVal = eventfd_read(plugin->worker.sync, &efdVal);
                if (retVal < 0) {
                    safuLogErrErrno("eventfd_read (worker.sync) failed");
                } else if (plugin->state != PLUGIN_STATE_STARTED) {
                    _LOG_ERR_PLUGIN_WRONG_STATE(plugin, "STARTED");
                } else {
                    result = SAFU_RESULT_OK;
                }
            }
        }
    }

    return result;
}

safuResultE_t elosPluginStop(elosPlugin_t *plugin) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (plugin == NULL) {
        safuLogErr("NULL parameter");
    } else {
        bool stopNeeded = false;

        switch (plugin->state) {
            case PLUGIN_STATE_STARTED:
                stopNeeded = true;
                break;
            case PLUGIN_STATE_INVALID:
            case PLUGIN_STATE_INITIALIZED:
            case PLUGIN_STATE_STOPPED:
            case PLUGIN_STATE_LOADED:
            case PLUGIN_STATE_UNLOADED:
            case PLUGIN_STATE_ERROR:
                _LOG_ERR_PLUGIN_WRONG_STATE(plugin, "STARTED");
                break;
            default:
                _LOG_ERR_PLUGIN_UNKNOWN_STATE(plugin);
                break;
        }

        if (stopNeeded == true) {
            result = plugin->func.stop(plugin);
            if (result != SAFU_RESULT_OK) {
                safuLogErrF("PluginWorker stop call failed for %s", plugin->config->key);
            } else {
                eventfd_t efdVal = 0;
                int retVal;

                retVal = eventfd_read(plugin->worker.sync, &efdVal);
                if (retVal < 0) {
                    safuLogErrErrno("eventfd_read (worker.sync) failed");
                } else if (plugin->state != PLUGIN_STATE_STOPPED) {
                    _LOG_ERR_PLUGIN_WRONG_STATE(plugin, "STOPPED");
                } else {
                    result = SAFU_RESULT_OK;
                }
            }
        }
    }

    return result;
}

safuResultE_t elosPluginUnload(elosPlugin_t *plugin) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (plugin == NULL) {
        safuLogErr("Invalid parameter");
    } else {
        bool unloadNeeded = true;

        switch (plugin->state) {
            case PLUGIN_STATE_STOPPED:
            case PLUGIN_STATE_ERROR:
                break;
            case PLUGIN_STATE_STARTED:
                result = elosPluginStop(plugin);
                if (result != SAFU_RESULT_OK) {
                    safuLogErr("elosPluginStop failed");
                }
                break;
            case PLUGIN_STATE_INITIALIZED:
            case PLUGIN_STATE_UNLOADED:
                unloadNeeded = false;
                break;
            default:
                _LOG_ERR_PLUGIN_UNKNOWN_STATE(plugin);
                result = SAFU_RESULT_FAILED;
                unloadNeeded = false;
                break;
        }

        if (unloadNeeded == true) {
            if (plugin->worker.isThreadRunning) {
                int retVal = pthread_join(plugin->worker.thread, NULL);
                if (retVal < 0) {
                    safuLogErr("Plugin: pthread_join failed!");
                    result = SAFU_RESULT_FAILED;
                } else {
                    plugin->worker.isThreadRunning = false;
                }
            }

            if (plugin->func.unload != NULL) {
                result = plugin->func.unload(plugin);
                if (result != SAFU_RESULT_OK) {
                    safuLogWarn("PluginWorker: Unload function failed (might result in memory leaks)");
                }
            }

            plugin->state = PLUGIN_STATE_UNLOADED;
        }
    }

    return result;
}

safuResultE_t elosPluginDeleteMembers(elosPlugin_t *plugin) {
    safuResultE_t result = SAFU_RESULT_OK;

    if (plugin != NULL) {
        bool cleanupNeeded = false;

        switch (plugin->state) {
            case PLUGIN_STATE_STARTED:
            case PLUGIN_STATE_STOPPED:
            case PLUGIN_STATE_ERROR:
                result = elosPluginUnload(plugin);
                if (result != SAFU_RESULT_OK) {
                    safuLogWarn("elosPluginUnload failed (might result in memory leaks)");
                }
                cleanupNeeded = true;
                break;
            case PLUGIN_STATE_INITIALIZED:
            case PLUGIN_STATE_UNLOADED:
                cleanupNeeded = true;
                break;
            case PLUGIN_STATE_INVALID:
                break;
            default:
                _LOG_ERR_PLUGIN_UNKNOWN_STATE(plugin);
                result = SAFU_RESULT_FAILED;
                break;
        }

        if (cleanupNeeded == true) {
            int retVal;

            if (plugin->dlHandle != NULL) {
                retVal = dlclose(plugin->dlHandle);
                if (retVal < 0) {
                    safuLogWarnF("dlclose for plugin id:%d failed with: %s", plugin->id, strerror(errno));
                    result = SAFU_RESULT_FAILED;
                }
            }

            free(plugin->file);

            retVal = close(plugin->stop);
            if (retVal < 0) {
                safuLogWarnF("close (stop) for plugin id:%d failed with: %s", plugin->id, strerror(errno));
                result = SAFU_RESULT_FAILED;
            }

            retVal = close(plugin->sync);
            if (retVal < 0) {
                safuLogWarnF("close (sync) for plugin id:%d failed with: %s", plugin->id, strerror(errno));
                result = SAFU_RESULT_FAILED;
            }

            retVal = close(plugin->worker.sync);
            if (retVal < 0) {
                safuLogWarnF("close (worker.sync) for plugin id:%d failed with: %s", plugin->id, strerror(errno));
                result = SAFU_RESULT_FAILED;
            }

            memset(plugin, 0, sizeof(elosPlugin_t));
        }
    }

    return result;
}

safuResultE_t elosPluginGetName(const elosPlugin_t *plugin, const char **name) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (plugin != NULL && plugin->config != NULL && name != NULL) {
        *name = plugin->config->key;
        result = SAFU_RESULT_OK;
    } else {
        safuLogErr("Invalid parameters");
    }

    return result;
}

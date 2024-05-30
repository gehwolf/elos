// SPDX-License-Identifier: MIT
#include <errno.h>
#include <locale.h>
#include <safu/common.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "elos/clientmanager/clientmanager.h"
#include "elos/config/config.h"
#include "elos/connectionmanager/connectionmanager.h"
#include "elos/eloslog/eloslog.h"
#include "elos/eventdispatcher/eventdispatcher.h"
#include "elos/eventlogging/LogAggregator.h"
#include "elos/eventprocessor/eventprocessor.h"
#include "elos/logger/logger.h"
#include "elos/pluginmanager/pluginmanager.h"
#include "elos/scanner_manager_legacy/scanner_manager.h"
#include "elos/scannermanager/scannermanager.h"
#include "elos/storagemanager/storagemanager.h"
#include "safu/common.h"
#include "safu/log.h"
#include "samconf/samconf.h"
#include "version.h"

#define MAIN_SLEEP_TIME_USEC (100 * 1000)

struct serverContext {
    samconfConfig_t *config;
    elosConnectionManager_t connectionManagerContext;
    elosClientManager_t clientManagerContext;
    elosScannerManagerLegacyContext_t scannerManagerLegacyContext;
    elosScannerManager_t scannerManager;
    elosPluginManager_t pluginManager;
    elosLogAggregator_t logAggregator;
    elosEventDispatcher_t eventDispatcher;
    elosEventProcessor_t eventProcessor;
    elosStorageManager_t storageManager;
    elosLogger_t *logger;
};

static int elosActive = 1;

void elosSignalHandler(int signalno) {
    switch (signalno) {
        case SIGINT:
            safuLogDebug("Received SIGINT");
            elosActive = 0;
            signal(SIGINT, SIG_DFL);
            break;
        case SIGTERM:
            safuLogDebug("Received SIGTERM");
            signal(SIGTERM, SIG_DFL);
            elosActive = 0;
            break;
        default:
            safuLogWarn("Received unregistered signal");
            break;
    }
}

int elosServerShutdown(struct serverContext *ctx) {
    if (ctx == NULL) {
        safuLogErr("invalid parameter");
        return EXIT_FAILURE;
    }

    safuLogInfo("Shutting down...");
    int result = EXIT_SUCCESS;

    if (elosLogAggregatorShutdown(&ctx->logAggregator) != SAFU_RESULT_OK) {
        safuLogErr("Shutting down log aggregator failed!");
        result = EXIT_FAILURE;
    }
    if (elosClientManagerStop(&ctx->clientManagerContext) != SAFU_RESULT_OK) {
        safuLogErr("Unloading client plugins failed!");
        result = EXIT_FAILURE;
    }
    if (elosClientManagerDeleteMembers(&ctx->clientManagerContext) != SAFU_RESULT_OK) {
        safuLogErr("Stoping client manager failed!");
        result = EXIT_FAILURE;
    }
    if (elosStorageManagerStop(&ctx->storageManager) != SAFU_RESULT_OK) {
        safuLogErr("Stopping storage manager failed!");
        result = EXIT_FAILURE;
    } else if (elosStorageManagerDeleteMembers(&ctx->storageManager) != SAFU_RESULT_OK) {
        safuLogErr("Deleting storage manager failed!");
        result = EXIT_FAILURE;
    }
    if (elosConnectionManagerStop(&ctx->connectionManagerContext) != SAFU_RESULT_OK) {
        safuLogErr("Stoping connection manager failed!");
        result = EXIT_FAILURE;
    } else if (elosConnectionManagerDeleteMembers(&ctx->connectionManagerContext) != SAFU_RESULT_OK) {
        safuLogErr("Deleting connection manager failed!");
        result = EXIT_FAILURE;
    }
    if (elosScannerManagerLegacyStop(&ctx->scannerManagerLegacyContext) != NO_ERROR) {
        safuLogErr("Stopping scanner manager legacy failed!");
        result = EXIT_FAILURE;
    }
    if (elosScannerManagerStop(&ctx->scannerManager) != SAFU_RESULT_OK) {
        safuLogErr("Stopping scanner manager failed!");
        result = EXIT_FAILURE;
    } else if (elosScannerManagerDeleteMembers(&ctx->scannerManager) != SAFU_RESULT_OK) {
        safuLogErr("Deleting scanner manager failed!");
        result = EXIT_FAILURE;
    }
    if (elosEventDispatcherDeleteMembers(&ctx->eventDispatcher) != SAFU_RESULT_OK) {
        safuLogErr("Deleting event dispatcher members failed!");
        result = EXIT_FAILURE;
    }
    if (elosEventProcessorDeleteMembers(&ctx->eventProcessor) != SAFU_RESULT_OK) {
        safuLogErr("Deleting event processor members failed!");
        result = EXIT_FAILURE;
    }
    if (elosPluginManagerDeleteMembers(&ctx->pluginManager) != SAFU_RESULT_OK) {
        safuLogErr("Deleting plugin processor members failed!");
        result = EXIT_FAILURE;
    }
    if (elosLoggerDeleteMembers(ctx->logger) != SAFU_RESULT_OK) {
        safuLogErr("Deleting logger members failed!");
        result = EXIT_FAILURE;
    }
    if (samconfConfigDelete(ctx->config) != SAMCONF_CONFIG_OK) {
        safuLogErr("Deleting config failed!");
        result = EXIT_FAILURE;
    }

    return result;
}

int main(int argc, char **argv) {
    int retval;
    struct serverContext context = {0};

    setlocale(LC_ALL, "C");
    safuLogSetPrefix(ELOSD_LOG_PREFIX);

    if (elosIsVersionRequested((const char **)argv, argc)) {
        const char *verstr;
        verstr = elosGetVersionString();
        safuLogInfoF("elosd-%s", verstr);
        return EXIT_SUCCESS;
    }

    if (signal(SIGINT, elosSignalHandler) == SIG_ERR) {
        safuLogErrErrno("signal_SIGINT");
        return EXIT_FAILURE;
    }

    if (signal(SIGTERM, elosSignalHandler) == SIG_ERR) {
        safuLogErrErrno("signal(SIGTERM)");
        return EXIT_FAILURE;
    }

    elosLoggerGetDefaultLogger(&context.logger);

    elosLog(ELOS_MSG_CODE_DEBUG_LOG, ELOS_SEVERITY_DEBUG, ELOS_CLASSIFICATION_ELOS, "internal logger initialized");

    safuLogDebug("Load configuration");
    safuResultE_t result = elosConfigLoad(&context.config);
    if (result != SAFU_RESULT_OK) {
        safuLogErr("samconfLoad");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }

    safuLogLevelE_t logLevel = elosConfigGetElosdLogLevel(context.config);
    safuLogStatusE_t loggingResult = safuLogSetStreamLevel(logLevel);
    if (loggingResult != SAFU_LOG_STATUS_SUCCESS) {
        safuLogWarnF("setting log level: %s failed! Fallback to log level: %s.", safuLogLevelToString(logLevel),
                     safuLogLevelToString(safuLogGetStreamLevel()));
    }
    loggingResult = safuLogSetFilterStr(elosConfigGetElosdLogFilter(context.config));
    if (loggingResult != SAFU_LOG_STATUS_SUCCESS) {
        safuLogWarn("setting log filter failed!");
    }

    safuLogInfoF("Setup:\n\tlisten on: %s:%d\n\thardwareid: %s\n\tlog level: %s\n\tlog filter: %s\n\tscanner path: %s",
                 elosConfigGetElosdInterface(context.config), elosConfigGetElosdPort(context.config),
                 safuGetHardwareId(), safuLogLevelToString(safuLogGetStreamLevel()),
                 elosConfigGetElosdLogFilter(context.config), elosConfigGetElosdScannerPath(context.config));

    safuLogDebug("Initialize EventProcessor");
    elosEventProcessorParam_t const epParam = {.config = context.config};
    retval = elosEventProcessorInitialize(&context.eventProcessor, &epParam);
    if (retval < 0) {
        safuLogErr("elosEventProcessorInitialize");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }

    safuLogDebug("Initialize EventDispatcher");
    elosEventDispatcherParam_t const edParam = {
        .eventProcessor = &context.eventProcessor,
        .healthTimeInterval = NULL,
        .pollTimeout = NULL,
    };
    retval = elosEventDispatcherInitialize(&context.eventDispatcher, &edParam);
    if (retval < 0) {
        safuLogErr("elosEventDispatcherInitialize");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }

    safuLogDebug("Initialize PluginManager");
    elosPluginManagerParam_t const pmParam = {
        .config = context.config,
        .eventProcessor = &context.eventProcessor,
        .eventDispatcher = &context.eventDispatcher,
        .logAggregator = &context.logAggregator,
    };
    result = elosPluginManagerInitialize(&context.pluginManager, &pmParam);
    if (result != SAFU_RESULT_OK) {
        safuLogErr("elosPluginManagerInitialize had errors during execution");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }

    safuLogDebug("Initialize StorageManager");
    elosStorageManagerParam_t const smParam = {.config = context.config, .pluginManager = &context.pluginManager};
    result = elosStorageManagerInitialize(&context.storageManager, &smParam);
    if (result != SAFU_RESULT_OK) {
        safuLogErr("elosStorageManagerInitialize had errors during execution");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    } else {
        result = elosStorageManagerStart(&context.storageManager);
        if (result != SAFU_RESULT_OK) {
            safuLogErr("elosStorageManagerStart had errors during execution");
            elosServerShutdown(&context);
            return EXIT_FAILURE;
        }
    }

    safuLogDebug("Initialize client manager");
    elosClientManagerParam_t const clmParam = {.config = context.config, .pluginManager = &context.pluginManager};
    result = elosClientManagerInitialize(&context.clientManagerContext, &clmParam);
    if (result != SAFU_RESULT_OK) {
        safuLogErr("elosClientManagerInitialize had errors during execution");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    } else {
        result = elosClientManagerStart(&context.clientManagerContext);
        if (result != SAFU_RESULT_OK) {
            safuLogErr("elosClientManagerStart had errors during execution");
            elosServerShutdown(&context);
            return EXIT_FAILURE;
        }
    }

    safuLogDebug("Start LogAggregator");
    elosLogAggregatorParam_t const laParam = {.config = context.config, .backends = &context.storageManager.backends};
    result = elosLogAggregatorStart(&context.logAggregator, &laParam);
    if (result != SAFU_RESULT_OK) {
        safuLogWarn("elosLogAggregatorStart had errors during execution");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }

    safuLogDebug("Start connection manager");
    elosConnectionManagerParam_t cmParams = {
        .config = context.config,
        .logAggregator = &context.logAggregator,
        .eventProcessor = &context.eventProcessor,
        .eventDispatcher = &context.eventDispatcher,
    };
    result = elosConnectionManagerInitialize(&context.connectionManagerContext, &cmParams);
    if (result != SAFU_RESULT_OK) {
        safuLogErr("elosConnectionManagerInitialize");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    } else {
        retval = elosConnectionManagerStart(&context.connectionManagerContext);
        if (retval < 0) {
            safuLogErr("elosConnectionManagerStart");
            elosServerShutdown(&context);
            return EXIT_FAILURE;
        }
    }

    safuLogDebug("Start EventDispatcher");
    retval = elosEventDispatcherStart(&context.eventDispatcher);
    if (retval < 0) {
        safuLogErr("elosEventDispatcherStart");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }
    elosEventDispatcherBufferAdd(&context.eventDispatcher, context.logger->logEventBuffer);

    safuLogDebug("Initialize scanner manager");
    elosScannerManagerParam_t const scmParam = {.config = context.config, .pluginManager = &context.pluginManager};
    result = elosScannerManagerInitialize(&context.scannerManager, &scmParam);
    if (result != SAFU_RESULT_OK) {
        safuLogErr("elosScannerManagerInitialize had errors during execution");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    } else {
        result = elosScannerManagerStart(&context.scannerManager);
        if (result != SAFU_RESULT_OK) {
            safuLogErr("elosScannerManagerLoad");
            elosServerShutdown(&context);
            return EXIT_FAILURE;
        }
    }

    safuLogDebug("Start legacy scanner manager");
    elosScannerManagerLegacyParam_t scannerManagerLegacyParam = {
        .config = context.config,
        .logAggregator = &context.logAggregator,
        .eventDispatcher = &context.eventDispatcher,
    };
    retval = elosScannerManagerLegacyStart(&context.scannerManagerLegacyContext, &scannerManagerLegacyParam);
    if ((retval != NO_ERROR) && (retval != NO_FATAL_ERRORS)) {
        safuLogErr("elosServerScannerLoad");
        elosServerShutdown(&context);
        return EXIT_FAILURE;
    }

    safuLogInfo("Running...");

    while (elosActive) {
        // Temporary solution to allow graceful program termination with raise() from threads
        usleep(MAIN_SLEEP_TIME_USEC);
    }

    retval = elosServerShutdown(&context);

    return retval;
}

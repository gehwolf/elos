#include <stdlib.h>
#include <string.h>
#include "safu/log.h"

#include "elos/rpnfilter/builder.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    elosRpnFilterResultE_t result = RPNFILTER_RESULT_ERROR;

    safuLogStatusE_t loggingResult = safuLogSetStreamLevel(SAFU_LOG_LEVEL_NONE);
    char *filter = calloc(size + 1, sizeof(char));
    strncpy(filter, (const char*)data, size);
    const elosRpnFilterParam_t param = {.filterString = filter};
    elosRpnFilterBuilder_t *builder = NULL;
    result = elosRpnFilterBuilderNew(&param, &builder);
    if (result == RPNFILTER_RESULT_OK) {
        result = elosRpnFilterBuilderTokenize(builder);
        result = elosRpnFilterBuilderDelete(builder);
    }
    free(filter);

    return 0;
}

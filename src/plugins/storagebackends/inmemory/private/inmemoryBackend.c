// SPDX-License-Identifier: MIT

#include "inmemoryBackend.h"

#include <safu/common.h>
#include <safu/defines.h>
#include <safu/log.h>
#include <safu/mutex.h>
#include <safu/result.h>
#include <safu/ringbuffer.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "elos/event/event.h"
#include "elos/event/event_types.h"

typedef struct _eventEntry {
    struct {
        uint16_t size;
        uint8_t hardwareIdSize;
        uint8_t reserved;
    } header;
    elosEvent_t event;
    uint8_t data[];
} _eventEntry_t;

safuResultE_t elosInMemoryBackendNew(elosInMemoryBackend_t **inmemory, elosInMemoryBackendParam_t const *param) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    elosInMemoryBackend_t *newBackend = safuAllocMem(NULL, sizeof(elosInMemoryBackend_t));
    if (newBackend == NULL) {
        safuLogErr("safuAllocMem failed");
        result = SAFU_RESULT_FAILED;
    } else {
        result = elosInMemoryBackendInit(newBackend, param);
        if (result != SAFU_RESULT_OK) {
            safuLogErr("Failed 'elosInMemoryBackendInit'");
        } else {
            *inmemory = newBackend;
        }
    }

    return result;
}

safuResultE_t elosInMemoryBackendInit(elosInMemoryBackend_t *inmemory, elosInMemoryBackendParam_t const *param) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (inmemory == NULL || param == NULL) {
        safuLogErr("Null parameter given");
    } else {
        SAFU_PTHREAD_MUTEX_INIT_WITH_RESULT(&inmemory->mutex, NULL, result)
        if (result != SAFU_RESULT_OK) {
            safuLogErr("Failed to intialize mutex");
        } else {
            inmemory->buffer = safuAllocMem(NULL, param->size * 1024);
            if (inmemory->buffer == NULL) {
                safuLogErrF("Failed to allocate in memory store of %lukb", param->size);
            } else {
                inmemory->next = inmemory->buffer;
                inmemory->newest = NULL;
                inmemory->oldest = NULL;
            }
        }
    }

    return result;
}

safuResultE_t elosInMemoryBackendDelete(elosInMemoryBackend_t *inmemory) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (inmemory == NULL) {
        safuLogErr("Null parameter given");
    } else {
        elosInMemoryBackendDeleteMembers(inmemory);
        free(inmemory);
        result = SAFU_RESULT_OK;
    }

    return result;
}

safuResultE_t elosInMemoryBackendDeleteMembers(elosInMemoryBackend_t *inmemory) {
    safuResultE_t result = SAFU_RESULT_OK;

    if (inmemory == NULL) {
        safuLogErr("Null parameter given");
    } else {
        SAFU_PTHREAD_MUTEX_DESTROY(&inmemory->mutex, result = SAFU_RESULT_FAILED);
        free(inmemory->buffer);
        memset(inmemory, 0, sizeof(*inmemory));
    }

    return result;
}

safuResultE_t elosInMemoryBackendStart(elosInMemoryBackend_t *inmemory) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (inmemory == NULL) {
        safuLogErr("Invalid parameter");
    }

    return result;
}

safuResultE_t elosInMemoryBackendStop(elosInMemoryBackend_t *inmemory) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if (inmemory == NULL) {
        safuLogErr("Null parameter given");
    }

    return result;
}

static _eventEntry_t *_allocateEntry(elosInMemoryBackend_t *inmemory, size_t entrySize) {
    _eventEntry_t *entry = inmemory->next;
    size_t size = (uint8_t *)inmemory->next - (uint8_t *)inmemory->buffer;

    if (size + entrySize >= inmemory->size) {
        entry = inmemory->buffer;
        if(&((uint8_t*)entry)[entrySize] + entrySize >= (uint8_t*)inmemory->oldest){
        }
    }

    return entry;
}

safuResultE_t elosInMemoryBackendPersist(elosInMemoryBackend_t *inmemory, const elosEvent_t *event) {
    safuResultE_t result = SAFU_RESULT_FAILED;

    if ((inmemory == NULL) || (event == NULL)) {
        safuLogErr("Null parameter given");
    } else {
        SAFU_PTHREAD_MUTEX_LOCK_WITH_RESULT(&inmemory->mutex, result);
        if (result == SAFU_RESULT_OK) {
            size_t hardwareIdSize = strnlen(event->hardwareid, UINT8_MAX);
            size_t payloadSize = strnlen(event->payload, UINT16_MAX);
            size_t entrySize = hardwareIdSize + payloadSize + sizeof(_eventEntry_t);

            _eventEntry_t *entry = _allocateEntry(inmemory, entrySize);
            entry->header.size = entrySize;
            entry->header.hardwareIdSize = hardwareIdSize;
            memcpy(&entry->event, event, sizeof(elosEvent_t));
            entry->event.hardwareid = (char *)entry->data;
            entry->event.payload = (char *)entry->data + hardwareIdSize;
            memcpy(entry->event.hardwareid, event->hardwareid, hardwareIdSize);
            memcpy(entry->event.payload, event->payload, payloadSize);

            inmemory->newest = entry;
            inmemory->next = &((uint8_t *)entry)[entrySize];
            if (inmemory->oldest == NULL) {
                inmemory->oldest = entry;
            }
        }
    }

    return result;
}

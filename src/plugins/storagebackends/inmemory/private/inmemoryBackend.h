// SPDX-License-Identifier: MIT

#pragma once

#include <elos/event/event.h>
#include <pthread.h>
#include <safu/ringbuffer_types.h>


/*******************************************************************
 * Members:
 *     size: size of inmemory storage in kilo bytes
 ******************************************************************/
typedef struct elosInMemoryBackendParam {
    size_t size;
} elosInMemoryBackendParam_t;

/*******************************************************************
 * Members:
 *     
 *     mutex: a mutex to enusre  thread safe buffer access
 ******************************************************************/
typedef struct elosInMemoryBackend {
    size_t size;
    void* buffer;
    void* next;
    void* oldest;
    void* newest;
    pthread_mutex_t mutex;
} elosInMemoryBackend_t;

/*******************************************************************
 * Allocate a new `elosInMemoryBackend`. It shall be safe to call
 * `elosInMemoryBackendDelete` even if `SAFU_RESULT_FAILED` is returned.
 * The new inmemory is initialized by `elosInMemoryBackendInit`.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` pointer to be set to the
 *                new inmemory. In case of an error it is unchanged.
 *       param: configuration options see elosInMemoryBackendParam_t
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendNew(elosInMemoryBackend_t **inmemory, elosInMemoryBackendParam_t const *param);

/*******************************************************************
 * Initialize a `elosInMemoryBackend`. It shall be safe to call
 * `elosInMemoryBackendDelete` even if `SAFU_RESULT_FAILED` is returned.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be initialized.
 *       param: configuration options see elosInMemoryBackendParam_t
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendNew(elosInMemoryBackend_t **inmemory, elosInMemoryBackendParam_t const *param);

/*******************************************************************
 * Initialize a `elosInMemoryBackend`. It shall be safe to call
 * `elosInMemoryBackendDelete` even if `SAFU_RESULT_FAILED` is returned.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be initialized.
 *       connectionString: Absolut path to the the DLT unix socket
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendInit(elosInMemoryBackend_t *inmemory, elosInMemoryBackendParam_t const *param);

/*******************************************************************
 * Delete a `elosInMemoryBackend`. It also delete its members by calling
 * `eloInMemoryBackendDelete`.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be deleted.
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendDelete(elosInMemoryBackend_t *inmemory);

/*******************************************************************
 * Delete and free all resources allocated by members of
 * `elosInMemoryBackend`.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be deleted.
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendDeleteMembers(elosInMemoryBackend_t *inmemory);

/*******************************************************************
 * Start a `elosInMemoryBackend` which actually connect to the DLT daemon.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be started.
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendStart(elosInMemoryBackend_t *inmemory);

/*******************************************************************
 * Stop a `elosInMemoryBackend`. It shall be safe to call this function on an
 * already stopped instance.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be stopped.
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendStop(elosInMemoryBackend_t *inmemory);

/*******************************************************************
 * Store an event into DLT infrastructure.
 *
 * Parameters:
 *       inmemory: pointer to a `elosInMemoryBackend` to be stopped.
 *       event: the event to store
 *
 * Returns:
 *      - `SAFU_RESULT_OK` on success
 *      - `SAFU_RESULT_FAILED` on failure
 ******************************************************************/
safuResultE_t elosInMemoryBackendPersist(elosInMemoryBackend_t *inmemory, const elosEvent_t *event);

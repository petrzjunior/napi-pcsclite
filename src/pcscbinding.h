#pragma once

#define NAPI_VERSION 4
#include <node_api.h>

#include "pcsclite.h"

#ifdef _WIN32
char *pcsc_stringify_error(LONG err);
#endif

void destructor(napi_env env, void *finalize_data, void *finalize_hint);

extern STATE statePresent;
extern STATE stateEmpty;

/* Establish context
 * This function needs to be called first.
 * @return context
 */
napi_value establish(napi_env env, napi_callback_info info);

/* Release context
 * This function needs to be called last.
 * @param context
 */
napi_value release(napi_env env, napi_callback_info info);

/* Get connected readers
 * @param context
 * @return array<string> readers' names
 */
napi_value getReaders(napi_env env, napi_callback_info info);

/* Connect to card
 * @param context
 * @param string Reader name
 * @return handle Card handle
 */
napi_value connectCard(napi_env env, napi_callback_info info);

/* Disconnect from card
 * @param handle
 */
napi_value disconnectCard(napi_env env, napi_callback_info info);

/* Cancel blocking wait
 * @param context
 */
napi_value cancel(napi_env env, napi_callback_info info);

/* Transmit data to card
 * @param handle
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
napi_value transmit(napi_env env, napi_callback_info info);

/* Get card status
 * @param handle
 * @return state
 */
napi_value getStatus(napi_env env, napi_callback_info info);

/* Send direct command to the reader
 * @param handle
 * @param command
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
napi_value directCommand(napi_env env, napi_callback_info info);

/* Subscribe to global change callback
 * @param context
 * @param callback
 */
napi_value globalChangeSubscribe(napi_env env, napi_callback_info info);

/* Wait until global state is changed
 * @param context
 * @return state
 */
napi_value waitUntilGlobalChange(napi_env env, napi_callback_info info);

/* Wait until reader state is changed
 * @param context
 * @param string Reader name
 * @param state Current state
 * @return state New srate
 */
napi_value waitUntilReaderChange(napi_env env, napi_callback_info info);

/* Wait until a new reader is connected
 * @param context
 * @return string Reader name
 */
napi_value waitUntilReaderConnected(napi_env env, napi_callback_info info);

/* Wait until desired reader state
 * @param context
 * @param string readerName
 * @param state Desired state
 */
napi_value waitUntilReaderState(napi_env env, napi_callback_info info);

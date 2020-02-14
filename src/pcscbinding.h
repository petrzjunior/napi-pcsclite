#pragma once

#define NAPI_VERSION 4

#include <node_api.h>

#include "pcsclite.h"

#ifdef _WIN32
#include <stdio.h>
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

void globalStatusExecute(napi_env _, void *data);

void globalStatusFinish(napi_env env, napi_status status, void *data);

/* Get global status change
 * @param context
 * @return promise(bool, err) Did change happen
 */
napi_value getGlobalStatusChange(napi_env env, napi_callback_info info);

void readerStatusExecute(napi_env _, void *data);

void readerStatusFinish(napi_env env, napi_status status, void *data);

/* Get reader status change
 * @param context
 * @param readerName
 * @return promise(state, err) New state
 */
napi_value getReaderStatusChange(napi_env env, napi_callback_info info);

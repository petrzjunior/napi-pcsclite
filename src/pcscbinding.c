#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <node_api.h>

#include "pcsclite.h"
#include "pcscbinding.h"

STATE statePresent = SCARD_STATE_PRESENT;
STATE stateEmpty = SCARD_STATE_EMPTY;
#ifdef _WIN32
char *pcsc_stringify_error(LONG err)
{
	// TODO: Add full error messages
	char *buffer = malloc(25 * sizeof(char));
	snprintf(buffer, sizeof(buffer), "Error code: %li\n", err);
	return buffer;
}
#endif

#define CATCH(expr)                                                        \
	{                                                                      \
		LONG err = expr;                                                   \
		if (err)                                                           \
		{                                                                  \
			CHECK(napi_throw_error(env, NULL, pcsc_stringify_error(err))); \
			return NULL;                                                   \
		}                                                                  \
	}
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CHECK(x)                                                                                        \
	if (x != napi_ok)                                                                                   \
	{                                                                                                   \
		napi_throw_error(env, NULL, "Internal error in file " __FILE__ " on line " TOSTRING(__LINE__)); \
		return NULL;                                                                                    \
	}

#define CHECK_ARGUMENT_COUNT(count)                                            \
	size_t argc = count;                                                       \
	napi_value args[count];                                                    \
	CHECK(napi_get_cb_info(env, info, &argc, args, NULL, NULL));               \
	if (argc != count)                                                         \
	{                                                                          \
		CHECK(napi_throw_error(env, NULL, "Expected " #count " argument(s)")); \
		return NULL;                                                           \
	}

#define CHECK_ARGUMENT_TYPE(i, type)                                                  \
	{                                                                                 \
		napi_valuetype actual_type;                                                   \
		CHECK(napi_typeof(env, args[i], &actual_type));                               \
		if (actual_type != type)                                                      \
		{                                                                             \
			napi_throw_type_error(env, NULL, "Wrong argument type, expected " #type); \
			return NULL;                                                              \
		}                                                                             \
	}

#define DECLARE_NAPI_METHOD(name, func)                        \
	{                                                          \
		name, NULL, func, NULL, NULL, NULL, napi_default, NULL \
	}

#define DECLARE_NAPI_CONSTANT(name, value)                      \
	{                                                           \
		name, NULL, NULL, NULL, NULL, value, napi_default, NULL \
	}

void destructor(napi_env env, void *finalize_data, void *finalize_hint)
{
	free(finalize_data);
}

/* Establish context
 * This function needs to be called first.
 * @return context
 */
napi_value establish(napi_env env, napi_callback_info info)
{
	SCARDCONTEXT *context = malloc(sizeof(SCARDCONTEXT));
	CHECK(pcscEstablish(context));

	napi_value ret_val;
	CHECK(napi_create_external(env, context, destructor, NULL, &ret_val));
	return ret_val;
}

/* Release context
 * This function needs to be called last.
 * @param context
 */
napi_value release(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));

	CATCH(pcscRelease(*context))

	return NULL;
}

/* Get connected readers
 * @param context
 * @return string Readers' names in a string
 */
napi_value getReaders(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));

	DWORD bufSize = 0;
	char *buffer;
	CATCH(pcscGetReaders(*context, &buffer, &bufSize));

	napi_value ret_val;
	CHECK(napi_create_string_utf8(env, buffer, bufSize, &ret_val));
	free(buffer);
	return ret_val;
}

/* Connect to card
 * @param context
 * @param string Reader name
 * @return handle Card handle
 */
napi_value connectCard(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_string)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));
	char readerName[50];
	CHECK(napi_get_value_string_utf8(env, args[1], readerName, sizeof(readerName), NULL));

	SCARDHANDLE *handle = malloc(sizeof(SCARDHANDLE));
	CATCH(pcscConnect(*context, readerName, handle));

	napi_value ret_val;
	CHECK(napi_create_external(env, handle, destructor, NULL, &ret_val));
	return ret_val;
}

/* Disconnect from card
 * @param handle
 */
napi_value disconnectCard(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDHANDLE *handle;
	CHECK(napi_get_value_external(env, args[0], (void **)&handle));

	CATCH(pcscDisconnect(*handle));

	return NULL;
}

/* Cancel blocking wait
 * @param context
 */
napi_value cancel(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));

	CATCH(pcscCancel(*context));

	return NULL;
}

/* Transmit data to card
 * @param handle
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
napi_value transmit(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDHANDLE *handle;
	CHECK(napi_get_value_external(env, args[0], (void **)&handle));
	{
		bool result;
		CHECK(napi_is_buffer(env, args[1], &result));
		if (!result)
		{
			CHECK(napi_throw_type_error(env, NULL, "Wrong argument type, expected Buffer"));
			return NULL;
		}
	}
	size_t sendSize;
	BYTE *sendData;
	CHECK(napi_get_buffer_info(env, args[1], (void **)&sendData, &sendSize));

	DWORD recvSize = MAX_BUFFER_SIZE;
	LPBYTE recvData;
	CATCH(pcscTransmit(*handle, sendData, sendSize, &recvData, &recvSize));

	LPBYTE buffer;
	napi_value ret_val;
	CHECK(napi_create_buffer(env, recvSize, (void **)&buffer, &ret_val));
	memcpy(buffer, recvData, recvSize);
	free(recvData);
	return ret_val;
}

/* Get card status
 * @param handle
 * @return state
 */
napi_value getStatus(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDHANDLE *handle;
	CHECK(napi_get_value_external(env, args[0], (void **)&handle));

	STATE *state = malloc(sizeof(STATE));
	CATCH(pcscGetStatus(*handle, state));

	napi_value ret_val;
	CHECK(napi_create_external(env, state, destructor, NULL, &ret_val));
	return ret_val;
}

/* Wait until global state is changed
 * @param context
 * @return state
 */
napi_value waitUntilGlobalChange(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));

	STATE *newState = malloc(sizeof(STATE));
	CATCH(pcscWaitUntilGlobalChange(*context, newState));

	napi_value ret_val;
	CHECK(napi_create_external(env, newState, destructor, NULL, &ret_val));
	return ret_val;
}

/* Wait until reader state is changed
 * @param context
 * @param string Reader name
 * @param state Current state
 * @return state New srate
 */
napi_value waitUntilReaderChange(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));
	char readerName[50];
	CHECK(napi_get_value_string_utf8(env, args[1], readerName, sizeof(readerName), NULL));
	STATE *curState;
	CHECK(napi_get_value_external(env, args[1], (void **)&curState));

	STATE *newState = malloc(sizeof(STATE));
	CATCH(pcscWaitUntilReaderChange(*context, *curState, readerName, newState));

	napi_value ret_val;
	CHECK(napi_create_external(env, newState, destructor, NULL, &ret_val));
	return ret_val;
}

/* Wait until a new reader is connected
 * @param context
 * @return string Reader name
 */
napi_value waitUntilReaderConnected(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));

	LPSTR buffer;
	DWORD bufsize;
	CATCH(pcscWaitUntilReaderConnected(*context, &buffer, &bufsize));

	napi_value ret_val;
	CHECK(napi_create_string_utf8(env, buffer, bufsize, &ret_val));
	return ret_val;
}

/* Wait until desired reader state
 * @param context
 * @param string readerName
 * @param state Desired state
 */
napi_value waitUntilReaderState(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(3)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_string)
	CHECK_ARGUMENT_TYPE(2, napi_external)
	SCARDCONTEXT *context;
	CHECK(napi_get_value_external(env, args[0], (void **)&context));
	CATCH(pcscIsContextValid(*context));
	char readerName[50];
	CHECK(napi_get_value_string_utf8(env, args[1], readerName, sizeof(readerName), NULL));
	STATE *state;
	CHECK(napi_get_value_external(env, args[2], (void **)&state));

	CATCH(pcscWaitUntilReaderState(*context, readerName, *state));

	return NULL;
}

napi_value Init(napi_env env, napi_value exports)
{
	napi_value constant_state_empty, constant_state_present;
	napi_create_external(env, &stateEmpty, NULL, NULL, &constant_state_empty);
	napi_create_external(env, &statePresent, NULL, NULL, &constant_state_present);
	napi_property_descriptor properties[14] = {
		DECLARE_NAPI_METHOD("establish", establish),
		DECLARE_NAPI_METHOD("release", release),
		DECLARE_NAPI_METHOD("getReaders", getReaders),
		DECLARE_NAPI_METHOD("connect", connectCard),
		DECLARE_NAPI_METHOD("disconnect", disconnectCard),
		DECLARE_NAPI_METHOD("cancel", cancel),
		DECLARE_NAPI_METHOD("transmit", transmit),
		DECLARE_NAPI_METHOD("getStatus", getStatus),
		DECLARE_NAPI_METHOD("waitUntilGlobalChange", waitUntilGlobalChange),
		DECLARE_NAPI_METHOD("waitUntilReaderChange", waitUntilReaderChange),
		DECLARE_NAPI_METHOD("waitUntilReaderConnected", waitUntilReaderConnected),
		DECLARE_NAPI_METHOD("waitUntilReaderState", waitUntilReaderState),
		DECLARE_NAPI_CONSTANT("stateEmpty", constant_state_empty),
		DECLARE_NAPI_CONSTANT("statePresent", constant_state_present),
	};
	CHECK(napi_define_properties(env, exports, 14, properties));

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)

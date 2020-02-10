#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#define NAPI_VERSION 4
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

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define THROW(error) napi_throw_error(env, NULL, error)

#define THROW_PCSC(err_code) THROW(pcsc_stringify_error(err_code))

#define THROW_NAPI(err_code) THROW("Internal error in file " __FILE__ " on line " TOSTRING(__LINE__))

#define CHECK_PCSC(expr, ...)   \
	{                           \
		LONG err = expr;        \
		if (err)                \
		{                       \
			THROW_PCSC(err);    \
			return __VA_ARGS__; \
		}                       \
	}
#define CHECK_NAPI(expr, ...)   \
	{                           \
		napi_status err = expr; \
		if (err)                \
		{                       \
			THROW_NAPI(err);    \
			return __VA_ARGS__; \
		}                       \
	}

#define CHECK_ARGUMENT_COUNT(count)                                                       \
	size_t argc = count;                                                                  \
	napi_value args[count];                                                               \
	CHECK_NAPI(napi_get_cb_info(env, info, &argc, args, NULL, NULL), NULL);               \
	if (argc != count)                                                                    \
	{                                                                                     \
		CHECK_NAPI(napi_throw_error(env, NULL, "Expected " #count " argument(s)"), NULL); \
		return NULL;                                                                      \
	}

#define CHECK_ARGUMENT_TYPE(i, type)                                                  \
	{                                                                                 \
		napi_valuetype actual_type;                                                   \
		CHECK_NAPI(napi_typeof(env, args[i], &actual_type), NULL);                    \
		if (actual_type != type)                                                      \
		{                                                                             \
			napi_throw_type_error(env, NULL, "Wrong argument type, expected " #type); \
			return NULL;                                                              \
		}                                                                             \
	}

#define CHECK_ARGUMENT_BUFFER(i)                                                                        \
	{                                                                                                   \
		bool result;                                                                                    \
		CHECK_NAPI(napi_is_buffer(env, args[i], &result), NULL);                                        \
		if (!result)                                                                                    \
		{                                                                                               \
			CHECK_NAPI(napi_throw_type_error(env, NULL, "Wrong argument type, expected Buffer"), NULL); \
			return NULL;                                                                                \
		}                                                                                               \
	}

#define DECLARE_NAPI_METHOD(name, func)                        \
	{                                                          \
		name, NULL, func, NULL, NULL, NULL, napi_default, NULL \
	}

#define DECLARE_NAPI_CONSTANT(name, value)                      \
	{                                                           \
		name, NULL, NULL, NULL, NULL, value, napi_default, NULL \
	}

typedef struct async_exec_data
{
	napi_threadsafe_function callback;
	napi_async_work work;
	void *argument;
} Async_exec_data;

typedef struct async_return_data
{
	void *ret_val;
	LONG error;
} Async_return_data;

void destructor(napi_env env, void *finalize_data, void *finalize_hint)
{
	free(finalize_data);
}

// Construct javascript Buffer from data array
napi_value constructBuffer(napi_env env, BYTE *data, size_t length)
{
	void *buffer;
	napi_value ret_val;
	CHECK_NAPI(napi_create_buffer(env, length, &buffer, &ret_val), NULL);
	memcpy(buffer, data, length);
	return ret_val;
}

/* Establish context
 * This function needs to be called first.
 * @return context
 */
napi_value establish(napi_env env, napi_callback_info info)
{
	SCARDCONTEXT *context = malloc(sizeof(SCARDCONTEXT));
	CHECK_PCSC(pcscEstablish(context), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, context, destructor, NULL, &ret_val), NULL);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);

	CHECK_PCSC(pcscRelease(*context), NULL)

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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);

	DWORD bufSize = 0;
	char *buffer;
	CHECK_PCSC(pcscGetReaders(*context, &buffer, &bufSize), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_string_utf8(env, buffer, bufSize, &ret_val), NULL);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);
	char readerName[50];
	CHECK_NAPI(napi_get_value_string_utf8(env, args[1], readerName, sizeof(readerName), NULL), NULL);

	SCARDHANDLE *handle = malloc(sizeof(SCARDHANDLE));
	CHECK_PCSC(pcscConnect(*context, readerName, handle), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, handle, destructor, NULL, &ret_val), NULL);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&handle), NULL);

	CHECK_PCSC(pcscDisconnect(*handle), NULL);

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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);

	CHECK_PCSC(pcscCancel(*context), NULL);

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
	CHECK_ARGUMENT_BUFFER(1)
	SCARDHANDLE *handle;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&handle), NULL);
	size_t sendSize;
	BYTE *sendData;
	CHECK_NAPI(napi_get_buffer_info(env, args[1], (void **)&sendData, &sendSize), NULL);

	DWORD recvSize = MAX_BUFFER_SIZE;
	BYTE recvData[MAX_BUFFER_SIZE];
	CHECK_PCSC(pcscTransmit(*handle, sendData, sendSize, recvData, &recvSize), NULL);

	return constructBuffer(env, recvData, recvSize);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&handle), NULL);

	STATE *state = malloc(sizeof(STATE));
	CHECK_PCSC(pcscGetStatus(*handle, state), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, state, destructor, NULL, &ret_val), NULL);
	return ret_val;
}

/* Send direct command to the reader
 * @param handle
 * @param command
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
napi_value directCommand(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(3)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_external)
	CHECK_ARGUMENT_BUFFER(2)
	SCARDHANDLE *handle;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&handle), NULL);
	DWORD *command;
	CHECK_NAPI(napi_get_value_external(env, args[1], (void **)&command), NULL);
	size_t sendSize;
	BYTE *sendData;
	CHECK_NAPI(napi_get_buffer_info(env, args[2], (void **)&sendData, &sendSize), NULL);

	DWORD recvSize = MAX_BUFFER_SIZE;
	BYTE recvData[MAX_BUFFER_SIZE];
	CHECK_PCSC(pcscDirectCommand(*handle, *command, sendData, sendSize, recvData, &recvSize), NULL);

	return constructBuffer(env, recvData, recvSize);
}

// Called in main thread, executes callback into JS
void jsCallbackCaller(napi_env env, napi_value js_cb, void *context, void *data)
{
	Async_return_data *async_data = (Async_return_data *)data;
	// env can be null if Node.js is shutting down
	if (env != NULL)
	{
		if (async_data->error)
		{
			THROW_NAPI(async_data->error);
		}
		else if (async_data->ret_val != NULL)
		{
			napi_value global_scope;
			napi_value ret_val;
			napi_status napi_error;
			napi_error = napi_get_global(env, &global_scope);
			if (!napi_error)
			{
				napi_error = napi_create_string_utf8(env, async_data->ret_val, strlen(async_data->ret_val), &ret_val);
			}
			if (!napi_error)
			{
				// blocking call to JS
				napi_error = napi_call_function(env, global_scope, js_cb, 1, &ret_val, NULL);
			}
			free(async_data->ret_val);
			if (napi_error)
			{
				THROW_NAPI(napi_error);
			}
		}
	}
	free(async_data);
}

// Called in worker thread, blocks and send callback asynchronously
void globalChangeExecute(napi_env _, void *data)
{
	Async_exec_data *exec_data = (Async_exec_data *)data;
	SCARDCONTEXT *context = exec_data->argument;
	LONG pcsc_error;
	pcsc_error = pcscIsContextValid(*context);
	if (!pcsc_error)
	{
		napi_acquire_threadsafe_function(exec_data->callback);
		LPSTR buffer;
		DWORD bufsize;
		pcsc_error = pcscWaitUntilReaderConnected(*context, &buffer, &bufsize);
		while (!pcsc_error)
		{
			// Shedule call into main thread
			Async_return_data *return_data = malloc(sizeof(Async_return_data));
			return_data->ret_val = buffer;
			return_data->error = pcsc_error;
			napi_call_threadsafe_function(exec_data->callback, return_data, napi_tsfn_nonblocking);

			STATE state;
			pcsc_error = pcscWaitUntilGlobalChange(*context, &state);
			pcsc_error = pcscWaitUntilReaderConnected(*context, &buffer, &bufsize);
			pcsc_error = pcscGetReaders(*context, &buffer, &bufsize);
		}
		napi_release_threadsafe_function(exec_data->callback, napi_tsfn_release);
	}
}

// Called in main thread, worker destructor
void globalChangeFinish(napi_env env, napi_status status, void *data)
{
	Async_exec_data *exec_data = (Async_exec_data *)data;
	CHECK_NAPI(napi_delete_async_work(env, exec_data->work))
	free(exec_data);
}

/* Subscribe to global change callback
 * @param context
 * @param callback(string readerName)
 */
napi_value globalChangeSubscribe(napi_env env, napi_callback_info info)
{
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_function)
	// All napi_values have to be extracted or referenced to avoid GC
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);

	Async_exec_data *exec_data = malloc(sizeof(Async_exec_data));
	// TODO: check context is no being GC'd
	exec_data->argument = context;
	napi_value work_name;
	CHECK_NAPI(napi_create_string_utf8(env, "pcscbinding.globalChangeSubscribe", NAPI_AUTO_LENGTH, &work_name), NULL);

	// Bind JS callback to native 'jsCallbackCaller'
	CHECK_NAPI(napi_create_threadsafe_function(env, args[1], NULL, work_name, 0, 1, NULL, NULL, NULL, jsCallbackCaller, &exec_data->callback), NULL);

	// Create async worker
	CHECK_NAPI(napi_create_async_work(env, NULL, work_name, globalChangeExecute, globalChangeFinish, exec_data, &exec_data->work), NULL)
	CHECK_NAPI(napi_queue_async_work(env, exec_data->work), NULL)

	return NULL;
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);

	STATE *newState = malloc(sizeof(STATE));
	CHECK_PCSC(pcscWaitUntilGlobalChange(*context, newState), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, newState, destructor, NULL, &ret_val), NULL);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);
	char readerName[50];
	CHECK_NAPI(napi_get_value_string_utf8(env, args[1], readerName, sizeof(readerName), NULL), NULL);
	STATE *curState;
	CHECK_NAPI(napi_get_value_external(env, args[1], (void **)&curState), NULL);

	STATE *newState = malloc(sizeof(STATE));
	CHECK_PCSC(pcscWaitUntilReaderChange(*context, *curState, readerName, newState), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, newState, destructor, NULL, &ret_val), NULL);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);

	LPSTR buffer;
	DWORD bufsize;
	CHECK_PCSC(pcscWaitUntilReaderConnected(*context, &buffer, &bufsize), NULL);

	napi_value ret_val;
	CHECK_NAPI(napi_create_string_utf8(env, buffer, bufsize, &ret_val), NULL);
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
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **)&context), NULL);
	CHECK_PCSC(pcscIsContextValid(*context), NULL);
	char readerName[50];
	CHECK_NAPI(napi_get_value_string_utf8(env, args[1], readerName, sizeof(readerName), NULL), NULL);
	STATE *state;
	CHECK_NAPI(napi_get_value_external(env, args[2], (void **)&state), NULL);

	CHECK_PCSC(pcscWaitUntilReaderState(*context, readerName, *state), NULL);

	return NULL;
}

napi_value Init(napi_env env, napi_value exports)
{
	napi_value constant_state_empty, constant_state_present;
	napi_create_external(env, &stateEmpty, NULL, NULL, &constant_state_empty);
	napi_create_external(env, &statePresent, NULL, NULL, &constant_state_present);
	napi_property_descriptor properties[16] = {
		DECLARE_NAPI_METHOD("establish", establish),
		DECLARE_NAPI_METHOD("release", release),
		DECLARE_NAPI_METHOD("getReaders", getReaders),
		DECLARE_NAPI_METHOD("connect", connectCard),
		DECLARE_NAPI_METHOD("disconnect", disconnectCard),
		DECLARE_NAPI_METHOD("cancel", cancel),
		DECLARE_NAPI_METHOD("transmit", transmit),
		DECLARE_NAPI_METHOD("getStatus", getStatus),
		DECLARE_NAPI_METHOD("directCommand", directCommand),
		DECLARE_NAPI_METHOD("globalChangeSubscribe", globalChangeSubscribe),
		DECLARE_NAPI_METHOD("waitUntilGlobalChange", waitUntilGlobalChange),
		DECLARE_NAPI_METHOD("waitUntilReaderChange", waitUntilReaderChange),
		DECLARE_NAPI_METHOD("waitUntilReaderConnected", waitUntilReaderConnected),
		DECLARE_NAPI_METHOD("waitUntilReaderState", waitUntilReaderState),
		DECLARE_NAPI_CONSTANT("stateEmpty", constant_state_empty),
		DECLARE_NAPI_CONSTANT("statePresent", constant_state_present),
	};
	CHECK_NAPI(napi_define_properties(env, exports, 16, properties), NULL);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)

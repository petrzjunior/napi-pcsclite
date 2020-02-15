#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define NAPI_VERSION 4
#include <node_api.h>
#include "pcsclite.h"

#ifndef MAX_READERNAME
#define MAX_READERNAME 128
#endif

#define MAX_BUFFER_SIZE 264

STATE state_present = SCARD_STATE_PRESENT;
STATE state_empty = SCARD_STATE_EMPTY;
#ifdef _WIN32

#include <stdio.h>

char *pcsc_stringify_error(LONG err)
{
	// TODO: Add full error messages
	char *buffer = malloc(25 * sizeof(char));
	snprintf(buffer, sizeof(buffer), "Error code: %li\n", err);
	return buffer;
}
#endif

#ifdef __GNUC__
#define UNUSED(name) __attribute__((unused)) name##_unused
#else
#define UNUSED(name) name##_unused
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
    size_t argc = (count);                                                                \
    napi_value args[(count) > 0 ? (count) : 1];                                             \
    CHECK_NAPI(napi_get_cb_info(env, info, &argc, args, NULL, NULL), NULL);               \
    if (argc != ((count)))                                                                \
    {                                                                                     \
        CHECK_NAPI(napi_throw_error(env, NULL, "Expected " #count " argument(s)"), NULL); \
        return NULL;                                                                      \
    }

#define CHECK_ARGUMENT_TYPE(i, type)                                                           \
    {                                                                                          \
        napi_valuetype actual_type;                                                            \
        CHECK_NAPI(napi_typeof(env, args[i], &actual_type), NULL);                             \
        if (actual_type != (type))                                                             \
        {                                                                                      \
            napi_throw_type_error(env, NULL, "Argument #" #i ": wrong type, expected " #type); \
            return NULL;                                                                       \
        }                                                                                      \
    }

#define CHECK_ARGUMENT_BUFFER(i)                                                               \
    {                                                                                          \
        bool result;                                                                           \
        CHECK_NAPI(napi_is_buffer(env, args[i], &result), NULL);                               \
        if (!result)                                                                           \
        {                                                                                      \
            napi_throw_type_error(env, NULL, "Argument #" #i ": wrong type, expected Buffer"); \
            return NULL;                                                                       \
        }                                                                                      \
    }

#define DECLARE_NAPI_METHOD(name, func)                        \
    {                                                          \
        name, NULL, func, NULL, NULL, NULL, napi_default, NULL \
    }

#define DECLARE_NAPI_CONSTANT(name, value)                      \
    {                                                           \
        name, NULL, NULL, NULL, NULL, value, napi_default, NULL \
    }

struct async_exec_data {
	napi_async_work work;
	napi_deferred deferred;
	LONG error;
	SCARDCONTEXT context;
	STATE state;
	LPCSTR reader_name;
};

napi_value construct_error(napi_env env, const char *text) {
	napi_value err_text, error;
	napi_create_string_utf8(env, text, strlen(text), &err_text);
	napi_create_error(env, NULL, err_text, &error);
	return error;
}

void destructor(napi_env UNUSED(env), void *finalize_data, void *UNUSED(finalize_hint)) {
	free(finalize_data);
}

// Construct javascript Buffer from data array
napi_value construct_buffer(napi_env env, BYTE *data, size_t length) {
	void *buffer;
	napi_value ret_val;
	CHECK_NAPI(napi_create_buffer(env, length, &buffer, &ret_val), NULL)
	memcpy(buffer, data, length);
	return ret_val;
}

/* Establish context
 * This function needs to be called first.
 * @return context
 */
napi_value establish(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(0)
	SCARDCONTEXT *context = malloc(sizeof(SCARDCONTEXT));
	CHECK_PCSC(pcsc_establish(context), NULL)

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, context, destructor, NULL, &ret_val), NULL)
	return ret_val;
}

/* Release context
 * This function needs to be called last.
 * @param context
 */
napi_value release(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)
	CHECK_PCSC(pcsc_is_context_valid(*context), NULL)

	CHECK_PCSC(pcsc_release(*context), NULL)

	return NULL;
}

/* Get connected readers
 * @param context
 * @return array<string> readers' names
 */
napi_value get_readers(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)
	CHECK_PCSC(pcsc_is_context_valid(*context), NULL)

	DWORD buf_size = 0;
	char *buffer;
	CHECK_PCSC(pcsc_get_readers(*context, &buffer, &buf_size), NULL)

	napi_value ret_val;
	CHECK_NAPI(napi_create_array(env, &ret_val), NULL)
	if (buf_size > 0) {
		char *iterator = buffer;
		// There is an extra null character at the end
		for (uint32_t i = 0; *iterator; i++) {
			// Names are null-terminated, split them into array of strings
			size_t length = strlen(iterator);
			napi_value token;
			CHECK_NAPI(napi_create_string_utf8(env, iterator, length, &token), NULL)
			CHECK_NAPI(napi_set_element(env, ret_val, i, token), NULL)
			iterator += length + 1;
		}
		free(buffer);
	}
	return ret_val;
}

/* Connect to card
 * @param context
 * @param string Reader name
 * @return handle Card handle
 */
napi_value connect_card(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_string)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)
	CHECK_PCSC(pcsc_is_context_valid(*context), NULL)
	char reader_name[MAX_READERNAME];
	CHECK_NAPI(napi_get_value_string_utf8(env, args[1], reader_name, sizeof(reader_name), NULL), NULL)

	SCARDHANDLE *handle = malloc(sizeof(SCARDHANDLE));
	CHECK_PCSC(pcsc_connect(*context, reader_name, handle), NULL)

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, handle, destructor, NULL, &ret_val), NULL)
	return ret_val;
}

/* Disconnect from card
 * @param handle
 */
napi_value disconnect_card(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDHANDLE *handle;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &handle), NULL)

	CHECK_PCSC(pcsc_disconnect(*handle), NULL)

	return NULL;
}

/* Cancel blocking wait
 * @param context
 */
napi_value cancel(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)
	CHECK_PCSC(pcsc_is_context_valid(*context), NULL)

	CHECK_PCSC(pcsc_cancel(*context), NULL)

	return NULL;
}

/* Transmit data to card
 * @param handle
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
napi_value transmit(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_BUFFER(1)
	SCARDHANDLE *handle;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &handle), NULL)
	size_t send_size;
	BYTE *send_data;
	CHECK_NAPI(napi_get_buffer_info(env, args[1], (void **) &send_data, &send_size), NULL)

	DWORD recv_size = MAX_BUFFER_SIZE;
	BYTE recv_data[MAX_BUFFER_SIZE];
	CHECK_PCSC(pcsc_transmit(*handle, send_data, send_size, recv_data, &recv_size), NULL)

	return construct_buffer(env, recv_data, recv_size);
}

/* Get card status
 * @param context
 * @param string Reader name
 * @return state
 */
napi_value get_status(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_string)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)
	char reader_name[MAX_READERNAME];
	CHECK_NAPI(napi_get_value_string_utf8(env, args[1], reader_name, sizeof(reader_name), NULL), NULL)

	STATE *state = malloc(sizeof(STATE));
	CHECK_PCSC(pcsc_get_status(*context, reader_name, state), NULL)

	napi_value ret_val;
	CHECK_NAPI(napi_create_external(env, state, destructor, NULL, &ret_val), NULL)
	return ret_val;
}

/* Send direct command to the reader
 * @param handle
 * @param command
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
napi_value direct_command(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(3)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_external)
	CHECK_ARGUMENT_BUFFER(2)
	SCARDHANDLE *handle;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &handle), NULL)
	DWORD *command;
	CHECK_NAPI(napi_get_value_external(env, args[1], (void **) &command), NULL)
	size_t send_size;
	BYTE *send_data;
	CHECK_NAPI(napi_get_buffer_info(env, args[2], (void **) &send_data, &send_size), NULL)

	DWORD recv_size = MAX_BUFFER_SIZE;
	BYTE recv_data[MAX_BUFFER_SIZE];
	CHECK_PCSC(pcsc_direct_command(*handle, *command, send_data, send_size, recv_data, &recv_size), NULL)

	return construct_buffer(env, recv_data, recv_size);
}

void global_status_execute(napi_env UNUSED(env), void *data) {
	struct async_exec_data *exec_data = (struct async_exec_data *) data;
	// Call blocking function
	exec_data->error = pcsc_wait_until_global_change(exec_data->context, &exec_data->state);

}

void global_status_finish(napi_env env, napi_status status, void *data) {
	struct async_exec_data *exec_data = (struct async_exec_data *) data;
	if (status == napi_cancelled || exec_data->error) {
		// Reject promise
		napi_value rejection = construct_error(env, pcsc_stringify_error(exec_data->error));
		napi_reject_deferred(env, exec_data->deferred, rejection);
	} else {
		// Resolve promise
		napi_value resolution;
		napi_get_boolean(env, (bool) (exec_data->state & SCARD_STATE_CHANGED), &resolution);
		napi_resolve_deferred(env, exec_data->deferred, resolution);
	}
	napi_delete_async_work(env, exec_data->work);
	free(exec_data);
}

/* Get global status change
 * @param context
 * @return promise(bool, err) Did change happen
 */
napi_value get_global_status_change(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)

	napi_deferred deferred;
	napi_value promise;
	CHECK_NAPI(napi_create_promise(env, &deferred, &promise), NULL)

	napi_value work_name;
	CHECK_NAPI(napi_create_string_utf8(env, "pcscbinding.getGlobalStatusChange", NAPI_AUTO_LENGTH, &work_name), NULL)
	struct async_exec_data *exec_data = malloc(sizeof(struct async_exec_data));
	exec_data->context = *context;
	exec_data->deferred = deferred;

	// Create async worker
	CHECK_NAPI(napi_create_async_work(env, NULL, work_name, global_status_execute, global_status_finish, exec_data,
	                                  &exec_data->work), NULL)
	CHECK_NAPI(napi_queue_async_work(env, exec_data->work), NULL)

	return promise;
}

void reader_status_execute(napi_env UNUSED(env), void *data) {
	struct async_exec_data *exec_data = (struct async_exec_data *) data;
	// Call blocking function
	exec_data->error = pcsc_wait_until_reader_change(exec_data->context, exec_data->state, exec_data->reader_name,
	                                                 &exec_data->state);
}

void reader_status_finish(napi_env env, napi_status status, void *data) {
	struct async_exec_data *exec_data = (struct async_exec_data *) data;
	if (status == napi_cancelled || exec_data->error) {
		// Reject promise
		napi_value rejection = construct_error(env, pcsc_stringify_error(exec_data->error));
		napi_reject_deferred(env, exec_data->deferred, rejection);
	} else {
		// Resolve promise
		napi_value resolution;
		STATE *new_state = malloc(sizeof(STATE));
		napi_create_external(env, new_state, destructor, NULL, &resolution);
		napi_resolve_deferred(env, exec_data->deferred, resolution);
	}
	napi_delete_async_work(env, exec_data->work);
	free(exec_data);
}

/* Get reader status change
 * @param context
 * @param readerName
 * @return promise(state, err) New state
 */
napi_value get_reader_status_change(napi_env env, napi_callback_info info) {
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, napi_external)
	CHECK_ARGUMENT_TYPE(1, napi_string)
	SCARDCONTEXT *context;
	CHECK_NAPI(napi_get_value_external(env, args[0], (void **) &context), NULL)
	char *reader_name = malloc(MAX_READERNAME);
	CHECK_NAPI(napi_get_value_string_utf8(env, args[1], reader_name, MAX_READERNAME, NULL), NULL)

	napi_deferred deferred;
	napi_value promise;
	CHECK_NAPI(napi_create_promise(env, &deferred, &promise), NULL)

	napi_value work_name;
	CHECK_NAPI(napi_create_string_utf8(env, "pcscbinding.getReaderStatusChange", NAPI_AUTO_LENGTH, &work_name), NULL)
	struct async_exec_data *exec_data = malloc(sizeof(struct async_exec_data));
	exec_data->context = *context;
	exec_data->reader_name = reader_name;
	exec_data->deferred = deferred;

	// Create async worker
	CHECK_NAPI(napi_create_async_work(env, NULL, work_name, reader_status_execute, reader_status_finish, exec_data,
	                                  &exec_data->work), NULL)
	CHECK_NAPI(napi_queue_async_work(env, exec_data->work), NULL)

	return promise;
}

napi_value init(napi_env env, napi_value exports) {
	napi_value constant_state_empty, constant_state_present;
	napi_create_external(env, &state_empty, NULL, NULL, &constant_state_empty);
	napi_create_external(env, &state_present, NULL, NULL, &constant_state_present);
	napi_property_descriptor properties[13] = {
			DECLARE_NAPI_METHOD("establish", establish),
			DECLARE_NAPI_METHOD("release", release),
			DECLARE_NAPI_METHOD("getReaders", get_readers),
			DECLARE_NAPI_METHOD("connect", connect_card),
			DECLARE_NAPI_METHOD("disconnect", disconnect_card),
			DECLARE_NAPI_METHOD("cancel", cancel),
			DECLARE_NAPI_METHOD("transmit", transmit),
			DECLARE_NAPI_METHOD("getStatus", get_status),
			DECLARE_NAPI_METHOD("directCommand", direct_command),
			DECLARE_NAPI_METHOD("getGlobalStatusChange", get_global_status_change),
			DECLARE_NAPI_METHOD("getReaderStatusChange", get_reader_status_change),
			DECLARE_NAPI_CONSTANT("stateEmpty", constant_state_empty),
			DECLARE_NAPI_CONSTANT("statePresent", constant_state_present),
	};
	CHECK_NAPI(napi_define_properties(env, exports, 13, properties), NULL)

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init
)

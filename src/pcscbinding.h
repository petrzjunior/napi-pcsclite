#pragma once

#include <napi.h>

#include "pcsclite.h"

#ifdef _WIN32
std::string pcsc_stringify_error(LONG code)
{
	return std::string("Error code: ") + std::to_string(code);
}
#endif

#define CATCH(expr)                                                                        \
	{                                                                                      \
		LONG err = expr;                                                                   \
		if (err)                                                                           \
		{                                                                                  \
			Napi::Error::New(env, pcsc_stringify_error(err)).ThrowAsJavaScriptException(); \
			return env.Null();                                                             \
		}                                                                                  \
	}

template <typename T>
void destructor(Napi::Env env, T *value);

#define CHECK_ARGUMENT_COUNT(len)                                                                              \
	if (info.Length() + 1 < len + 1)                                                                           \
	{                                                                                                          \
		Napi::TypeError::New(env, "Too few arguments supplied, expected " #len).ThrowAsJavaScriptException();  \
		return env.Null();                                                                                     \
	}                                                                                                          \
	if (info.Length() > len)                                                                                   \
	{                                                                                                          \
		Napi::TypeError::New(env, "Too many arguments supplied, expected " #len).ThrowAsJavaScriptException(); \
		return env.Null();                                                                                     \
	}

#define CHECK_ARGUMENT_TYPE(i, type)                                                                             \
	if (!info[i].Is##type())                                                                                     \
	{                                                                                                            \
		Napi::TypeError::New(env, "Argument #" #i " error, expected type: " #type).ThrowAsJavaScriptException(); \
		return env.Null();                                                                                       \
	}

extern STATE statePresent;
extern STATE stateEmpty;

/* Establish context
 * This function needs to be called first.
 * @return context
 */
Napi::Value establish(const Napi::CallbackInfo &info);

/* Release context
 * This function needs to be called last.
 * @param context
 */
Napi::Value release(const Napi::CallbackInfo &info);

/* Get connected readers
 * @param context
 * @return string Readers' names in a string
 */
Napi::Value getReaders(const Napi::CallbackInfo &info);

/* Connect to card
 * @param context
 * @param string Reader name
 * @return handle Card handle
 */
Napi::Value connect(const Napi::CallbackInfo &info);

/* Disconnect from card
 * @param handle
 */
Napi::Value disconnect(const Napi::CallbackInfo &info);

/* Transmit data to card
 * @param handle
 * @param Buffer<uint8_t> sendData
 * @return Buffer<uint8_t> recvData
 */
Napi::Value transmit(const Napi::CallbackInfo &info);

/* Get card status
 * @param handle
 * @return state
 */
Napi::Value getStatus(const Napi::CallbackInfo &info);

/* Wait until global state is changed
 * @param context
 * @return state
 */
Napi::Value waitUntilGlobalChange(const Napi::CallbackInfo &info);

/* Wait until reader state is changed
 * @param context
 * @param string Reader name
 * @param state Current state
 * @return state New srate
 */
Napi::Value waitUntilReaderChange(const Napi::CallbackInfo &info);

/* Wait until a new reader is connected
 * @param context
 * @return string Reader name
 */
Napi::Value waitUntilReaderConnected(const Napi::CallbackInfo &info);

/* Wait until desired reader state
 * @param context
 * @param string readerName
 * @param state Desired state
 */
Napi::Value waitUntilReaderState(const Napi::CallbackInfo &info);

class pcscEmitter : public Napi::ObjectWrap<pcscEmitter>
{
private:
	SCARDCONTEXT context = 0;

public:
	static Napi::FunctionReference constructor;
	pcscEmitter(const Napi::CallbackInfo &info) : Napi::ObjectWrap<pcscEmitter>(info) {}

	Napi::Value watch(const Napi::CallbackInfo &info);

	static void Initialize(Napi::Env &env, Napi::Object &target);
};

class pcscReader : public Napi::ObjectWrap<pcscReader>
{
private:
	SCARDCONTEXT *context;
	std::string readerName;

public:
	static Napi::FunctionReference constructor;

	pcscReader(const Napi::CallbackInfo &info);

	static void Initialize(Napi::Env &env, Napi::Object &target);

	/* Send data to present card
	 * @param Buffer<uint8_t> Send data
	 * @return Buffer<uint8_t> Received data
	 */
	Napi::Value send(const Napi::CallbackInfo &info);
};
#include <napi.h>

#include <stdio.h>
#include <string>

#include <winscard.h>
#include <wintypes.h>

#include "pcsclite.h"

#define CATCH(err)                                                                     \
	if (err)                                                                           \
	{                                                                                  \
		Napi::Error::New(env, pcsc_stringify_error(err)).ThrowAsJavaScriptException(); \
		return env.Null();                                                             \
	}

#define READER_NOTIFICATION "\\\\?PnP?\\Notification"

template <typename T>
void deleteValue(Napi::Env env, T *value)
{
	printf("DEBUG: Finalizer called for value\n");
	delete value;
}

template <typename T>
void deleteArray(Napi::Env env, T array[])
{
	printf("DEBUG Finalizer called for array\n");
	delete[] array;
}

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

/* Estabilish context
 * This function needs to be called first.
 * @return context
 */
Napi::Value pcscEstabilish(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(0)

	SCARDCONTEXT *context = new SCARDCONTEXT();
	CATCH(SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, context));

	return Napi::External<SCARDCONTEXT>::New(env, context, deleteValue<SCARDCONTEXT>);
}

/* Release context
 * This function needs to be called last.
 * @param context
 */
Napi::Value pcscRelease(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, External)
	SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();

	CATCH(SCardReleaseContext(*context));
	return env.Null();
}

/* Get connected readers
 * @param context
 * @return string Readers' names in a string
 */
Napi::Value pcscGetReaders(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, External)
	SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();

	DWORD bufSize = 0;
	CATCH(SCardListReaders(*context, NULL, NULL, &bufSize));

	if (bufSize == 0)
	{
		return env.Null();
	}
	LPSTR buffer = new char[bufSize];
	if (buffer == NULL)
	{
		CATCH(SCARD_E_NO_MEMORY)
		return env.Null();
	}
	CATCH(SCardListReaders(*context, NULL, buffer, &bufSize));

	return Napi::String::New(env, buffer);
}

/* Connect to card
 * @param context
 * @param string Reader name
 * @return handle Card handle
 */
Napi::Value pcscConnect(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, External)
	CHECK_ARGUMENT_TYPE(1, String)
	SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
	std::string reader = info[1].As<Napi::String>().Utf8Value();

	DWORD activeProtocol;
	SCARDHANDLE *handle = new SCARDHANDLE();
	CATCH(SCardConnect(*context, reader.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, handle, &activeProtocol));
	return Napi::External<SCARDHANDLE>::New(env, handle, deleteValue<SCARDHANDLE>);
}

/* Disconnect from card
 * @param handle
 */
Napi::Value pcscDisconnect(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, External)
	SCARDHANDLE *handle = info[0].As<Napi::External<SCARDHANDLE>>().Data();

	CATCH(SCardDisconnect(*handle, SCARD_UNPOWER_CARD));
	return env.Null();
}
/* Transmit data to card
 * @param handle
 * @param ArrayBuffer sendData
 * @return ArrayBuffer recvData
 */
Napi::Value pcscTransmit(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, External)
	CHECK_ARGUMENT_TYPE(1, ArrayBuffer)
	SCARDHANDLE *handle = info[0].As<Napi::External<SCARDHANDLE>>().Data();
	Napi::ArrayBuffer sendData = info[1].As<Napi::ArrayBuffer>();

	DWORD recvSize = MAX_BUFFER_SIZE;
	BYTE *recvData = new BYTE[recvSize];
	CATCH(SCardTransmit(*handle, SCARD_PCI_T0, (BYTE *)sendData.Data(), (DWORD)sendData.ByteLength(), NULL, recvData, &recvSize));
	return Napi::ArrayBuffer::New(env, recvData, recvSize, deleteArray<void>);
}

/* Get card status
 * @param handle
 * @return state
 */
Napi::Value pcscGetStatus(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(1)
	CHECK_ARGUMENT_TYPE(0, External)
	SCARDHANDLE *handle = info[0].As<Napi::External<SCARDHANDLE>>().Data();

	DWORD *state = new DWORD();
	CATCH(SCardStatus(*handle, NULL, NULL, state, NULL, NULL, NULL));
	return Napi::External<DWORD>::New(env, state, deleteArray<DWORD>);
}

/* Wait until global state is changed
 * @param context
 * @param handle
 * @return state
 */
Napi::Value pcscWaitUntilGlobalState(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, External)
	CHECK_ARGUMENT_TYPE(1, External)
	SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
	SCARDHANDLE *handle = info[1].As<Napi::External<SCARDHANDLE>>().Data();

	SCARD_READERSTATE state;
	state.szReader = READER_NOTIFICATION;
	state.dwCurrentState = SCARD_STATE_UNAWARE;

	CATCH(SCardGetStatusChange(*context, INFINITE, &state, 1));

	DWORD *newState = new DWORD(state.dwEventState);
	return Napi::External<DWORD>::New(env, newState, deleteValue<DWORD>);
}

/* Wait until reader state is changed
 * @param context
 * @param string Reader name
 * @param state Current state
 * @return state New srate
 */
Napi::Value pcscWaitUntilReaderChange(const Napi::CallbackInfo &info)
{

	Napi::Env env = info.Env();
	CHECK_ARGUMENT_COUNT(2)
	CHECK_ARGUMENT_TYPE(0, External)
	CHECK_ARGUMENT_TYPE(1, External)
	SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
	std::string readerName = info[1].As<Napi::String>().Utf8Value();
	DWORD *curState = info[2].As<Napi::External<DWORD>>().Data();

	LONG error;
	SCARD_READERSTATE state;
	state.szReader = readerName.c_str();
	state.dwCurrentState = *curState;

	CATCH(SCardGetStatusChange(*context, INFINITE, &state, 1));
	DWORD *newState = new DWORD(state.dwEventState);
	return Napi::External<DWORD>::New(env, newState, deleteValue<DWORD>);
}

/*LONG pcscWaitUntilReaderConnected(LPSTR *buffer, DWORD *bufSize)
{

	LONG error = this->GetReaders(buffer, bufSize);
	if (error == SCARD_E_NO_READERS_AVAILABLE)
	{
		DWORD globalState;
		do
		{
			error = this->WaitUntilGlobalChange(&globalState);
		} while (!error && globalState & SCARD_STATE_UNAVAILABLE);
	}
	if (error)
	{
		return error;
	}
	return this->GetReaders(buffer, bufSize);
}
LONG pcscWaitUntilReaderState(LPSTR buffer, DWORD desiredState)
{
	LONG error;
	DWORD readerState = SCARD_STATE_UNAWARE;
	do
	{
		error = this->WaitUntilReaderChange(readerState, buffer, &readerState);
	} while (!error && !(readerState & desiredState));
	return error;
}*/

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
	Napi::HandleScope scope(env);

	exports.Set("estabilish", Napi::Function::New(env, pcscEstabilish, "estabilish"));
	exports.Set("release", Napi::Function::New(env, pcscRelease, "release"));
	exports.Set("getReaders", Napi::Function::New(env, pcscGetReaders, "getReaders"));
	exports.Set("connect", Napi::Function::New(env, pcscConnect, "connect"));
	exports.Set("disconnect", Napi::Function::New(env, pcscDisconnect, "disconnect"));
	exports.Set("transmit", Napi::Function::New(env, pcscTransmit, "transmit"));
	exports.Set("getStatus", Napi::Function::New(env, pcscTransmit, "getStatus"));
	return exports;
}

NODE_API_MODULE(pcsc, Init)

/*int main()
{
	CATCH(pcscInit());

	LPSTR names;
	DWORD namesLen;
	CATCH(pcscWaitUntilReaderConnected(&names, &namesLen));
	printf("Reader connected\n");
	printf("Bufsize: %lu\n", namesLen);
	printf("Reader: %s\n", names);

	while (1)
	{
		CATCH(pcscWaitUntilReaderState(names, SCARD_STATE_PRESENT));
		printf("Card inserted\n");

		SCARDHANDLE handle;
		CATCH(pcscConnect(names, &handle));

		const BYTE send[] = {0xFF, 0xB0, 0x00, 0x0D, 0x04};
		LPBYTE received;
		DWORD recvSize;
		CATCH(pcscTransmit(handle, send, sizeof(send), &received, &recvSize));

		for (int i = 0; i < recvSize; i++)
		{
			printf("0x%x ", received[i]);
		}
		printf("\n");

		CATCH(pcscDisconnect(handle));
		free(received);

		CATCH(pcscWaitUntilReaderState(names, SCARD_STATE_EMPTY));
		printf("Card removed\n");
	}

	free(names);
	CATCH(pcscDestroy());
	return 0;
}*/
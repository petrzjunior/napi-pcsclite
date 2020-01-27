#include <napi.h>
#include <wintypes.h>
#include <winscard.h>

#include "pcscreader.h"

Napi::FunctionReference PCSCReader::constructor;

Napi::Object PCSCReader::Init(Napi::Env env, Napi::Object exports)
{
	Napi::HandleScope scope(env);

	Napi::Function func =
		DefineClass(env,
					"PCSCReader",
					{InstanceMethod("waitUntilReaderConnected", &PCSCReader::WaitUntilReaderConnected),
					 InstanceMethod("waitUntilCardInserted", &PCSCReader::WaitUntilCardInserted),
					 InstanceMethod("transmit", &PCSCReader::Transmit)});

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

	exports.Set("PCSCReader", func);
	return exports;
}

PCSCReader::PCSCReader(const Napi::CallbackInfo &info) : Napi::ObjectWrap<PCSCReader>(info)
{
	this->pcsclite = new PCSClite();
}

PCSCReader::~PCSCReader()
{
	if (this->name)
	{
		delete[] this->name;
		this->name = nullptr;
	}
	if (this->pcsclite)
	{
		delete this->pcsclite;
		this->pcsclite = nullptr;
	}
}

Napi::Value PCSCReader::WaitUntilReaderConnected(const Napi::CallbackInfo &info)
{
	DWORD bufsize;
	this->pcsclite->WaitUntilReaderConnected(&this->name, &bufsize);
}

Napi::Value PCSCReader::WaitUntilCardInserted(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	if (!this->name)
	{
		Napi::Error::New(env, "Reader not connected").ThrowAsJavaScriptException();
	}
	this->pcsclite->WaitUntilReaderState(this->name, SCARD_STATE_PRESENT);
}

Napi::Value PCSCReader::Transmit(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	if (!this->name)
	{
		Napi::Error::New(env, "Reader not connected").ThrowAsJavaScriptException();
	}
	if (info.Length() < 1)
	{
		Napi::TypeError::New(env, "No argument supplied").ThrowAsJavaScriptException();
		return env.Null();
	}
	if (!info[0].IsArrayBuffer())
	{
		Napi::TypeError::New(env, "Wrong argument type").ThrowAsJavaScriptException();
		return env.Null();
	}
	LPBYTE recvData, sendData = (LPBYTE)info[0].As<Napi::ArrayBuffer>().Data();
	DWORD recvSize, sendSize = info[0].As<Napi::ArrayBuffer>().ByteLength();

	SCARDHANDLE handle;
	this->pcsclite->Connect(name, &handle);
	this->pcsclite->Transmit(handle, sendData, sendSize, &recvData, &recvSize);
	this->pcsclite->Disconnect(handle);

	return Napi::ArrayBuffer::New(env, recvData, recvSize);
}

Napi::Object InitReader(Napi::Env env, Napi::Object exports)
{
	return PCSCReader::Init(env, exports);
}

NODE_API_MODULE(pcscreader, InitReader)

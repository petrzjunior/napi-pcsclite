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
					{InstanceMethod("waitUntilReaderConnected", &PCSCReader::WaitUntilReaderConnected)});

	constructor = Napi::Persistent(func);
	constructor.SuppressDestruct();

	exports.Set("PCSCReader", func);
	return exports;
}

PCSCReader::PCSCReader(const Napi::CallbackInfo &info) : Napi::ObjectWrap<PCSCReader>(info)
{
	this->pcsclite = new PCSClite();
}

Napi::Value PCSCReader::WaitUntilReaderConnected(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();

	LPSTR buffer;
	LONG bufsize;
	this->pcsclite->WaitUntilReaderConnected(&buffer, &bufsize);
	return Napi::String::New(env, buffer);
}

Napi::Object InitReader(Napi::Env env, Napi::Object exports)
{
	return PCSCReader::Init(env, exports);
}

NODE_API_MODULE(pcscreader, InitReader)

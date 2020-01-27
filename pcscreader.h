#pragma once

#include <napi.h>
#include <wintypes.h>
#include <winscard.h>

#include "pcsclite.h"

class PCSCReader : public Napi::ObjectWrap<PCSCReader>
{
public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports); // Object factory

	PCSCReader(const Napi::CallbackInfo &info);
	~PCSCReader();

private:
	Napi::Value WaitUntilReaderConnected(const Napi::CallbackInfo &info);
	Napi::Value WaitUntilCardInserted(const Napi::CallbackInfo &info);
	Napi::Value Transmit(const Napi::CallbackInfo &info);

private:
	static Napi::FunctionReference constructor;
	PCSClite *pcsclite = nullptr;
	LPSTR name = nullptr;
};

Napi::Value GetReaders(const Napi::CallbackInfo &info);
Napi::Value Connect(const Napi::CallbackInfo &info);
Napi::Value Disconnect(const Napi::CallbackInfo &info);
Napi::Value GetStatus(const Napi::CallbackInfo &info);
Napi::Value Transmit(const Napi::CallbackInfo &info);
Napi::Value WaitUntilReaderChange(const Napi::CallbackInfo &info);
Napi::Value WaitUntilGlobalChange(const Napi::CallbackInfo &info);
Napi::Value WaitUntilReaderConnected(const Napi::CallbackInfo &info);
Napi::Value WaitUntilReaderState(const Napi::CallbackInfo &info);

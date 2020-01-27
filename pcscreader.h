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

private:
	Napi::Value WaitUntilReaderConnected(const Napi::CallbackInfo &info);

private:
	static Napi::FunctionReference constructor;
	PCSClite *pcsclite;
};

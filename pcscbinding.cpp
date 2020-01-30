#include <napi.h>
#include <string>
#include <winscard.h>
#include <wintypes.h>

#include "pcsclite.h"
#include "pcscbinding.h"

STATE statePresent = SCARD_STATE_PRESENT;
STATE stateEmpty = SCARD_STATE_EMPTY;

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
Napi::Value estabilish(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(0)

    SCARDCONTEXT *context = new SCARDCONTEXT();
    CATCH(pcscEstabilish(context));

    return Napi::External<SCARDCONTEXT>::New(env, context, deleteValue<SCARDCONTEXT>);
}

/* Release context
 * This function needs to be called last.
 * @param context
 */
Napi::Value release(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, External)
    SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();

    CATCH(pcscRelease(*context))

    return env.Null();
}

/* Get connected readers
 * @param context
 * @return string Readers' names in a string
 */
Napi::Value getReaders(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, External)
    SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();

    DWORD bufSize = 0;
    LPSTR buffer = new char[bufSize];
    CATCH(pcscGetReaders(*context, &buffer, &bufSize));

    Napi::String readerName = Napi::String::New(env, buffer);
    delete[] buffer;
    return readerName;
}

/* Connect to card
 * @param context
 * @param string Reader name
 * @return handle Card handle
 */
Napi::Value connect(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(2)
    CHECK_ARGUMENT_TYPE(0, External)
    CHECK_ARGUMENT_TYPE(1, String)
    SCARDCONTEXT *context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
    std::string readerName = info[1].As<Napi::String>().Utf8Value();

    SCARDHANDLE *handle = new SCARDHANDLE();
    CATCH(pcscConnect(*context, readerName.c_str(), handle));

    return Napi::External<SCARDHANDLE>::New(env, handle, deleteValue<SCARDHANDLE>);
}

/* Disconnect from card
 * @param handle
 */
Napi::Value disconnect(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, External)
    SCARDHANDLE *const handle = info[0].As<Napi::External<SCARDHANDLE>>().Data();

    CATCH(pcscDisconnect(*handle));

    return env.Null();
}
/* Transmit data to card
 * @param handle
 * @param ArrayBuffer sendData
 * @return ArrayBuffer recvData
 */
Napi::Value transmit(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(2)
    CHECK_ARGUMENT_TYPE(0, External)
    CHECK_ARGUMENT_TYPE(1, ArrayBuffer)
    SCARDHANDLE *const handle = info[0].As<Napi::External<SCARDHANDLE>>().Data();
    Napi::ArrayBuffer sendData = info[1].As<Napi::ArrayBuffer>();

    DWORD recvSize = MAX_BUFFER_SIZE;
    LPBYTE recvData;
    CATCH(pcscTransmit(*handle, (BYTE *)sendData.Data(), (DWORD)sendData.ByteLength(), &recvData, &recvSize));

    return Napi::ArrayBuffer::New(env, recvData, recvSize, deleteArray<void>); // FIXME: internal static cast failing
}

/* Get card status
 * @param handle
 * @return state
 */
Napi::Value getStatus(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, External)
    SCARDHANDLE *const handle = info[0].As<Napi::External<SCARDHANDLE>>().Data();

    STATE *state = new STATE();
    CATCH(pcscGetStatus(*handle, state));

    return Napi::External<STATE>::New(env, state, deleteValue<STATE>);
}

/* Wait until global state is changed
 * @param context
 * @return state
 */
Napi::Value waitUntilGlobalChange(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, External)
    SCARDCONTEXT *const context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();

    STATE *newState = new STATE();
    CATCH(pcscWaitUntilGlobalChange(*context, newState));

    return Napi::External<STATE>::New(env, newState, deleteValue<STATE>);
}

/* Wait until reader state is changed
 * @param context
 * @param string Reader name
 * @param state Current state
 * @return state New srate
 */
Napi::Value waitUntilReaderChange(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(2)
    CHECK_ARGUMENT_TYPE(0, External)
    CHECK_ARGUMENT_TYPE(1, External)
    SCARDCONTEXT *const context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
    std::string readerName = info[1].As<Napi::String>().Utf8Value();
    STATE *const curState = info[2].As<Napi::External<STATE>>().Data();

    STATE *newState = new STATE();
    CATCH(pcscWaitUntilReaderChange(*context, *curState, readerName.c_str(), newState));

    return Napi::External<STATE>::New(env, newState, deleteValue<STATE>);
}

/* Wait until a new reader is connected
 * @param context
 * @return string Reader name
 */
Napi::Value waitUntilReaderConnected(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, External)
    SCARDCONTEXT *const context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();

    LPSTR *buffer = new LPSTR();
    DWORD bufsize;
    CATCH(pcscWaitUntilReaderConnected(*context, buffer, &bufsize));

    Napi::String readerName = Napi::String::New(env, *buffer);
    delete[](*buffer);
    return readerName;
}

/* Wait until desired reader state
 * @param context
 * @param string readerName
 * @param state Desired state
 */
Napi::Value waitUntilReaderState(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(3)
    CHECK_ARGUMENT_TYPE(0, External)
    CHECK_ARGUMENT_TYPE(1, String)
    CHECK_ARGUMENT_TYPE(2, External)
    SCARDCONTEXT *const context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
    std::string readerName = info[1].As<Napi::String>().Utf8Value();
    STATE *const state = info[2].As<Napi::External<STATE>>().Data();

    CATCH(pcscWaitUntilReaderState(*context, readerName.c_str(), *state));

    return env.Null();
}

Napi::FunctionReference pcscEmitter::constructor;

Napi::Value pcscEmitter::watch(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Function emit = info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();

    SCARDCONTEXT *context = new SCARDCONTEXT();
    CATCH(pcscEstabilish(context));
    LPSTR buffer;
    DWORD bufSize;
    CATCH(pcscWaitUntilReaderConnected(*context, &buffer, &bufSize));
    emit.Call(info.This(), {Napi::String::New(env, "reader")});
    while (true)
    {
        CATCH(pcscWaitUntilReaderState(*context, buffer, statePresent));

        Napi::Object reader = pcscReader::constructor.New({Napi::External<SCARDCONTEXT>::New(env, context), Napi::String::New(env, buffer)});
        emit.Call(info.This(), {Napi::String::New(env, "present"), reader});

        CATCH(pcscWaitUntilReaderState(*context, buffer, stateEmpty));

        emit.Call(info.This(), {Napi::String::New(env, "empty")});
    }
    delete context;
    delete[] buffer;
    return Napi::String::New(env, "OK");
}

void pcscEmitter::Initialize(Napi::Env &env, Napi::Object &target)
{
    Napi::HandleScope scope(env);
    Napi::Function ctor = DefineClass(env, "pcscEmitter", {InstanceMethod("watch", &pcscEmitter::watch)});
    constructor = Napi::Persistent(ctor);
    constructor.SuppressDestruct();
    target.Set("pcscEmitter", ctor);
}

Napi::FunctionReference pcscReader::constructor;

pcscReader::pcscReader(const Napi::CallbackInfo &info) : Napi::ObjectWrap<pcscReader>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    if (info.Length() != 2 || !info[0].IsExternal() || !info[1].IsString())
    {
        Napi::TypeError::New(env, "Bad pcscReader constructor").ThrowAsJavaScriptException();
        return;
    }
    this->context = info[0].As<Napi::External<SCARDCONTEXT>>().Data();
    this->readerName = std::string(info[1].As<Napi::String>().Utf8Value());
}

void pcscReader::Initialize(Napi::Env &env, Napi::Object &target)
{
    Napi::HandleScope scope(env);
    Napi::Function ctor = DefineClass(env, "pcscReader", {InstanceMethod("send", &pcscReader::send)});
    constructor = Napi::Persistent(ctor);
    constructor.SuppressDestruct();
    target.Set("pcscReader", ctor); // TODO: probably not needed
}

/* Send data to present card
 * @param ArrayBuffer Send data
 * @return ArrayBuffer Received data
 */
Napi::Value pcscReader::send(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    CHECK_ARGUMENT_COUNT(1)
    CHECK_ARGUMENT_TYPE(0, ArrayBuffer)
    Napi::ArrayBuffer sendData = info[0].As<Napi::ArrayBuffer>();

    SCARDHANDLE handle;
    CATCH(pcscConnect(*this->context, this->readerName.c_str(), &handle));
    LPBYTE recvData;
    DWORD recvSize;
    CATCH(pcscTransmit(handle, (LPCBYTE)sendData.Data(), (DWORD)sendData.ByteLength(), &recvData, &recvSize));
    CATCH(pcscDisconnect(handle));

    return Napi::ArrayBuffer::New(env, recvData, recvSize, deleteArray<void>); // FIXME: internal static cast failing
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    // Functions
    exports.Set("estabilish", Napi::Function::New(env, estabilish));
    exports.Set("release", Napi::Function::New(env, release));
    exports.Set("getReaders", Napi::Function::New(env, getReaders));
    exports.Set("connect", Napi::Function::New(env, connect));
    exports.Set("disconnect", Napi::Function::New(env, disconnect));
    exports.Set("transmit", Napi::Function::New(env, transmit));
    exports.Set("getStatus", Napi::Function::New(env, getStatus));
    exports.Set("waitUntilGlobalChange", Napi::Function::New(env, waitUntilGlobalChange));
    exports.Set("waitUntilReaderChange", Napi::Function::New(env, waitUntilReaderChange));
    exports.Set("waitUntilReaderConnected", Napi::Function::New(env, waitUntilReaderConnected));
    exports.Set("waitUntilReaderState", Napi::Function::New(env, waitUntilReaderState));

    // Constants
    exports.Set("statePresent", Napi::External<STATE>::New(env, &statePresent));
    exports.Set("stateEmpty", Napi::External<STATE>::New(env, &stateEmpty));

    // Objects
    pcscEmitter::Initialize(env, exports);
    pcscReader::Initialize(env, exports);

    return exports;
}

NODE_API_MODULE(pcsc, Init)
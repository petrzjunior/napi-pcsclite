#include <napi.h>
#include <string>
#include <winscard.h>
#include <wintypes.h>

#include "pcsclite.h"

static STATE statePresent = SCARD_STATE_PRESENT;
static STATE stateEmpty = SCARD_STATE_EMPTY;

#define CATCH(err)                                                                     \
    if (err)                                                                           \
    {                                                                                  \
        Napi::Error::New(env, pcsc_stringify_error(err)).ThrowAsJavaScriptException(); \
        return env.Null();                                                             \
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
    std::string readerName = info[1].As<Napi::String>();
    STATE *const state = info[2].As<Napi::External<STATE>>().Data();

    CATCH(pcscWaitUntilReaderState(*context, readerName.c_str(), *state));

    return env.Null();
}

class pcscEmitter : public Napi::ObjectWrap<pcscEmitter>
{
public:
    pcscEmitter(const Napi::CallbackInfo &info) : Napi::ObjectWrap<pcscEmitter>(info) {}
    Napi::Value watch(const Napi::CallbackInfo &info)
    {
        Napi::Env env = info.Env();
        Napi::Function emit = info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();

        SCARDCONTEXT context;
        CATCH(pcscEstabilish(&context));
        LPSTR buffer;
        DWORD bufSize;
        CATCH(pcscWaitUntilReaderConnected(context, &buffer, &bufSize));
        emit.Call(info.This(), {Napi::String::New(env, "reader")});
        while (true)
        {
            CATCH(pcscWaitUntilReaderState(context, buffer, statePresent));
            emit.Call(info.This(), {Napi::String::New(env, "present")});
            CATCH(pcscWaitUntilReaderState(context, buffer, stateEmpty));
            emit.Call(info.This(), {Napi::String::New(env, "empty")});
        }
        return Napi::String::New(env, "OK");
    }
};

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
    Napi::Function func = Napi::ObjectWrap<pcscEmitter>::DefineClass(env, "pcscEmitter", {Napi::ObjectWrap<pcscEmitter>::InstanceMethod("watch", &pcscEmitter::watch)});
    Napi::Persistent(func).SuppressDestruct();
    exports.Set("pcscEmitter", func);

    return exports;
}

NODE_API_MODULE(pcsc, Init)
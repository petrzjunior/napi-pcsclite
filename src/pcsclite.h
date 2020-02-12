#pragma once

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else

#include <winscard.h>

#endif

#ifdef __WIN32
#define MAX_READERNAME 256
#endif

#define MAX_BUFFER_SIZE 264

typedef DWORD STATE;

LONG pcscEstablish(SCARDCONTEXT *context);

LONG pcscRelease(SCARDCONTEXT context);

LONG pcscGetReaders(SCARDCONTEXT context, LPSTR *buffer, DWORD *bufferSize);

LONG pcscConnect(SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle);

LONG pcscDisconnect(SCARDHANDLE handle);

LONG pcscCancel(SCARDCONTEXT context);

LONG pcscGetStatus(SCARDCONTEXT context, LPCSTR reader, STATE *newState);

LONG pcscTransmit(SCARDHANDLE handle, LPCBYTE sendData, DWORD sendSize, LPBYTE recvData, DWORD *recvSize);

LONG pcscDirectCommand(SCARDHANDLE handle, DWORD command, LPCBYTE sendData, DWORD sendSize, LPCBYTE recvData,
                       DWORD *recvSize);

LONG pcscWaitUntilReaderChange(SCARDCONTEXT context, STATE curState, LPCSTR readerName, STATE *newState);

LONG pcscWaitUntilGlobalChange(SCARDCONTEXT context, STATE *newState);

LONG pcscWaitUntilReaderConnected(SCARDCONTEXT context, LPSTR *buffer, DWORD *bufSize);

LONG pcscWaitUntilReaderState(SCARDCONTEXT context, LPCSTR buffer, STATE desiredState);

LONG pcscIsContextValid(SCARDCONTEXT context);

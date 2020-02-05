#pragma once

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#define MAX_BUFFER_SIZE 264

typedef DWORD STATE;

LONG pcscEstablish(SCARDCONTEXT *context);
LONG pcscRelease(const SCARDCONTEXT context);
LONG pcscGetReaders(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufferSize);
LONG pcscConnect(const SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle);
LONG pcscDisconnect(const SCARDHANDLE handle);
LONG pcscCancel(const SCARDCONTEXT context);
LONG pcscGetStatus(const SCARDHANDLE handle, STATE *state);
LONG pcscTransmit(const SCARDHANDLE handle, LPCBYTE sendData, DWORD sendSize, LPBYTE *recvData, DWORD *recvSize);
LONG pcscWaitUntilReaderChange(const SCARDCONTEXT context, STATE curState, LPCSTR readerName, STATE *newState);
LONG pcscWaitUntilGlobalChange(const SCARDCONTEXT context, STATE *newState);
LONG pcscWaitUntilReaderConnected(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufSize);
LONG pcscWaitUntilReaderState(const SCARDCONTEXT context, LPCSTR buffer, STATE desiredState);

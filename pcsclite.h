#pragma once

//#include <wintypes.h>
//#include <winscard.h>

LONG pcscEstabilish(SCARDCONTEXT *context);
LONG pcscRelease(const SCARDCONTEXT context);
LONG pcscGetReaders(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufferSize);
LONG pcscConnect(const SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle);
LONG pcscDisconnect(const SCARDHANDLE handle);
LONG pcscGetStatus(const SCARDHANDLE handle, DWORD *state);
LONG pcscTransmit(const SCARDHANDLE handle, LPCBYTE sendData, DWORD sendSize, LPBYTE *recvData, DWORD *recvSize);
LONG pcscWaitUntilReaderChange(const SCARDCONTEXT context, DWORD curState, LPCSTR readerName, DWORD *newState);
LONG pcscWaitUntilGlobalChange(const SCARDCONTEXT context, DWORD *newState);
LONG pcscWaitUntilReaderConnected(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufSize);
LONG pcscWaitUntilReaderState(const SCARDCONTEXT context, LPCSTR buffer, DWORD desiredState);
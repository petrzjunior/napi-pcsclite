#pragma once

#include <wintypes.h>
#include <winscard.h>

class PCSClite
{
public:
    PCSClite();
    ~PCSClite();

    LONG GetReaders(LPSTR *buffer, DWORD *bufferSize);
    LONG Connect(LPCSTR reader, SCARDHANDLE *handle);
    LONG Disconnect(SCARDHANDLE handle);
    LONG GetStatus(SCARDHANDLE handle, DWORD *state);
    LONG Transmit(SCARDHANDLE handle, LPCBYTE sendData, DWORD sendSize, LPBYTE *recvData, DWORD *recvSize);
    LONG WaitUntilReaderChange(DWORD curState, LPCSTR readerName, DWORD *newState);
    LONG WaitUntilGlobalChange(DWORD *newState);
    LONG WaitUntilReaderConnected(LPSTR *buffer, DWORD *bufSize);
    LONG WaitUntilReaderState(LPSTR buffer, DWORD desiredState);

private:
    SCARDCONTEXT context = 0;
}
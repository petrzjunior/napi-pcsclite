#include <winscard.h>
#include <wintypes.h>

#include "pcsclite.h"

#define READER_NOTIFICATION "\\\\?PnP?\\Notification"

LONG pcscEstabilish(SCARDCONTEXT *context)
{
	return SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, context);
}

LONG pcscRelease(const SCARDCONTEXT context)
{
	if (context != 0)
	{
		return SCardReleaseContext(context);
	}
	return SCARD_E_INVALID_PARAMETER;
}

LONG pcscGetReaders(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufferSize)
{
	LONG error;
	LPSTR buf;
	DWORD bufSize = 0;
	error = SCardListReaders(context, nullptr, nullptr, &bufSize);
	if (error)
	{
		return error;
	}
	buf = new char[bufSize];
	if (buf == nullptr)
	{
		return SCARD_E_NO_MEMORY;
	}
	error = SCardListReaders(context, nullptr, buf, &bufSize);
	*buffer = buf;
	*bufferSize = bufSize;
	return error;
}

LONG pcscConnect(const SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle)
{
	DWORD activeProtocol;
	return SCardConnect(context, reader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, handle, &activeProtocol);
}

LONG pcscDisconnect(const SCARDHANDLE handle)
{
	return SCardDisconnect(handle, SCARD_UNPOWER_CARD);
}

LONG pcscGetStatus(const SCARDHANDLE handle, DWORD *state)
{
	return SCardStatus(handle, nullptr, nullptr, state, nullptr, nullptr, nullptr);
}

LONG pcscTransmit(const SCARDHANDLE handle, LPCBYTE sendData, DWORD sendSize, LPBYTE *recvData, DWORD *recvSize)
{
	*recvSize = MAX_BUFFER_SIZE;
	*recvData = new BYTE[*recvSize];
	if (*recvData == nullptr)
	{
		return SCARD_E_NO_MEMORY;
	}
	return SCardTransmit(handle, SCARD_PCI_T0, sendData, sendSize, nullptr, *recvData, recvSize);
}

LONG pcscWaitUntilReaderChange(const SCARDCONTEXT context, DWORD curState, LPCSTR readerName, DWORD *newState)
{
	LONG error;
	SCARD_READERSTATE state;
	state.szReader = readerName;
	state.dwCurrentState = curState;

	error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*newState = state.dwEventState;
	return error;
}

LONG pcscWaitUntilGlobalChange(const SCARDCONTEXT context, DWORD *newState)
{
	LONG error;
	SCARD_READERSTATE state;
	state.szReader = READER_NOTIFICATION;
	state.dwCurrentState = SCARD_STATE_UNAWARE;

	error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*newState = state.dwEventState;
	return error;
}

LONG pcscWaitUntilReaderConnected(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufSize)
{

	LONG error = pcscGetReaders(context, buffer, bufSize);
	if (error == SCARD_E_NO_READERS_AVAILABLE)
	{
		DWORD globalState;
		do
		{
			error = pcscWaitUntilGlobalChange(context, &globalState);
		} while (!error && globalState & SCARD_STATE_UNAVAILABLE);
	}
	if (error)
	{
		return error;
	}
	return pcscGetReaders(context, buffer, bufSize);
}
LONG pcscWaitUntilReaderState(const SCARDCONTEXT context, LPCSTR buffer, DWORD desiredState)
{
	LONG error;
	DWORD readerState = SCARD_STATE_UNAWARE;
	do
	{
		error = pcscWaitUntilReaderChange(context, readerState, buffer, &readerState);
	} while (!error && !(readerState & desiredState));
	return error;
}

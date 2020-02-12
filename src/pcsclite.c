#include <stdlib.h>
#include <stddef.h>
#include "pcsclite.h"

#define READER_NOTIFICATION "\\\\?PnP?\\Notification"

LONG pcscEstablish(SCARDCONTEXT *context) {
	return SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, context);
}

LONG pcscRelease(const SCARDCONTEXT context) {
	if (context != 0) {
		return SCardReleaseContext(context);
	}
	return SCARD_E_INVALID_PARAMETER;
}

LONG pcscGetReaders(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufferSize) {
	LONG error;
	DWORD bufSize = 0;
	error = SCardListReaders(context, NULL, NULL, &bufSize);
	if (error) {
		if (error == SCARD_E_NO_READERS_AVAILABLE) {
			*buffer = NULL;
			*bufferSize = 0;
			return SCARD_S_SUCCESS;
		}
		return error;
	}
	LPSTR buf = (LPSTR) malloc(sizeof(char) * bufSize);
	if (buf == NULL) {
		return SCARD_E_NO_MEMORY;
	}
	error = SCardListReaders(context, NULL, buf, &bufSize);
	if (error == SCARD_E_NO_READERS_AVAILABLE) {
		*buffer = NULL;
		*bufferSize = 0;
		return SCARD_S_SUCCESS;
	}
	*buffer = buf;
	*bufferSize = bufSize;
	return error;
}

LONG pcscConnect(const SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle) {
	DWORD activeProtocol;
	return SCardConnect(context, reader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, handle, &activeProtocol);
}

LONG pcscDisconnect(const SCARDHANDLE handle) {
	return SCardDisconnect(handle, SCARD_UNPOWER_CARD);
}

LONG pcscCancel(const SCARDCONTEXT context) {
	return SCardCancel(context);
}

LONG pcscGetStatus(const SCARDCONTEXT context, LPCSTR reader, STATE *newState) {
	SCARD_READERSTATE state;
	state.szReader = reader;
	state.dwCurrentState = SCARD_STATE_UNAWARE;

	LONG error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*newState = state.dwEventState;
	return error;
}

LONG pcscTransmit(const SCARDHANDLE handle, LPCBYTE sendData, DWORD sendSize, LPBYTE recvData, DWORD *recvSize) {
	LONG err = SCardTransmit(handle, SCARD_PCI_T0, sendData, sendSize, NULL, recvData, recvSize);
	if (err == SCARD_W_RESET_CARD) {
		// Card was reset, update state
		DWORD activeProtocol;
		err = SCardReconnect(handle, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, SCARD_RESET_CARD, &activeProtocol);
	}
	return err;
}

LONG pcscDirectCommand(const SCARDHANDLE handle, DWORD command, LPCBYTE sendData, DWORD sendSize, LPCBYTE recvData,
                       DWORD *recvSize) {
	return SCardControl(handle, command, (void *) sendData, sendSize, (void *) recvData, *recvSize, recvSize);
}

LONG pcscWaitUntilReaderChange(const SCARDCONTEXT context, STATE curState, LPCSTR readerName, STATE *newState) {
	LONG error;
	SCARD_READERSTATE state;
	state.szReader = readerName;
	state.dwCurrentState = curState;

	error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*newState = state.dwEventState;
	return error;
}

LONG pcscWaitUntilGlobalChange(const SCARDCONTEXT context, STATE *newState) {
	LONG error;
	SCARD_READERSTATE state;
	state.szReader = READER_NOTIFICATION;
	state.dwCurrentState = SCARD_STATE_UNAWARE;

	error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*newState = state.dwEventState;
	return error;
}

LONG pcscWaitUntilReaderConnected(const SCARDCONTEXT context, LPSTR *buffer, DWORD *bufSize) {

	LONG error = pcscGetReaders(context, buffer, bufSize);
	if (error == SCARD_E_NO_READERS_AVAILABLE) {
		STATE globalState;
		do {
			error = pcscWaitUntilGlobalChange(context, &globalState);
		} while (!error && globalState & SCARD_STATE_UNAVAILABLE);
		if (!error) {
			return pcscGetReaders(context, buffer, bufSize);
		}
	}
	return error;
}

LONG pcscWaitUntilReaderState(const SCARDCONTEXT context, LPCSTR buffer, STATE desiredState) {
	LONG error;
	STATE readerState = SCARD_STATE_UNAWARE;
	do {
		error = pcscWaitUntilReaderChange(context, readerState, buffer, &readerState);
	} while (!error && !(readerState & desiredState));
	return error;
}

LONG pcscIsContextValid(const SCARDCONTEXT context) {
	return SCardIsValidContext(context);
}

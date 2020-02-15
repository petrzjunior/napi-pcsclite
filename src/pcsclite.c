#include <stdlib.h>
#include <stddef.h>

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>

#else
#include <winscard.h>

#endif
#include "pcsclite.h"


#define READER_NOTIFICATION "\\\\?PnP?\\Notification"

LONG pcsc_establish(SCARDCONTEXT *context) {
	return SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, context);
}

LONG pcsc_release(const SCARDCONTEXT context) {
	if (context != 0) {
		return SCardReleaseContext(context);
	}
	return SCARD_E_INVALID_PARAMETER;
}

LONG pcsc_get_readers(const SCARDCONTEXT context, LPSTR *buffer, DWORD *buffer_size) {
	LONG error;
	DWORD buf_size = 0;
	error = SCardListReaders(context, NULL, NULL, &buf_size);
	if (error) {
		if (error == SCARD_E_NO_READERS_AVAILABLE) {
			*buffer = NULL;
			*buffer_size = 0;
			return SCARD_S_SUCCESS;
		}
		return error;
	}
	LPSTR buf = (LPSTR) malloc(sizeof(char) * buf_size);
	if (buf == NULL) {
		return SCARD_E_NO_MEMORY;
	}
	error = SCardListReaders(context, NULL, buf, &buf_size);
	if (error == SCARD_E_NO_READERS_AVAILABLE) {
		*buffer = NULL;
		*buffer_size = 0;
		return SCARD_S_SUCCESS;
	}
	*buffer = buf;
	*buffer_size = buf_size;
	return error;
}

LONG pcsc_connect(const SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle) {
	DWORD active_protocol;
	return SCardConnect(context, reader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, handle, &active_protocol);
}

LONG pcsc_disconnect(const SCARDHANDLE handle) {
	return SCardDisconnect(handle, SCARD_UNPOWER_CARD);
}

LONG pcsc_cancel(const SCARDCONTEXT context) {
	return SCardCancel(context);
}

LONG pcsc_get_status(const SCARDCONTEXT context, LPCSTR reader, STATE *new_state) {
	SCARD_READERSTATE state;
	state.szReader = reader;
	state.dwCurrentState = SCARD_STATE_UNAWARE;

	LONG error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*new_state = state.dwEventState;
	return error;
}

LONG pcsc_transmit(const SCARDHANDLE handle, LPCBYTE send_data, DWORD send_size, LPBYTE recv_data, DWORD *recv_size) {
	LONG err = SCardTransmit(handle, SCARD_PCI_T0, send_data, send_size, NULL, recv_data, recv_size);
	if (err == SCARD_W_RESET_CARD) {
		// Card was reset, update state
		DWORD active_protocol;
		err = SCardReconnect(handle, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, SCARD_RESET_CARD, &active_protocol);
	}
	return err;
}

LONG pcsc_direct_command(const SCARDHANDLE handle, DWORD command, LPCBYTE send_data, DWORD send_size, LPCBYTE recv_data,
                         DWORD *recv_size) {
	return SCardControl(handle, command, (void *) send_data, send_size, (void *) recv_data, *recv_size, recv_size);
}

LONG pcsc_wait_until_reader_change(const SCARDCONTEXT context, STATE cur_state, LPCSTR reader_name, STATE *new_state) {
	LONG error;
	SCARD_READERSTATE state;
	state.szReader = reader_name;
	state.dwCurrentState = cur_state;

	error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*new_state = state.dwEventState;
	return error;
}

LONG pcsc_wait_until_global_change(const SCARDCONTEXT context, STATE *new_state) {
	LONG error;
	SCARD_READERSTATE state;
	state.szReader = READER_NOTIFICATION;
	state.dwCurrentState = SCARD_STATE_UNAWARE;

	error = SCardGetStatusChange(context, INFINITE, &state, 1);
	*new_state = state.dwEventState;
	return error;
}

LONG pcsc_is_context_valid(const SCARDCONTEXT context) {
	return SCardIsValidContext(context);
}

#pragma once

#if defined(__APPLE__)
#include <PSCS/wintypes.h>

#elif defined(_WIN32)
#include <winscard.h>

#else
#include <pcsclite.h>

#endif

typedef DWORD STATE;

LONG pcsc_establish(SCARDCONTEXT *context);

LONG pcsc_release(SCARDCONTEXT context);

LONG pcsc_get_readers(SCARDCONTEXT context, LPSTR *buffer, DWORD *buffer_size);

LONG pcsc_connect(SCARDCONTEXT context, LPCSTR reader, SCARDHANDLE *handle);

LONG pcsc_disconnect(SCARDHANDLE handle);

LONG pcsc_cancel(SCARDCONTEXT context);

LONG pcsc_get_status(SCARDCONTEXT context, LPCSTR reader, STATE *new_state);

LONG pcsc_transmit(SCARDHANDLE handle, LPCBYTE send_data, DWORD send_size, LPBYTE recv_data, DWORD *recv_size);

LONG pcsc_direct_command(SCARDHANDLE handle, DWORD command, LPCBYTE send_data, DWORD send_size, LPCBYTE recv_data,
                         DWORD *recv_size);

LONG pcsc_wait_until_reader_change(SCARDCONTEXT context, STATE cur_state, LPCSTR reader_name, STATE *new_state);

LONG pcsc_wait_until_global_change(SCARDCONTEXT context, STATE *new_state);

LONG pcsc_is_context_valid(SCARDCONTEXT context);

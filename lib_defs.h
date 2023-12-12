#ifndef LIB_DEFS_H_
#define LIB_DEFS_H_

#include "packet_defs.h"

namespace ICMP {

	// ICMP create file function pointer from ICMP.dll
	typedef HANDLE(WINAPI* LP_ICMP_create_file)(VOID);

	// ICMP close handle function pointer from ICMP.dll
	typedef BOOL(WINAPI* LP_ICMP_close_handle)(HANDLE ICMPhandle);

	// ICMP send echo function pointer from ICMP.dll
	typedef DWORD(WINAPI* LP_ICMP_send_echo)(
		HANDLE ICMPhandle,
		unsigned long destination_address,
		LPVOID request_data,
		WORD request_size,
		IP_OPTION_INFORMATION* request_options,
		LPVOID reply_buffer,
		DWORD reply_size,
		DWORD timeout
	);

} // namespace ICMP

#endif
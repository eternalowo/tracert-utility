#ifndef HANDLE_ICMP_H_
#define HANDLE_ICMP_H_

#include <string>
#include <iostream>
#include <ws2tcpip.h>

#include "lib_defs.h"

namespace my_tracert {

	void tracert_response(
		unsigned long* IP,
		ICMP::LP_ICMP_send_echo FUNCP_send_echo,
		ICMP::ICMP_ECHO_REPLY* Pecho_reply,
		HANDLE ICMPhandle,
		unsigned long destination_address,
		LPVOID request_data,
		WORD request_size,
		ICMP::IP_OPTION_INFORMATION* request_options,
		LPVOID reply_buffer,
		DWORD reply_size,
		DWORD timeout
	);

	void resolve_tracert_hostname(unsigned long address, const char* dest_host_ip, char* dest_host_name);

	[[nodiscard]] int resolve_IP(const char* hostname, unsigned long& destination_ip);

} // namespace my_tracert

#endif
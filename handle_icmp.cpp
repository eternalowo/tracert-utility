#include "handle_icmp.h"

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
	) {
		// handles sent ICMP echo response
		unsigned long response = FUNCP_send_echo(ICMPhandle, destination_address, request_data,
			request_size, request_options, reply_buffer, reply_size, timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms");
			if (Pecho_reply->address.S_un.S_addr != INADDR_ANY)
				*IP = Pecho_reply->address.S_un.S_addr;
		}
		else {
			std::cout << "\t*";
		}
	}

	void resolve_tracert_hostname(unsigned long address, const char* dest_host_ip, char* dest_host_name) {
		// resolving hostname by IP
		sockaddr_in dest_ip;
		memset(&dest_ip, 0, sizeof(sockaddr_in));
		dest_ip.sin_family = AF_INET;
		dest_ip.sin_addr.S_un.S_addr = address;
		dest_ip.sin_port = 0;
		if (getnameinfo((struct sockaddr*)&dest_ip, sizeof(dest_ip), dest_host_name, ICMP::MAX_HOST_NAME, NULL, 0, 0) == 0) {
			if (strcmp(dest_host_name, dest_host_ip) == 0) {
				std::cout << "\t" << dest_host_ip << std::endl;
			}
			else {
				std::cout << "\t" << dest_host_name << " [" << dest_host_ip << "]" << std::endl;
			}
		}
		else {
			std::cout << "\t" << dest_host_ip << std::endl;
		}
	}

	[[nodiscard]] int resolve_IP(const char* hostname, unsigned long& destination_ip) {
		// resolving IP by hostname
		addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		addrinfo* result;
		if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
			std::cout << "Unable to resolve target system name " << hostname << std::endl;
			WSACleanup();
			return 1;
		}
		destination_ip = reinterpret_cast<struct sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
		freeaddrinfo(result);
		return 0;
	}

} // namespace my_tracert
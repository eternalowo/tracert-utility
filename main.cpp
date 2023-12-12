#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>
#include <winsock2.h>

#include "lib_defs.h"
#include "tools.h"
#include "handle_icmp.h"

#pragma comment (lib, "ws2_32.lib")

int main(int argc, char* argv[]) {

	// max hops count, set by '-h' flag
	int hops = ICMP::MAX_HOPS;

	// max timeout in ms, set by '-w' flag
	unsigned long timeout = ICMP::MAX_TIMEOUT;

	// bool value that shows if programm have to resolve domain name, set by '-d' flag
	bool resolve_host = true;

	// handling command prompt args ( NEEDS UPDATE - LOOKS HORRENDOUS )
	if (argc == 1) {
		std::cout << "Enter host name: ";
		char temp[256];
		std::cin.getline(temp, sizeof(temp));
		argv[1] = temp;	
	}
	else if (argc >= 3) {
		for (std::size_t i = 2; i < argc; ++i) {
			if (strcmp(argv[i], "-h") == 0 && i + 1 <= argc && tools::is_convertible_to_int(argv[i + 1])) {
				hops = std::stoi(argv[i + 1]);
			}
			else if (strcmp(argv[i], "-h") == 0 && (i + 1 > argc || !tools::is_convertible_to_int(argv[i + 1]))) {
				std::cout << std::endl << "Incorrect usage of -h flag" << std::endl;
				tools::show_usage(argv[0]);
				return 1;
			}
			if (strcmp(argv[i], "-w") == 0 && i + 1 <= argc && tools::is_convertible_to_int(argv[i + 1])) {
				timeout = std::stoi(argv[i + 1]);
			}
			else if (strcmp(argv[i], "-w") == 0 && (i + 1 > argc || !tools::is_convertible_to_int(argv[i + 1]))) {
				std::cout << std::endl << "Incorrect usage of -w flag" << std::endl;
				tools::show_usage(argv[0]);
				return 1;
			}
			if (strcmp(argv[i], "-d") == 0) {
				resolve_host = false;
			}
		}
	}
	else if (argc != 2) {
		tools::show_usage(argv[0]);
		return 1;
	}

	// starting up WSA
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		std::cout << "Failed to initialize Winsock" << std::endl;
		return 1;
	}

	// resolving IP by domain name
	unsigned long destination_ip;
	if (my_tracert::resolve_IP(argv[1], destination_ip) == 1)
		return 1;
	
	// setting up ICMP.dll lib
	HINSTANCE ICMPlib = LoadLibrary(L"ICMP.dll");
	if (!ICMPlib) {
		std::cout << "Could not load up the ICMP DLL";
		WSACleanup();
		return -1;
	}

	// getting function pointers
	ICMP::LP_ICMP_create_file FUNCP_create_file = reinterpret_cast<ICMP::LP_ICMP_create_file>(GetProcAddress(ICMPlib, "IcmpCreateFile"));
	ICMP::LP_ICMP_close_handle FUNCP_close_handle = reinterpret_cast<ICMP::LP_ICMP_close_handle>(GetProcAddress(ICMPlib, "IcmpCloseHandle"));
	ICMP::LP_ICMP_send_echo FUNCP_send_echo = reinterpret_cast<ICMP::LP_ICMP_send_echo>(GetProcAddress(ICMPlib, "IcmpSendEcho"));

	if (FUNCP_create_file == NULL || FUNCP_send_echo == NULL || FUNCP_close_handle == NULL){
		std::cout << "Could not find ICMP functions in the ICMP DLL";
		WSACleanup();
		return 1;
	}

	// getting descriptor for ICMP 
	HANDLE handleICMP = FUNCP_create_file();
	if (handleICMP == INVALID_HANDLE_VALUE) {
		std::cout << "Could not get a valid ICMP handle";
		WSACleanup();
		return 1;
	}

	std::cout << "\nTracing route to " << argv[1] << " [" << inet_ntoa(*reinterpret_cast<in_addr*>(&destination_ip)) << "]"
		<< std::endl <<"with a maximum of " << hops << " hops." << std::endl << std::endl;

	// setting up information for IP header
	ICMP::IP_OPTION_INFORMATION IPoption;
	memset(&IPoption, 0, sizeof(ICMP::IP_OPTION_INFORMATION));
	IPoption.TTL = 1;

	// setting data for ICMP
	char ICMP_send_data[ICMP::ICMP_DATA_SIZE];
	memset(ICMP_send_data, 'F', sizeof(ICMP_send_data));

	char ICMP_reply_data[sizeof(ICMP::ICMP_ECHO_REPLY) + ICMP::ICMP_DATA_SIZE];
	ICMP::ICMP_ECHO_REPLY* Pecho_reply = reinterpret_cast<ICMP::ICMP_ECHO_REPLY*>(ICMP_reply_data);

	unsigned long response;
	unsigned long IP;

	sockaddr_in dest_ip;
	char dest_host[ICMP::MAX_HOST_NAME];
	char dest_host_ip[ICMP::MAX_IP_LEN];

	// sending ICMP packets
	while (hops--) {

		IP = INADDR_ANY;

		std::cout << "  " << static_cast<int>(IPoption.TTL) << "   ";

		// sending ICMP requests
		my_tracert::tracert_response(&IP, FUNCP_send_echo, Pecho_reply, handleICMP, destination_ip,
			ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);

		my_tracert::tracert_response(&IP, FUNCP_send_echo, Pecho_reply, handleICMP, destination_ip,
			ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);

		my_tracert::tracert_response(&IP, FUNCP_send_echo, Pecho_reply, handleICMP, destination_ip,
			ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);

		strcpy_s(dest_host_ip, inet_ntoa(*reinterpret_cast<in_addr*>(&IP)));

		if (IP == INADDR_ANY) {
			std::cout << "\tRequest timed out" << std::endl;
		}
		else if (resolve_host) {
			// resolving hostname by IP
			my_tracert::resolve_tracert_hostname(IP, dest_host_ip, dest_host);
		}
		else {
			std::cout << "\t" << dest_host_ip << std::endl;
		}

		// if destination point is reached 
		if (IP == destination_ip) {
			std::cout << "\nTrace complete" << std::endl;
			break;
		}

		++IPoption.TTL;
	}

	// if destination point was not reached
	if (hops == -1)
		std::cout << "\nTrace complete" << std::endl;

	FUNCP_close_handle(handleICMP);
	WSACleanup();
	return 0;
}

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>

#include <ws2tcpip.h>
#include <winsock2.h>

#define MAX_HOPS          30
#define ICMP_DATA_SIZE    32
#define MAX_TIMEOUT       3000

struct IP_OPTION_INFORMATION {
	// ip option information structure from ipexport.h 
	unsigned char         TTL;
	unsigned char         service;
	unsigned char         flags;
	unsigned char         options_size;
	unsigned char*        options_data;
};

struct ICMP_ECHO_REPLY {
	// ICMP echo reply stucture from ipexport.h
	IN_ADDR               address;
	unsigned long         status;
	unsigned long         trip_time;
	unsigned short        data_size;
	unsigned short        reserved;
	void*                 data;
	IP_OPTION_INFORMATION options;
};

// ICMP create file function pointer from ICMP.dll
typedef HANDLE (WINAPI* LP_ICMP_create_file)(VOID);

// ICMP close handle function pointer from ICMP.dll
typedef BOOL (WINAPI* LP_ICMP_close_handle)(HANDLE ICMPhandle);

// ICMP send echo function pointer from ICMP.dll
typedef DWORD (WINAPI* LP_ICMP_send_echo)(
	HANDLE ICMPhandle,
	unsigned long destination_address,
	LPVOID request_data, 
	WORD request_size,
	IP_OPTION_INFORMATION* request_options,
	LPVOID reply_buffer, 
	DWORD reply_size, 
	DWORD timeout
);

bool is_convertible_to_int(const char* str) {
	try {
		std::stoi(str);
		return true;
	}
	catch (const std::invalid_argument&) {
		return false;
	}
	catch (const std::out_of_range&) {
		return false;
	}
}

inline void show_usage(char* path) {
	std::cout << std::endl << "Usage: " << path << " target_name [-h maximum_hops] [-w timeout]" << std::endl << std::endl;
	std::cout << "Parameters: " << std::endl;
	std::cout << "    -h maximum_hops\tMaximum number of hops to search for target." << std::endl;
	std::cout << "    -w timeout\t\tTimeout in milliseconds to wait for each reply." << std::endl;
};

#pragma comment (lib, "ws2_32.lib")

int main(int argc, char* argv[]) {

	// max hops count, set by '-h' flag
	int hops = MAX_HOPS;
	// max timeout in ms, set by '-w' flag
	unsigned long timeout = MAX_TIMEOUT;

	// handling command prompt args
	if (argc == 1) {
		std::cout << "Enter host name: ";
		char temp[256];
		std::cin.getline(temp, sizeof(temp));
		argv[1] = temp;
	}
	else if (argc >= 3) {
		for (std::size_t i = 2; i < argc; ++i) {
			if (strcmp(argv[i], "-h") == 0 && i + 1 <= argc && is_convertible_to_int(argv[i + 1])) {
				hops = std::stoi(argv[i + 1]);
			}
			else if (strcmp(argv[i], "-h") == 0 && (i + 1 > argc || !is_convertible_to_int(argv[i + 1]))) {
				std::cout << std::endl << "Incorrect usage of -h flag" << std::endl;
				show_usage(argv[0]);
				return 1;
			}
			if (strcmp(argv[i], "-w") == 0 && i + 1 <= argc && is_convertible_to_int(argv[i + 1])) {
				timeout = std::stoi(argv[i + 1]);
			}
			else if (strcmp(argv[i], "-w") == 0 && (i + 1 > argc || !is_convertible_to_int(argv[i + 1]))) {
				std::cout << std::endl << "Incorrect usage of -w flag" << std::endl;
				show_usage(argv[0]);
				return 1;
			}
		}
	}
	else if (argc != 2) {
		show_usage(argv[0]);
		return 1;
	}

	// starting up WSA
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		std::cout << "Failed to initialize Winsock" << std::endl;
		return 1;
	}

	// resolving IP by domain name
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	struct addrinfo* result;
	if (getaddrinfo(argv[1], NULL, &hints, &result) != 0) {
		std::cout << "Unable to resolve target system name " << argv[1] << std::endl;
		WSACleanup();
		return 1;
	}
	unsigned long destination_ip = reinterpret_cast<struct sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(result);
	
	// setting up ICMP.dll lib
	HINSTANCE ICMPlib = LoadLibrary(L"ICMP.dll");
	if (!ICMPlib) {
		std::cout << "Could not load up the ICMP DLL";
		WSACleanup();
		return -1;
	}

	// getting function pointers
	LP_ICMP_create_file FUNCP_create_file = reinterpret_cast<LP_ICMP_create_file>(GetProcAddress(ICMPlib, "IcmpCreateFile"));
	LP_ICMP_close_handle FUNCP_close_handle = reinterpret_cast<LP_ICMP_close_handle>(GetProcAddress(ICMPlib, "IcmpCloseHandle"));
	LP_ICMP_send_echo FUNCP_send_echo = reinterpret_cast<LP_ICMP_send_echo>(GetProcAddress(ICMPlib, "IcmpSendEcho"));

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
	IP_OPTION_INFORMATION IPoption;
	memset(&IPoption, 0, sizeof(IP_OPTION_INFORMATION));
	IPoption.TTL = 1;

	// setting data for ICMP
	char ICMP_send_data[ICMP_DATA_SIZE];
	memset(ICMP_send_data, 'F', sizeof(ICMP_send_data));

	char ICMP_reply_data[sizeof(ICMP_ECHO_REPLY) + ICMP_DATA_SIZE];

	ICMP_ECHO_REPLY* Pecho_reply = reinterpret_cast<ICMP_ECHO_REPLY*>(ICMP_reply_data);

	unsigned long response;
	unsigned long UL_IP;

	// sending ICMP packets
	while (hops--) {

		UL_IP = INADDR_ANY;

		std::cout << "  " << static_cast<int>(IPoption.TTL) << "   ";

		response = FUNCP_send_echo(handleICMP, destination_ip, ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms");
			if (Pecho_reply->address.S_un.S_addr != INADDR_ANY)
				UL_IP = Pecho_reply->address.S_un.S_addr;
		}
		else {
			std::cout << "\t*";
		}

		response = FUNCP_send_echo(handleICMP, destination_ip, ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms");
			if (Pecho_reply->address.S_un.S_addr != INADDR_ANY)
				UL_IP = Pecho_reply->address.S_un.S_addr;
		}
		else {
			std::cout << "\t*";
		}

		response = FUNCP_send_echo(handleICMP, destination_ip, ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms") << "\t";
			if (Pecho_reply->address.S_un.S_addr != INADDR_ANY)
				UL_IP = Pecho_reply->address.S_un.S_addr;
		}
		else {
			std::cout << "\t*\t";
		}

		if (UL_IP == INADDR_ANY) {
			std::cout << "Request timed out" << std::endl;
		}
		else {
			std::cout << inet_ntoa(*reinterpret_cast<in_addr*>(&UL_IP)) << std::endl;
		}

		if (UL_IP == destination_ip) {
			std::cout << "\nTrace complete" << std::endl;
			break;
		}

		++IPoption.TTL;
	}

	if (hops == -1)
		std::cout << "\nTrace complete" << std::endl;

	FUNCP_close_handle(handleICMP);
	WSACleanup();
	return 0;
}

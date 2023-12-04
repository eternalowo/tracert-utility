#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>

#include <signal.h>
#include <ws2tcpip.h>
#include <winsock2.h>

#define MAX_HOPS          30
#define ICMP_DATA_SIZE    32
#define MAX_TIMEOUT       3000

typedef struct _IP_OPTION_INFORMATION {
	unsigned char         TTL;
	unsigned char         service;
	unsigned char         flags;
	unsigned char         options_size;
	unsigned char*        options_data;
} IP_OPTION_INFORMATION;

typedef struct _ICMP_ECHO_REPLY {
	unsigned long         address;
	unsigned long         status;
	unsigned long         trip_time;
	unsigned short        data_size;
	unsigned short        reserved;
	void*                 data;
	IP_OPTION_INFORMATION options;
} ICMP_ECHO_REPLY;

typedef HANDLE (WINAPI* LP_ICMP_create_file)(VOID);

typedef BOOL   (WINAPI* LP_ICMP_close_handle)(HANDLE ICMPhandle);

typedef DWORD  (WINAPI* LP_ICMP_send_echo)(
	HANDLE ICMPhandle,
	unsigned long destination_address,
	LPVOID request_data, 
	WORD request_size,
	IP_OPTION_INFORMATION* request_options,
	LPVOID reply_buffer, 
	DWORD reply_size, 
	DWORD timeout
);

bool interrupted = false;

inline void interrupt_handler(int signal) {
	interrupted = true;
}

#pragma comment (lib, "ws2_32.lib")

int main(int argc, char* argv[]) {

	int hops = MAX_HOPS;
	unsigned long timeout = MAX_TIMEOUT;

	if (argc == 1) {
		std::cout << "Enter host name: ";
		char temp[256];
		std::cin.getline(temp, sizeof(temp));
		argv[1] = temp;
	}
	else if (argc == 4 && strcmp(argv[2], "-h") == 0) {
		hops = abs(std::stoi(argv[3]));
	}
	else if (argc == 4 && strcmp(argv[2], "-w") == 0) {
		timeout = abs(std::stoi(argv[3]));
	}
	else if (argc == 6 && strcmp(argv[2], "-w") == 0 && strcmp(argv[4], "-h") == 0) {
		timeout = abs(std::stoi(argv[3]));
		hops = abs(std::stoi(argv[5]));
	}
	else if (argc == 6 && strcmp(argv[4], "-w") == 0 && strcmp(argv[2], "-h") == 0) {
		timeout = abs(std::stoi(argv[5]));
		hops = abs(std::stoi(argv[3]));
	}
	else {
		std::cout << std::endl << "Usage: " << argv[0] << " target_name [-h maximum_hops] [-w timeout]" << std::endl << std::endl;
		std::cout << "Parameters: " << std::endl;
		std::cout << "    -h maximum_hops\tMaximum number of hops to search for target." << std::endl;
		std::cout << "    -w timeout\t\tTimeout in milliseconds to wait for each reply." << std::endl;
		return 1;
	}

	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		std::cout << "Failed to initialize Winsock" << std::endl;
		return 1;
	}

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

	HINSTANCE ICMPlib = LoadLibrary(L"ICMP.dll");
	if (!ICMPlib) {
		std::cout << "Could not load up the ICMP DLL";
		WSACleanup();
		return -1;
	}

	LP_ICMP_create_file FUNCP_create_file = reinterpret_cast<LP_ICMP_create_file>(GetProcAddress(ICMPlib, "IcmpCreateFile"));
	LP_ICMP_close_handle FUNCP_close_handle = reinterpret_cast<LP_ICMP_close_handle>(GetProcAddress(ICMPlib, "IcmpCloseHandle"));
	LP_ICMP_send_echo FUNCP_send_echo = reinterpret_cast<LP_ICMP_send_echo>(GetProcAddress(ICMPlib, "IcmpSendEcho"));

	if (FUNCP_create_file == NULL || FUNCP_send_echo == NULL || FUNCP_close_handle == NULL){
		std::cout << "Could not find ICMP functions in the ICMP DLL";
		WSACleanup();
		return 1;
	}

	HANDLE handleICMP = FUNCP_create_file();
	if (handleICMP == INVALID_HANDLE_VALUE) {
		std::cout << "Could not get a valid ICMP handle";
		WSACleanup();
		return 1;
	}

	std::cout << "\nTracing route to " << argv[1] << " [" << inet_ntoa(*reinterpret_cast<in_addr*>(&destination_ip)) << "]"
		<< std::endl <<"with a maximum of " << hops << " hops." << std::endl << std::endl;

	IP_OPTION_INFORMATION IPoption;
	memset(&IPoption, 0, sizeof(IP_OPTION_INFORMATION));
	IPoption.TTL = 1;

	char ICMP_send_data[ICMP_DATA_SIZE];
	memset(ICMP_send_data, 'E', sizeof(ICMP_send_data));

	char ICMP_reply_data[sizeof(ICMP_ECHO_REPLY) + ICMP_DATA_SIZE];

	ICMP_ECHO_REPLY* Pecho_reply = reinterpret_cast<ICMP_ECHO_REPLY*>(ICMP_reply_data);

	bool loop = true;
	unsigned long response;
	unsigned long UL_IP;

	signal(SIGINT, interrupt_handler);

	while (hops--) {

		if (interrupted) {
			std::cout << "^C";
			loop = false;
			break;
		}

		std::cout << "  " << static_cast<int>(IPoption.TTL) << "   ";

		response = FUNCP_send_echo(handleICMP, destination_ip, ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms");
			UL_IP = Pecho_reply->address;
		}
		else {
			std::cout << "\t*";
		}

		response = FUNCP_send_echo(handleICMP, destination_ip, ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms");
			UL_IP = Pecho_reply->address;
		}
		else {
			std::cout << "\t*";
		}

		response = FUNCP_send_echo(handleICMP, destination_ip, ICMP_send_data, sizeof(ICMP_send_data), &IPoption, ICMP_reply_data, sizeof(ICMP_reply_data), timeout);
		if (response != 0) {
			std::cout << "\t" << (Pecho_reply->trip_time == 0 ? "<1 ms" : std::to_string(Pecho_reply->trip_time) + " ms") << "\t";
			UL_IP = Pecho_reply->address;
			std::cout << inet_ntoa(*reinterpret_cast<in_addr*>(&UL_IP)) << std::endl;
		}
		else {
			std::cout << "\t*" << "\tRequest timed out" << std::endl;
		}

		if (UL_IP == destination_ip) {
			std::cout << "\nTrace complete" << std::endl;
			loop = false;
			break;
		}

		++IPoption.TTL;
	}

	if (loop)
		std::cout << "\nTrace complete" << std::endl;

	FUNCP_close_handle(handleICMP);
	WSACleanup();
	return 0;
}
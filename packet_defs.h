#ifndef PACKET_DEFS_H_
#define PACKET_DEFS_H

#include <winsock2.h>

namespace ICMP {

	// max hostname len
	constexpr auto MAX_HOST_NAME = 1025;
	// max trace route hops ( windows default - 30 hops) 
	constexpr auto MAX_HOPS = 30;
	// max IP len ( xxx.xxx.xxx.xxx )
	constexpr auto MAX_IP_LEN = 16;
	// ICMP packet data field
	constexpr auto ICMP_DATA_SIZE = 32;
	// max trace route timeout ( windows default - 3000 ms)
	constexpr auto MAX_TIMEOUT = 3000;

	struct IP_OPTION_INFORMATION {
		// ip option information structure from ipexport.h 
		unsigned char         TTL;
		unsigned char         service;
		unsigned char         flags;
		unsigned char         options_size;
		unsigned char* options_data;
	};

	struct ICMP_ECHO_REPLY {
		// ICMP echo reply stucture from ipexport.h
		IN_ADDR               address;
		unsigned long         status;
		unsigned long         trip_time;
		unsigned short        data_size;
		unsigned short        reserved;
		void* data;
		IP_OPTION_INFORMATION options;
	};

} // namespace ICMP

#endif 
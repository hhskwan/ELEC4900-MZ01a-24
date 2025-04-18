#ifndef PACKETPARSERINTERFACE_H
#define PACKETPARSERINTERFACE_H

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
    #include <winsock2.h>
    #include <ws2tcpip.h>
#elif defined(__linux__) || defined(__APPLE__)
    #define EXPORT __attribute__((visibility("default")))
#else
    #define EXPORT
#endif

#include <pcap.h>
#include <string>
#include <ctime>

// an abstraction storing the packet information analyzed
struct PacketInfo {
    std::string protocolName;
    std::string sourceAddress;
    std::string destinationAddress;
    std::string extraInfo;
    time_t timestamp;
    suseconds_t microsecond;
    tm tm_info;
    int packetSize = 0;
};

// Abstract Base Class for Parsing the protocols
// Network Layer protocol specific, for determination of source and destination
class ProtocolParser {
public:
    virtual ~ProtocolParser() {}
    virtual bool canParse(const u_char* packet, int packetLength, pcap_t* file) = 0;
    virtual bool parse(const u_char* packet, int packetLength, PacketInfo& info, pcap_t* file, pcap_pkthdr* header) = 0;
};

// factory function
extern "C"  {
    EXPORT ProtocolParser* createParser();
    EXPORT void destroyParser(ProtocolParser* parser);
}

#endif // PACKETPARSERINTERFACE_H

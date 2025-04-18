#include "IPv4Parser.h"
#include "iostream"

#ifdef _WIN32
    #pragma comment(lib, "Ws2_32.lib")
#endif

const int IPV4_HEADER_MIN_SIZE = 20;
const int IPV4_HEADER_MAX_SIZE = 60;
const int ETHERNET_HEADER_SIZE = 14;

IPv4Parser::IPv4Parser() {};
IPv4Parser::~IPv4Parser() {};


bool IPv4Parser::canParse(const u_char* packet, int packetLength, pcap_t* file) {
    DataLinkLayerProtocols layer2 = getLayer2Protocol(file);
    if (layer2 == DataLinkLayerProtocols::error || packet == nullptr || packetLength < IPV4_HEADER_MIN_SIZE) {
        return false;
    }

    if (layer2 == Others) {
        std::cout << "Unknown data link layer protocol" << std::endl;
        return false;
    }

    if (layer2 == Ethernet) {
        // for ethernet, 12th and 13th bytes are the type of layer 3 protocol
        // so get the 12th byte and shift to left by 8 bits, and perform bit-wise OR with 13th byte
        // e.g. 12th byte = 0x08 and 13th byte = 0x00,
        // packet[12] << 8 == 0x0800
        // then packet[12] << 8 | packet[13] = 0x0800 or 0x00 = 0x0800 or 0x0000 = 0x0800.
        uint16_t version = (packet[12] << 8 | packet[13]);
        if (version == 0x0800) {
            std::cout << "Is IPv4" << std::endl;
            return true;
        }
        else {
            return false;
        }
    }

    // More cases can be added to handle more protocols, for example wifi.

    return false;
}

bool IPv4Parser::parse(const u_char* packet, int packetLength, PacketInfo& info, pcap_t* file, pcap_pkthdr* header) {
    DataLinkLayerProtocols layer2 = getLayer2Protocol(file);
    if (layer2 == DataLinkLayerProtocols::error || packet == nullptr || packetLength < IPV4_HEADER_MIN_SIZE) {
        std::cout << "pcap file link type error or packet error" << std::endl;
        return false;
    }

    if (layer2 == Others) {
        std::cout << "Unknown data link layer protocol" << std::endl;
        return false;
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    bool success = false;

    switch (layer2) {
        case Ethernet: {
            // skip the ethernet header
            const u_char* IPhdr =  packet + ETHERNET_HEADER_SIZE;
            int ipPacketLength = packetLength - ETHERNET_HEADER_SIZE;
            if (ipPacketLength < IPV4_HEADER_MIN_SIZE) {
                success = false;
            }
            else {
                // get the source address and destination address
                char srcAddr[INET_ADDRSTRLEN] = {0};
                char destAddr[INET_ADDRSTRLEN] = {0};
                if(InetNtopA(AF_INET, (void*)(IPhdr + 12), srcAddr, INET_ADDRSTRLEN) == nullptr){
                    success = false;
                    WSACleanup();
                    return success;
                }
                if(InetNtopA(AF_INET, (void*)(IPhdr + 16), destAddr, INET_ADDRSTRLEN) == nullptr){
                    success = false;
                    WSACleanup();
                    return success;
                }

                info.protocolName = "IPv4";
                info.sourceAddress = std::string(srcAddr);
                info.destinationAddress = std::string(destAddr);
                info.packetSize = packetLength;
                info.timestamp = header->ts.tv_sec;
                info.microsecond = header->ts.tv_usec;
                info.tm_info = *localtime(&(info.timestamp));
                success = true;
            }
            break;
        }

        // More cases can be added to handle more protocols, for example wifi.

        default:
            std::cout << "Invalid data link layer" << std::endl;
            break;
    }

    WSACleanup();
    return success;
}

extern "C" ProtocolParser* createParser() {
    IPv4Parser* parser = new IPv4Parser();
    return parser;
}


extern "C" void destroyParser(ProtocolParser* parser) {
    delete parser;
}

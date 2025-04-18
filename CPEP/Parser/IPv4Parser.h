#ifndef IPV4PARSER_H
#define IPV4PARSER_H

#include "../PacketParserInterface.h"
#include "layer2.h"

class IPv4Parser : public ProtocolParser {
public:
    IPv4Parser();
    ~IPv4Parser() override;

    bool canParse(const u_char* packet, int packetLength, pcap_t* file) override;
    bool parse(const u_char* packet, int packetLength, PacketInfo& info, pcap_t* file, pcap_pkthdr* header) override;

};

extern "C" {
    EXPORT ProtocolParser* createParser();
    EXPORT void destroyParser(ProtocolParser* parser);
}

#endif // IPV4PARSER_H

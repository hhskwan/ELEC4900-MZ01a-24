#ifndef LAYER2_H
#define LAYER2_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <pcap.h>

// For scalability, this enumeration can add more protocol types, if needed.
enum DataLinkLayerProtocols{
    error = -1,
    Ethernet = 0,
    WIFI = 1,
    PPP = 2,
    Others = 3
};

// Helper function: return the protocol type of data link layer
extern "C" DataLinkLayerProtocols getLayer2Protocol(pcap_t* file);

#endif // LAYER2_H

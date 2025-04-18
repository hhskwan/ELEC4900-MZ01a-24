#include "layer2.h"

DataLinkLayerProtocols getLayer2Protocol(pcap_t* file){
    if (file == nullptr) {
        return error;
    }

    int type = pcap_datalink(file);
    DataLinkLayerProtocols protocol;
    switch (type) {
    case DLT_EN10MB: {
        protocol = Ethernet;
        break;
    }

    case DLT_IEEE802_11: {
        protocol = WIFI;
        break;
    }

    case DLT_PPP: {
        protocol = PPP;
        break;
    }

    default: {
        protocol = Others;
        break;
    }
    }

    return protocol;
}

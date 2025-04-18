#ifndef NETWORKUSER_H
#define NETWORKUSER_H

#include "PacketParserInterface.h"
#include "iostream"
#include "map"
#include "vector"
using namespace std;

class NetworkUser
{
public:
    NetworkUser();
    void setPktListSrc(const map<string, vector<PacketInfo>>& map);
    void setIpPairNumPkt(const map<pair<string, string>, int>& map);
    void setIpPairPktList(const map<pair<string, string>, vector<PacketInfo>>& map);

    map<string, vector<PacketInfo>> getPktListSrc() const;
    map<pair<string, string>, int> getIpPairNumPkt() const;
    map<pair<string, string>, vector<PacketInfo>> getIpPairPktList() const;

private:
    map<string, vector<PacketInfo>> pktListSrc;
    map<pair<string, string>, int> ip_pair_num_pkt;
    map<pair<string, string>, vector<PacketInfo>> ip_pair_pkt_list;
};

#endif // NETWORKUSER_H

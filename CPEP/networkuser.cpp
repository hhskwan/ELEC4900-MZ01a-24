#include "networkuser.h"

NetworkUser::NetworkUser(){}

void NetworkUser::setPktListSrc(const std::map<std::string, std::vector<PacketInfo>>& list){
    this->pktListSrc = list;
}

void NetworkUser::setIpPairNumPkt(const map<pair<string, string>, int>& map) {
    this->ip_pair_num_pkt = map;
}

void NetworkUser::setIpPairPktList(const map<pair<string, string>, vector<PacketInfo>>& map) {
    this->ip_pair_pkt_list = map;
}

std::map<std::string, std::vector<PacketInfo>> NetworkUser::getPktListSrc() const{
    return this->pktListSrc;
}

map<pair<string, string>, int> NetworkUser::getIpPairNumPkt() const {
    return this->ip_pair_num_pkt;
}

map<pair<string, string>, vector<PacketInfo>> NetworkUser::getIpPairPktList() const {
    return this->ip_pair_pkt_list;
}



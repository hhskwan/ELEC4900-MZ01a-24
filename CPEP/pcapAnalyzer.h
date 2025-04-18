#ifndef PCAPANALYZER_H
#define PCAPANALYZER_H

#include "PacketParserInterface.h"
#include "networkuser.h"

#include <QProcess>
#include <QDir>
#include <pcap.h>

QString getPcapFilePath(const QString& role);

void getNumPkts(pcap_t* file, const pcap_pkthdr* header, const u_char* data, int& count);

void packetCounter(u_char *user, const struct pcap_pkthdr *header, const u_char *data);

void getPacketList(const QString& role, int& numPackets, std::map<std::string, std::vector<PacketInfo>>& addressToPacketListMap,
                   ProtocolParser* parser);

std::map<std::pair<std::string, std::string>, int> findEffectivePktNum(const std::vector<NetworkUser>& users);

std::map<std::pair<std::string, std::string>, double> findBidirectionalSumOfLatency(const std::vector<NetworkUser>& users);

std::map<std::pair<std::string, std::string>, double> findThroughput(const std::vector<NetworkUser>& users);

std::map<std::pair<std::string, std::string>, double[2]>findMaxMinTimeMap(const std::vector<NetworkUser>& users);

std::map<std::pair<std::string, std::string>, int[2]> findLossRate(const std::vector<NetworkUser>& users,
                                                                   const std::map<std::pair<std::string, std::string>, int>& num_eff_pkt_map);

// Function for debugging script process
void writeToFile(const QString &output, QString role);

void waitForKeywordSync(QProcess &process, const QString &keyword);

void classifyInSize(std::map<std::pair<std::string, std::string>, int>& result,
                    const std::vector<std::map<std::string, std::vector<PacketInfo>>>& users);

void classifyInList(std::map<std::pair<std::string, std::string>, std::vector<PacketInfo>>& result,
                    const std::vector<std::map<std::string, std::vector<PacketInfo>>>& users);

void classifyInNonRepeatingList(const QString& role,
                                std::map<std::pair<std::string, std::string>, std::vector<PacketInfo>>& map1,
                                std::map<std::pair<std::string, std::string>, int>& map2,
                                ProtocolParser* parser);
#endif // PCAPANALYZER_H

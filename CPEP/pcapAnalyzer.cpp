#include <pcap.h>
#include <iostream>
#include <QDir>
#include <QMessageBox>
//#include <filesystem>

#include "networkuser.h"
#include "pcapAnalyzer.h"
#include "PacketParserInterface.h"

QString getPcapFilePath(const QString& role){
    /* Assumption:
     * ELEC3120 course project is a folder named as "foggytcpTest"
     * and is assumed to be in put inside the directory "CPEP"
     * For example,
     * In my (Steven) local machine,
     * The complete directory of CPEP is "C:\Users\User\Documents\ELEC4900_CPEP\CPEP",
     * then the complete directory of ELEC3120 course project is "C:\Users\User\Documents\ELEC4900_CPEP\CPEP\foggytcpTest",
     * which the .pcap file is in foggytcpTest, i.e.  "C:\Users\User\Documents\ELEC4900_CPEP\CPEP\foggytcpTest\*.pcap"
    */

    // get the current directory. In Steven's local machine, the working directory of the executable file (i.e. the app)
    // is "C:\Users\User\Documents\ELEC4900_CPEP\CPEP\build\Desktop_Qt_6_8_2_MinGW_64_bit-Debug".
    // So QDir::current() return the above directory.
    //qDebug() << role.compare("client") << role.compare("server");
    if (role.compare("client") && role.compare("server") && role.compare("merged")){
        qDebug() << "Invalid role, must be client or server, or merged for throughput calculation";
        return QString();
    }
    QDir current_directory = QDir::currentPath();
    //qDebug() << "Current Dir = " << currentDir.absolutePath();

    // Objectives: go to "(Prefix path)\CPEP\foggytcpTest".
    //
    while (!current_directory.absolutePath().endsWith("CPEP")){
        current_directory.cdUp();
    }

    current_directory.cd("foggytcpTest");
    QString path = current_directory.absolutePath() + "/" + role + ".pcap";
    return path;
};

void getNumPkts(pcap_t* file, const pcap_pkthdr* header, const u_char* data, int& count){
    count++;
}

void packetCounter(u_char *user, const struct pcap_pkthdr *header, const u_char *data) {
    int &count = *reinterpret_cast<int *>(user);
    getNumPkts(nullptr, header, data, count);
}

void getPacketList(const QString& role, int& numPackets, std::map<std::string, std::vector<PacketInfo>>& addressToPacketListMap, ProtocolParser* parser){
    QString path = getPcapFilePath(role);
    //QMessageBox::critical(nullptr, "Unexpected Error", "unable to get path:"+path);
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcapFile = pcap_open_offline(path.toLocal8Bit().constData(), errbuf);
    //QMessageBox::critical(nullptr, "Unexpected Error", errbuf);
    if (pcapFile == nullptr) {
        QMessageBox::critical(nullptr, "Unexpected Error", "unable to read:"+role+".pcap");
        return;
    }

    int ret;
    struct pcap_pkthdr* hdr;
    const u_char* pkt;
    // Method 1: use pcap_next_ex and while loop
    while((ret = pcap_next_ex(pcapFile, &hdr, &pkt)) == 1){
        ++numPackets;
        PacketInfo packetInfo;
        if (parser->parse(pkt, hdr->len, packetInfo, pcapFile, hdr)) {
            addressToPacketListMap[packetInfo.sourceAddress].push_back(packetInfo);
        }
    }

    // Method 2: pcap_loop
    //pcap_loop(file, 0, packetCounter, reinterpret_cast<u_char*>(&num_pkts));
    /*
    qDebug() << "Number of packet in" << role << "pcap file: " << numPackets;
    for (const auto& pair : addressToPacketListMap) {
        qDebug() << "address: " << pair.first << " Have " << pair.second.size() << " pkts";
    }
    */
    pcap_close(pcapFile); // this line is VERY IMPORTANT, MUST BE PRESENT
    return;
}

void classifyInNonRepeatingList(const QString& role,
                                std::map<std::pair<std::string, std::string>, std::vector<PacketInfo>>& map1,
                                std::map<std::pair<std::string, std::string>, int>& map2,
                                ProtocolParser* parser)
{
    QString path = getPcapFilePath(role);
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcapFile = pcap_open_offline(path.toLocal8Bit().constData(), errbuf);
    if (pcapFile == nullptr) {
        QMessageBox::critical(nullptr, "Unexpected Error", "unable to read:"+role+".pcap");
        return;
    }

    int ret;
    struct pcap_pkthdr* hdr;
    const u_char* pkt;
    // Method 1: use pcap_next_ex and while loop
    while((ret = pcap_next_ex(pcapFile, &hdr, &pkt)) == 1){
        PacketInfo packetInfo;
        if (parser->parse(pkt, hdr->len, packetInfo, pcapFile, hdr)) {
            std::string src = packetInfo.sourceAddress;
            std::string dest = packetInfo.destinationAddress;
            std::pair new_pair = std::make_pair(src, dest);
            map1[new_pair].push_back(packetInfo);
            map2[new_pair]++;
        }
    }

    pcap_close(pcapFile); // this line is VERY IMPORTANT, MUST BE PRESENT
    return;
}

// helper: classify in ip pair, and store the pkts in the list
// note: will repeat
void classifyInList(std::map<std::pair<std::string, std::string>, std::vector<PacketInfo>>& result,
                    const std::vector<std::map<std::string, std::vector<PacketInfo>>>& users)
{
    for (const std::map<std::string, std::vector<PacketInfo>>& user: users) {
        for (const auto& pair : user) {
            for (const PacketInfo& pkt : pair.second) {
                std::pair<std::string, std::string> ip_pair(pkt.sourceAddress, pkt.destinationAddress);
                result[ip_pair].push_back(pkt);
            }
        }
    }
}

// helper: classify in ip pair, and store the number of pkts this pair has
// note: will repeat
void classifyInSize(std::map<std::pair<std::string, std::string>, int>& result,
                    const std::vector<std::map<std::string, std::vector<PacketInfo>>>& users)
{
    for (const std::map<std::string, std::vector<PacketInfo>>& user: users) {
        for (const auto& pair : user) {
            for (const PacketInfo& pkt : pair.second) {
                std::pair<std::string, std::string> ip_pair(pkt.sourceAddress, pkt.destinationAddress);
                result[ip_pair]++;
            }
        }
    }
}

// Find effective number of packet by IP pair
// i.e. (IP1 <-> IP2) -> (number of effective pkts)
std::map<std::pair<std::string, std::string>, int> findEffectivePktNum(const std::vector<NetworkUser>& users){
    std::map<std::pair<std::string, std::string>, int> results;
    for (size_t i = 0; i < users.size(); ++i) {
        for (size_t j = i + 1; j < users.size(); ++j) {
            // this two outer loop are used to loop through each users,
            // and prevent repetitive comparison
            // Detailed illustration: for User i, compare to User (i+1 = j), (i+2 = j+1), (i+3 = j+2), ......, (n)
            const NetworkUser& user1 = users[i];
            const NetworkUser& user2 = users[j];

            for (const auto& pair1 : user1.getIpPairNumPkt()) {
                // for each ((src IP -> dest IP) -> No. of Packets) in user1
                // compare to in user 2, see if the key (src IP -> dest IP) exist in user2.IpPairNumPkt
                // if yes, then compare No. of Packets, use smaller one.
                // (Because smaller means packet lost, only consider those not lost)
                const auto& ip_pair = pair1.first;
                int num_pkt1 = pair1.second;

                const auto& ipPairMap = user2.getIpPairNumPkt();
                auto it = ipPairMap.find(ip_pair);
                if (it != user2.getIpPairNumPkt().end()) {
                    int num_pkt2 = it->second;
                    int num_eff_pkt = std::min(num_pkt1, num_pkt2);
                    // because NetworkUser.IpPairNumPkt stores in single direction
                    // and we want to store the results of this function in bidirection
                    // so, flip the (key-value) of key found, make sure that IP1 <-> IP2
                    // i.e. if the pair is (IP2 -> IP1) -> No. of Packets,
                    // that flip (IP2 -> IP1) into (IP1 <- IP2), and accumulate the value with (IP1 -> IP2)
                    // and becomes (IP1 <-> IP2) -> No.of effective packet
                    std::pair<std::string, std::string> normalized_pair = (ip_pair.first < ip_pair.second) ? ip_pair : std::make_pair(ip_pair.second, ip_pair.first);
                    results[normalized_pair] += num_eff_pkt;
                }
            }
        }
    }
    return results;
}

// Find sum of latencies by direction (e.g. IP1 to IP2, IP2 to IP1), and sum up into bidirectional, for average latency calculation
// overall idea: similar to findEffectivePktNum
std::map<std::pair<std::string, std::string>, double> findBidirectionalSumOfLatency(const std::vector<NetworkUser>& users){
    std::map<std::pair<std::string, std::string>, double> results;

    // Idea: classify each pcap files' packet into (number of distinct IP addresses) type
    //       then for each IP address, calculate the time difference with a distinct IP's packet
    //       Finally, divided by number of packets in a single file (not sum of packets in all pcap files, since they are repeated)
    for (size_t i = 0; i < users.size(); ++i) {
        for (size_t j = i + 1; j < users.size(); ++j) {
            const NetworkUser& user1 = users[i];
            const NetworkUser& user2 = users[j];

            for (const auto& firstPair : user1.getIpPairPktList()) {
                for (const auto& secondPair: user2.getIpPairPktList()) {
                    const auto& ip_pair = firstPair.first;
                    if (ip_pair != secondPair.first) {
                        continue;
                    }

                    double sum_of_latency_bidirectional = 0;
                    for (int i = 0; i < firstPair.second.size() && i < secondPair.second.size(); ++i) {
                        double timeServer = (double)(firstPair.second[i].timestamp) + (double)(firstPair.second[i].microsecond) / 1000000;
                        double timeClient = (double)(secondPair.second[i].timestamp) + (double)(secondPair.second[i].microsecond) / 1000000;
                        double time_diff = timeServer - timeClient;
                        if (time_diff < 0) {
                            time_diff *= -1;
                        }
                        sum_of_latency_bidirectional += time_diff;
                    }
                    std::pair<std::string, std::string> normalized_pair = (ip_pair.first < ip_pair.second) ? ip_pair : std::make_pair(ip_pair.second, ip_pair.first);
                    results[normalized_pair] += sum_of_latency_bidirectional;
                }
            }
        }
    }
    return results;
}

// overall idea: same as findEffectivePktNum, but this function does not really find the throughput, just accumulating the number of bytes
// transmitted successfully.
std::map<std::pair<std::string, std::string>, double> findThroughput(const std::vector<NetworkUser>& users){
    std::map<std::pair<std::string, std::string>, double> results;

    for (size_t i = 0; i < users.size(); ++i) {
        for (size_t j = i + 1; j < users.size(); ++j) {
            const NetworkUser& user1 = users[i];
            const NetworkUser& user2 = users[j];

            for (const auto& firstPair : user1.getIpPairPktList()) {
                for (const auto& secondPair: user2.getIpPairPktList()) {
                    const auto& ip_pair = firstPair.first;
                    std::vector<PacketInfo> listToBeUsed;
                    if (ip_pair == secondPair.first) {
                        // Only compare to same ip pair and in same direction
                        // use the list of packets which have smaller number of packets
                        // (again due to packet lost, only count those transmitted succesfully)
                        if (firstPair.second.size() <= secondPair.second.size()) {
                            listToBeUsed = firstPair.second;
                        }

                        else {
                            listToBeUsed = secondPair.second;
                        }

                        // accumulate
                        int totalBytes = 0;
                        for (const PacketInfo& pkt : listToBeUsed) {
                            totalBytes += pkt.packetSize;
                        }
                        // again change into bidirectional
                        std::pair<std::string, std::string> normalized_pair = (ip_pair.first < ip_pair.second) ? ip_pair : std::make_pair(ip_pair.second, ip_pair.first);
                        results[normalized_pair] += double (totalBytes);
                    }
                }
            }
        }
    }
    return results;
}

// overall idea: similar to findEffectivePktNum
std::map<std::pair<std::string, std::string>, double[2]> findMaxMinTimeMap(const std::vector<NetworkUser>& users){
    std::map<std::pair<std::string, std::string>, double[2]> results;

    for (size_t i = 0; i < users.size(); ++i) {
        for (size_t j = i + 1; j < users.size(); ++j) {
            const NetworkUser& user1 = users[i];
            const NetworkUser& user2 = users[j];

            for (const auto& firstPair : user1.getIpPairPktList()) {
                for (const auto& secondPair: user2.getIpPairPktList()) {
                    const auto& ip_pair = firstPair.first;
                    std::vector<PacketInfo> listToBeUsed;
                    int num_eff_pkt;
                    if (ip_pair == secondPair.first) {
                        if (firstPair.second.size() >= secondPair.second.size()) {
                            listToBeUsed = firstPair.second;
                            num_eff_pkt = secondPair.second.size();

                        }

                        else {
                            continue;
                        }

                        double min = std::numeric_limits<double>::max();
                        double max = 0.0;
                        for (int i = 0 ; i < num_eff_pkt; ++i) {
                            PacketInfo& pkt = listToBeUsed[i];
                            double timeStamp = (double)(pkt.timestamp) + (double)(pkt.microsecond) / 1000000;
                            min = std::min(min, timeStamp);
                            max = std::max(max, timeStamp);
                        }

                        std::pair<std::string, std::string> normalized_pair = (ip_pair.first < ip_pair.second) ?
                                                                               ip_pair :
                                                                               std::make_pair(ip_pair.second, ip_pair.first);
                        if (results.find(normalized_pair) == results.end()) {
                            results[normalized_pair][0] = max;
                            results[normalized_pair][1] = min;
                        }
                        else {
                            results[normalized_pair][0] = std::max(results[normalized_pair][0], max);
                            results[normalized_pair][1] = std::max(results[normalized_pair][1], min);
                        }
                    }
                }
            }
        }
    }
    return results;
}

// overall idea: similar to findEffectivePktNum, but this function does not really find loss rate,
// just for finding total number of packets sent and the number of packets successfully reached
// integer array[0] = number of packets successfully reached and integer array[1] = number of packets sent
std::map<std::pair<std::string, std::string>, int[2]> findLossRate(const std::vector<NetworkUser>& users,
                                                                   const std::map<std::pair<std::string, std::string>, int>& num_eff_pkt_map){
    std::map<std::pair<std::string, std::string>, int[2]> results;

    for (size_t i = 0; i < users.size(); ++i) {
        for (size_t j = i + 1; j < users.size(); ++j) {
            const NetworkUser& user1 = users[i];
            const NetworkUser& user2 = users[j];

            for (const auto& firstPair : user1.getIpPairNumPkt()) {
                std::pair<std::string, std::string> ip_pair = firstPair.first;
                std::pair<std::string, std::string> normalized_pair = (ip_pair.first < ip_pair.second) ? ip_pair : std::make_pair(ip_pair.second, ip_pair.first);
                auto it1 = num_eff_pkt_map.find(ip_pair);
                auto it2 = num_eff_pkt_map.find(normalized_pair);
                if (it1 == num_eff_pkt_map.end() && it2 == num_eff_pkt_map.end()) {
                    continue;
                }
                int num_pkt_recv = 0;
                if (it1 != num_eff_pkt_map.end()) {
                    num_pkt_recv = it1->second;
                }
                else if (it2 != num_eff_pkt_map.end()) {
                    num_pkt_recv = it2->second;
                }

                if (results.find(normalized_pair) == results.end()) {
                    results[normalized_pair][0] = num_pkt_recv;
                    results[normalized_pair][1] = 0;
                }

                for (const auto& secondPair : user2.getIpPairNumPkt()) {
                    if (ip_pair != secondPair.first) {
                        continue;
                    }
                    results[normalized_pair][1] += std::max(firstPair.second, secondPair.second);
                }
            }
        }
    }
    qDebug() << results.size();
    return results;
}

// Function for debugging script process
void writeToFile(const QString &output, QString role) {
    QFile file(role+".txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << output << "\n";
        file.close();
    } else {
        qDebug() << "Failed to open file for writing!";
    }
}

// Handle parallel processing
void waitForKeywordSync(QProcess &process, const QString &keyword) {
    while (true) {
        if (process.waitForReadyRead(100)) {
            QString output = process.readAllStandardOutput();
            qDebug() << output;
            if (output.contains(keyword)) {
                break;
            }
        }
    }
}

#ifndef TESTCASES_H
#define TESTCASES_H

#include <QMap>
#include <QString>
#include <QStringList>

QMap<QString, QMap<QString, QStringList>> testCases;

void setTestCases(){
    testCases["Performance Tests"]["Test case 1"] = {"200", "Mbps", "10", "ms", "0"};
    testCases["Performance Tests"]["Test case 2"] = {"250", "Mbps", "10", "ms", "0"};
    testCases["Performance Tests"]["Test case 3"] = {"300", "Mbps", "10", "ms","0"};
    testCases["Performance Tests"]["Test case 4"] = {"350", "Mbps", "10", "ms", "0"};
    testCases["Performance Tests"]["Test case 5"] = {"500", "Mbps", "10", "ms", "0"};
    testCases["Performance Tests"]["Test case 6"] = {"500", "Mbps", "20", "ms", "0"};
    testCases["Performance Tests"]["Test case 7"] = {"500", "Mbps", "30", "ms", "0"};
    testCases["Performance Tests"]["Test case 8"] = {"500", "Mbps", "40", "ms", "0"};
    testCases["Performance Tests"]["Test case 9"] = {"500", "Mbps", "45", "ms", "0"};
    testCases["Performance Tests"]["Test case 10"] = {"500", "Mbps", "50", "ms", "0"};

    testCases["Stress Tests"]["Test case 1"] = {"100", "Mbps", "20", "ms", "10"};
    testCases["Stress Tests"]["Test case 2"] = {"50", "Mbps", "10", "ms", "5"};
    testCases["Stress Tests"]["Test case 3"] = {"100", "Mbps", "100", "ms", "10"};
    testCases["Stress Tests"]["Test case 4"] = {"500", "Mbps", "20", "ms", "10"};
    testCases["Stress Tests"]["Test case 5"] = {"500", "Mbps", "100", "ms", "2"};
    testCases["Stress Tests"]["Test case 6"] = {"100", "Mbps", "30", "ms", "5"};
    testCases["Stress Tests"]["Test case 7"] = {"250", "Mbps", "15", "ms", "10"};
    testCases["Stress Tests"]["Test case 8"] = {"500", "Mbps", "20", "ms", "0"};
    testCases["Stress Tests"]["Test case 9"] = {"50", "Mbps", "60", "ms", "15"};
    testCases["Stress Tests"]["Test case 10"] = {"75", "Mbps", "25", "ms", "15"};
    testCases["Stress Tests"]["Test case 11"] = {"100", "Mbps", "30", "ms", "0"};
    testCases["Stress Tests"]["Test case 12"] = {"800", "Mbps", "5", "ms", "2"};
    testCases["Stress Tests"]["Test case 13"] = {"500", "Mbps", "20", "ms", "5"};
    testCases["Stress Tests"]["Test case 14"] = {"100", "Mbps", "50", "ms", "10"};
    testCases["Stress Tests"]["Test case 15"] = {"100", "Mbps", "10", "ms", "10"};
    testCases["Stress Tests"]["Test case 16"] = {"500", "Mbps", "20", "ms", "0"};
    testCases["Stress Tests"]["Test case 17"] = {"300", "Mbps", "5", "ms", "2"};
    testCases["Stress Tests"]["Test case 18"] = {"500", "Mbps", "10", "ms", "0"};
    testCases["Stress Tests"]["Test case 19"] = {"200", "Mbps", "10", "ms", "2"};
    testCases["Stress Tests"]["Test case 20"] = {"100", "Mbps", "20", "ms", "1"};
}

#endif // TESTCASES_H

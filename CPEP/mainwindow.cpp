#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QJsonDocument"
#include "pcapAnalyzer.h"
#include "PacketParserInterface.h"
#include "PerformanceDialog.h"
#include "networkuser.h"
#include "testcases.h"

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QLibrary>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <ctime>
#include <pcap.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->uploadButton, &QPushButton::clicked, this, &MainWindow::uploadFolder);
    ui->modeSelectionBox->addItem("Mode 1");
    ui->modeSelectionBox->addItem("Mode 2");
    ui->categoriesBox->addItem("Performance Tests");
    ui->categoriesBox->addItem("Stress Tests");
    for (int i = 1; i <= 20; ++i) {
        ui->stressTestsBox->addItem("Test case " + QString::number(i));
        if (i <= 10) {
            ui->performanceTestsBox->addItem("Test case " + QString::number(i));
        }
    }
    setTestCases();

    ui->bandwidthSlider->setRange(0, 1000);

    ui->bandwidthValue->setText("1 Mbps");
    ui->delayValue->setText("0 ms");
    ui->lossValue->setText("0 %");

    connect(ui->bandwidthSlider, &QSlider::valueChanged, this, [=](int value){
        ui->bandwidthValue->setText(QString::number(value) + " Mbps");
    });

    connect(ui->bandwidthValue, &QLineEdit::editingFinished, this, [=]() {
        bool ok;
        QString input;
        if (ui->bandwidthValue->text().contains(" Mbps")) {
            input = ui->bandwidthValue->text();
            input.remove(" Mbps");
        }

        int value = input.toInt(&ok);
        if (ok && value >= 0 && value <= 1000) {
            ui->bandwidthSlider->setValue(value);
        }
        else {
            ui->bandwidthValue->setText(QString::number(ui->bandwidthSlider->value())+ " Mbps");
        }
    });

    ui->delaySlider->setRange(0, 1000);

    connect(ui->delaySlider, &QSlider::valueChanged, this, [=](int value){
        ui->delayValue->setText(QString::number(value) + " ms");
    });

    connect(ui->delayValue, &QLineEdit::editingFinished, this, [=]() {
        bool ok;
        QString input;
        if (ui->delayValue->text().contains(" ms")) {
            input = ui->delayValue->text();
            input.remove(" ms");
        }
        int value = input.toInt(&ok);
        if (ok && value >= 0 && value <= 1000) {
            ui->delaySlider->setValue(value);
        }
        else {
            ui->delayValue->setText(QString::number(ui->delaySlider->value()) + " ms");
        }
    });

    ui->lossSlider->setRange(0, 10000);

    connect(ui->lossSlider, &QSlider::valueChanged, this, [=](int value){
        double loss = value / 100.0;
        ui->lossValue->setText(QString::number(loss) + " %");
    });

    connect(ui->lossValue, &QLineEdit::editingFinished, this, [=]() {
        bool ok;
        QString input;
        if (ui->lossValue->text().contains(" %")) {
            input = ui->lossValue->text();
            input.remove(" %");
        }
        int value = input.toInt(&ok);
        if (ok && value >= 0 && value <= 100) {
            ui->lossSlider->setValue(value * 100);
        }
        else {
            ui->lossValue->setText(QString::number(ui->lossSlider->value() / 100.0) + " %");
        }
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_testButton_clicked()
{
    // Go the CPEP folder, because  Vagrant up requires
    // user execute in the folder where Vagrantfile located in
    QProcess process;
    QDir dir(QCoreApplication::applicationDirPath());
    while(!dir.absolutePath().endsWith("CPEP")){
        if(!dir.cdUp()){
            break;
        }
    }
    process.setWorkingDirectory(dir.path());
    // boot up client and server VMs
    QString scriptPath = dir.filePath("boot_vm.sh");
    if(!QFile::exists(scriptPath)){
        ui->testLabel->setText("boot_vm.sh does not exist in CPEP folder");
        return;
    }
    ui->testLabel->setText("launching VMs, \n app may freeze but it is expected behavior");
    QApplication::processEvents();
    process.start("bash", QStringList() << "boot_vm.sh");
    // Wait until VM booted up
    if (process.waitForFinished(900000)) {
        ui->testLabel->setText("VM booted up!");
    }
    else {
        ui->testLabel->setText("Timeout, you can try again or manually boot the vm up");
    }

    // Debugging codes
    QByteArray terminal_output = process.readAllStandardOutput();
    QString terminal_output_str = QString::fromUtf8(terminal_output);
    QStringList terminal_output_str_list = terminal_output_str.split("\n");
    for (const QString& line : terminal_output_str_list) {
        qDebug() << line;
    }
}

void handleScriptReturnedValue(QProcess& process, const QString& role, QWidget* window) {
    switch (process.exitCode()) {
    case 124:
        QMessageBox::critical(window, "Script error", role + " script time out");
        break;
    case 255:
        QMessageBox::critical(window, "Script error", "Fail to login " + role + " VM, please check your VM status");
        break;
    case 126:
        QMessageBox::critical(window, "Script error", "Permission denied, please check the accessibility on " + role + "_script_cpep.sh");
        break;
    case 127:
        QMessageBox::critical(window, "Script error", role + "_script_cpep.sh does not exist in foggytcpTest folder");
        break;
    case -1:
        QMessageBox::critical(window, "Error", "Unknown Error");
        break;
    default:
        // case : script return 0
        break;
    }
}

void MainWindow::on_loginButton_clicked()
{
    QDir current_directory = QCoreApplication::applicationDirPath();
    while (!current_directory.absolutePath().endsWith("CPEP")) {
        if(!current_directory.cdUp()){
            break;
        }
    }

    QString mode = ui->modeSelectionBox->currentText();
    QString category;
    QString testCase;
    QStringList conditions;
    if (mode == "Mode 1") {
        category = ui->categoriesBox->currentText();
        if (category == "Performance Tests") {
            testCase = ui->performanceTestsBox->currentText();
        }
        else if (category == "Stress Tests") {
            testCase = ui->stressTestsBox->currentText();
        }
        else {
            return;
        }
        if (testCases.contains(category) && testCases[category].contains(testCase)) {
            conditions = testCases[category][testCase];
        }
        else {
            qDebug() << "Selected test case not found in testCases map!";
            return;
        }
    }

    else if (mode == "Mode 2"){
        int bandwidth = ui->bandwidthSlider->value();
        int delay = ui->delaySlider->value();
        double loss = ui->lossSlider->value() / 100.0;
        conditions = {QString::number(bandwidth), "Mbps", QString::number(delay), "ms", QString::number(loss)};
    }

    QString bandwidth = conditions[0];
    QString bwUnit = conditions[1];
    QString delay = conditions[2];
    QString loss = conditions[4];
    ui->testLabel->setText("log in VM and execute script, \n app may freeze but it is expected behavior");
    QApplication::processEvents();

    QProcess serverProcess;
    // Login server vm and execute the server script
    serverProcess.setWorkingDirectory(current_directory.absolutePath());
    serverProcess.start("bash", QStringList() << "server_vm_login.sh" << bandwidth << bwUnit << delay << loss);
    qDebug() << "executing server script";
    // Synchronization mechanism
    waitForKeywordSync(serverProcess, "Server VM script executing");

    // Login client vm and execute client script
    QProcess clientProcess;
    clientProcess.setWorkingDirectory(current_directory.absolutePath());
    clientProcess.start("bash", QStringList() << "client_vm_login.sh" << bandwidth << bwUnit << delay << loss);
    qDebug() << "executing client script";
    serverProcess.waitForFinished(90000);
    clientProcess.waitForFinished(90000);
    handleScriptReturnedValue(serverProcess, "server", this);
    handleScriptReturnedValue(clientProcess, "client", this);
    writeToFile("Server Process Output:\n" + serverProcess.readAllStandardOutput(), "server");
    writeToFile("Client Process Output:\n" + clientProcess.readAllStandardOutput(), "client");
}

typedef ProtocolParser* (*CreateParserFunc)();
typedef void (*DestroyParserFunc)(ProtocolParser*);

void MainWindow::on_pcapButton_clicked()
{
    QString path;
    QDir current_directory = QDir::currentPath();
    while (!current_directory.absolutePath().endsWith("CPEP")) {
        current_directory.cdUp();
    }
    path = current_directory.absolutePath();
    qDebug() << path;
    bool valid = false;
    QString dllName = QInputDialog::getText(nullptr, "Enter dll file name",
                                            "Please enter the name of parser dll file you want to use"
                                            "(for example, IPv4Parser.dll -> Enter 'IPv4Parser' without quotation mark)"
                                            "\n"
                                            "Note: Case Sensitive",
                                            QLineEdit::Normal,
                                            "IPv4Parser", &valid);
    if (!valid || dllName.isEmpty()) {
        return;
    }
    // Name or path of dynamic loading library
    QLibrary lib(path + "/Parser/" + dllName);

    if (!lib.load()) {
        QMessageBox::critical(this, "Unexpected Error", "unable to load DLL:"+lib.errorString());
        return;
    }

    CreateParserFunc createParser = (CreateParserFunc)lib.resolve("createParser");
    DestroyParserFunc destroyParser = (DestroyParserFunc)lib.resolve("destroyParser");

    if (!createParser || !destroyParser) {
        QMessageBox::critical(this, "Unexpected error : Cannot read function pointer:", lib.errorString() + "\n Please check your parser implementation");
        return;
    }

    // role parameters to be passed
    const QString& serverName = "server";
    const QString& clientName = "client";
    // number of packets
    int num_pkts_server = 0;
    int num_pkts_client = 0;
    // A hash map of (string - > list of packet) pairs. Here means IP address to Packet (sorted by source address)
    std::map<std::string, std::vector<PacketInfo>> serverAddressPacketMap = {};
    std::map<std::pair<std::string, std::string>, std::vector<PacketInfo>> serverIpPairPktsMap = {};
    std::map<std::pair<std::string, std::string>, int> serverIpPairNumPktsMap = {};
    // Initialize the parser for server's packet
    ProtocolParser* serverParser = createParser();
    // Initialize serverAddressPacketMap utilizing pass by reference, and accumulate num_pkts_server using again pass by reference
    getPacketList(serverName, num_pkts_server, serverAddressPacketMap, serverParser);
    classifyInNonRepeatingList(serverName, serverIpPairPktsMap, serverIpPairNumPktsMap, serverParser);
    NetworkUser server;
    // (src IP -> List of packets)
    server.setPktListSrc(serverAddressPacketMap);
    // ((src IP -> dest IP) -> List of Packets)
    server.setIpPairPktList(serverIpPairPktsMap);
    // ((src IP -> dest IP) -> No. of Packets)
    server.setIpPairNumPkt(serverIpPairNumPktsMap);

    // Same as above, but client this time
    std::map<std::string, std::vector<PacketInfo>> clientAddressPacketMap = {};
    std::map<std::pair<std::string, std::string>, std::vector<PacketInfo>> clientIpPairPktsMap = {};
    std::map<std::pair<std::string, std::string>, int> clientIpPairNumPktsMap = {};
    ProtocolParser* clientParser = createParser();
    getPacketList(clientName, num_pkts_client, clientAddressPacketMap, clientParser);
    classifyInNonRepeatingList(clientName, clientIpPairPktsMap, clientIpPairNumPktsMap, serverParser);

    NetworkUser client;
    // (src IP -> List of packets)
    client.setPktListSrc(clientAddressPacketMap);
    // ((src IP -> dest IP) -> List of Packets)
    client.setIpPairPktList(clientIpPairPktsMap);
    // ((src IP -> dest IP) -> No. of Packets)
    client.setIpPairNumPkt(clientIpPairNumPktsMap);

    // List of network users
    std::vector<NetworkUser> networkUsers;
    networkUsers.push_back(server);
    networkUsers.push_back(client);

    std::map<std::pair<std::string, std::string>, int> ipEffPktNumMap = findEffectivePktNum(networkUsers);
    /*
    for (const auto& pair : ipEffPktNumMap) {
        qDebug() << pair.first.first << "<->" << pair.first.second << ":" << pair.second;
    }*/

    // to create the latency map
    std::map<std::pair<std::string, std::string>, double> ipBiDirLatMap = findBidirectionalSumOfLatency(networkUsers);
    for (const auto& pair1 : ipEffPktNumMap) {
        for (const auto& pair2 : ipBiDirLatMap) {
            if (pair1.first != pair2.first) {
                continue;
            }
            const auto& ip_pair = pair1.first;
            double average_latency = pair2.second / pair1.second;
            qDebug() << pair2.second << pair1.second;
            ipBiDirLatMap[ip_pair] = average_latency;
        }
    }
    std::map<std::pair<std::string, std::string>, double>& ip_average_latency_map = ipBiDirLatMap;
    /*
    for (const auto& pair : ip_average_latency_map) {
        qDebug() << pair.first.first << "<->" << pair.first.second << "Average latency:" << pair.second << "s";
    }*/
    std::map<std::pair<std::string, std::string>, double> throughputMap = findThroughput(networkUsers);

    std::map<std::pair<std::string, std::string>, double[2]> ipMaxMinTimestampMap = findMaxMinTimeMap(networkUsers);
    std::map<std::pair<std::string, std::string>, double> ipEffectiveTransmissionTimeMap;
    for (const auto& pair : ipMaxMinTimestampMap) {
        ipEffectiveTransmissionTimeMap[pair.first] = pair.second[0] - pair.second[1];
    }
    /*
    for (const auto& pair : ipEffectiveTransmissionTimeMap) {
        qDebug() << pair.first.first << "<->" << pair.first.second << "effective tx time:" << pair.second << "s";
    }*/

    for (const auto& pair1 : throughputMap) {
        for (const auto& pair2 : ipEffectiveTransmissionTimeMap) {
            if (pair1.first != pair2.first) {
                continue;
            }
            double throughput = pair1.second / pair2.second;
            throughputMap[pair2.first] = throughput / 1000000.0;
        }
    }
    /*
    for (const auto& pair : throughputMap) {
        qDebug() << pair.first.first << "<->" << pair.first.second << "throughput:" << pair.second;
    }*/

    std::map<std::pair<std::string, std::string>, int[2]> ipRecvPktTotalPktMap = findLossRate(networkUsers, ipEffPktNumMap);
    std::map<std::pair<std::string, std::string>, double> lossRateMap;
    for (const auto& pair : ipRecvPktTotalPktMap) {
        std::pair<std::string, std::string> ip_pair = pair.first;
        qDebug() << pair.second[0] << pair.second[1];
        lossRateMap[ip_pair] = 1.0 - (double (pair.second[0]) / pair.second[1]);
    }
    /*
    for (const auto& pair : lossRateMap) {
        qDebug() << pair.first.first << "<->" << pair.first.second << "loss rate:" << pair.second;
    }*/

    for (const auto& pair1 : throughputMap) {
        for (const auto& pair2 : ip_average_latency_map) {
            for (const auto& pair3 : ipEffectiveTransmissionTimeMap) {
                for (const auto& pair4 : lossRateMap) {
                    if (pair1.first == pair2.first && pair1.first == pair3.first && pair1.first == pair4.first) {
                        PerformanceDisplayer resultUI(pair1.first, pair1.second, pair2.second * 1000, pair3.second, pair4.second);
                        resultUI.exec();
                    }
                }
            }
        }
    }
    destroyParser(serverParser);
    destroyParser(clientParser);
    lib.unload();
}

void MainWindow::uploadFolder() {
    // Open a folder selection dialog
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder to Upload", QDir::homePath());

    if (!folderPath.isEmpty()) {
        //qDebug() << "Selected folder:" << folderPath;

        // Destination directory where uploaded files will be stored
        QDir cpepDir = QDir::currentPath();
        while (!cpepDir.absolutePath().endsWith("CPEP")) {
            if(!cpepDir.cdUp()){
                break;
            }
        }
        QString destinationDir = cpepDir.absolutePath() + "/foggytcpTest";

        // Remove the existing directory and its contents
        QDir dir(destinationDir);
        if (dir.exists()) {
            dir.removeRecursively();  // This ensures all old files are deleted
        }

        // Create a fresh directory
        QDir().mkpath(destinationDir);

        processFolder(folderPath, destinationDir);
        QMessageBox::information(this, "Success", "Folder uploaded and stored successfully!");
    } else {
        qDebug() << "No folder selected.";
    }
}

void MainWindow::processFolder(const QString& folderPath, const QString& destinationPath) {
    QDir dir(folderPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Error", "The selected folder does not exist!");
        return;
    }

    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& entry : entries) {
        if (entry.isDir()) {
            QString newDestination = destinationPath + "/" + entry.fileName();
            QDir().mkpath(newDestination); // Ensure the subdirectory exists
            processFolder(entry.absoluteFilePath(), newDestination); // Recursive copy
        } else if (entry.isFile()) {
            QString destinationFile = destinationPath + "/" + entry.fileName();
            if (QFile::exists(destinationFile)) {
                QFile::remove(destinationFile); // Remove existing file before copying
            }

            if (!QFile::copy(entry.absoluteFilePath(), destinationFile)) {
                QMessageBox::information(this, "Fail", "fail to upload files!");
            }
        }
    }
}

void MainWindow::on_modeSelectionBox_currentIndexChanged(int index){
    ui->stackedWidget->setCurrentIndex(index);
}

void MainWindow::on_categoriesBox_currentIndexChanged(int index){
    ui->categoryPage->setCurrentIndex(index);
}


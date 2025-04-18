#include "PerformanceDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

PerformanceDisplayer::PerformanceDisplayer(std::pair<std::string, std::string> ip_pair,
                                           double throughput, double latency, double eff_tx_time, double loss_rate, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Simulation Results");
    setupUI(ip_pair, throughput, latency, eff_tx_time, loss_rate);
}

void PerformanceDisplayer::setupUI(std::pair<std::string, std::string> ip_pair, double throughput, double latency, double eff_tx_time, double loss_rate) {
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(("IP Pair: " + QString::fromStdString(ip_pair.first) + "<->" + QString::fromStdString(ip_pair.second))));
    layout->addWidget(new QLabel("Throughput: " + QString::number(throughput, 'f', 2) + " MBps"));
    layout->addWidget(new QLabel("Latency: " + QString::number(latency, 'f', 2) + " ms"));
    layout->addWidget(new QLabel("Effective Transmission Time: " + QString::number(eff_tx_time, 'f', 2) + "s"));
    layout->addWidget(new QLabel("Loss rate: " + QString::number(loss_rate * 100, 'f', 2) + "%"));

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

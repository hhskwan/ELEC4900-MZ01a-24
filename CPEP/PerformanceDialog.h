#ifndef PERFORMANCEDIALOG_H
#define PERFORMANCEDIALOG_H

#include <QDialog>

class QLabel;

class PerformanceDisplayer : public QDialog {
    Q_OBJECT
public:
    PerformanceDisplayer(std::pair<std::string, std::string> ip_pair,
                         double throughput, double latency, double eff_tx_time, double loss_rate, QWidget* parent = nullptr);
private:
    void setupUI(std::pair<std::string, std::string> ip_pair,
                 double throughput, double latency, double eff_tx_time, double loss_rate);
};

#endif // PERFORMANCEDIALOG_H

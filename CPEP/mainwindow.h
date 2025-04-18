#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_testButton_clicked();

    void on_loginButton_clicked();

    void on_pcapButton_clicked();

    void uploadFolder(); // New slot for folder upload

    //void on_dllButton_clicked();

    //void on_menuButton_clicked();

    void on_modeSelectionBox_currentIndexChanged(int index);

    void on_categoriesBox_currentIndexChanged(int index);

private:
    void processFolder(const QString& folderPath, const QString& destinationPath); // Updated function to copy files
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

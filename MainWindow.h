#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QString>
#include <QSemaphore>
#include <QtSerialPort/QSerialPort>
#include <functional>

#include "user/ScriptThread.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void localErrorCallback(uint8_t node_addr, uint8_t err);
    void nodeErrorCallback(uint8_t node_addr, uint8_t err);

    void sendDataCallback(uint8_t addr, uint8_t *data, uint8_t size);

    void log(unsigned char node_addr, const char* msg);

signals:
    void sig_updateScriptLabel(short currentLabel);
    void sig_serialData(unsigned char addr, QVector<unsigned char> arr);
    void sig_localError(unsigned char node_addr, unsigned char err);
    void sig_nodeError(unsigned char node_addr, unsigned char err);
    void sig_log(unsigned char node_addr, QString msg);

private slots:
    void on_localError(unsigned char node_addr, unsigned char err);
    void on_nodeError(unsigned char node_addr, unsigned char err);
    void on_log(unsigned char node_addr, QString msg);

    void on_serialDataToDevice(unsigned char addr, QVector<unsigned char> data);
    void on_serialDataFromDevice();

    void on_scriptLabel(short currentLabel);

    void on_pushButton_PortRefresh_clicked();

    void on_pushButton_ScriptLoad_clicked();

    void on_pushButton_ScriptExec_clicked();

    void on_pushButton_ScriptStop_clicked();

    void on_pushButton_Port_clicked();

private:
    Ui::MainWindow *ui;

    static QSerialPort serialPort;

    QHash<int16_t, int16_t> scriptIndexHash;   // <label, row number>
    int16_t lastLabel;

    void updateScriptLabel(int16_t currentLabel);

    ScriptThread scriptThread;
};

#endif // MAINWINDOW_H

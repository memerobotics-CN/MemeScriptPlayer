#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QVector>
#include <QGraphicsSimpleTextItem>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QDebug>

#include "dialogwait.h"

#include "MemeServoAPI/MemeServoAPI.h"


QSerialPort MainWindow::serialPort;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->comboBox_PortBaud->addItem("4800");
    ui->comboBox_PortBaud->addItem("9600");
    ui->comboBox_PortBaud->addItem("19200");
    ui->comboBox_PortBaud->addItem("38400");
    ui->comboBox_PortBaud->addItem("57600");
    ui->comboBox_PortBaud->addItem("115200");
    ui->comboBox_PortBaud->addItem("230400");
    ui->comboBox_PortBaud->setCurrentText("115200");

    statusBar()->showMessage(QObject::tr("Copyright (C) 2016-2017 Meme Robotics Corp."));

    qRegisterMetaType<QVector<unsigned char> >("QVector<unsigned char>");

    QObject::connect(this, SIGNAL(sig_updateScriptLabel(short)), this, SLOT(on_scriptLabel(short)));
    QObject::connect(this, SIGNAL(sig_localError(unsigned char, unsigned char)), this, SLOT(on_localError(unsigned char, unsigned char)));
    QObject::connect(this, SIGNAL(sig_nodeError(unsigned char, unsigned char)), this, SLOT(on_nodeError(unsigned char, unsigned char)));
    QObject::connect(this, SIGNAL(sig_log(unsigned char, QString)), this, SLOT(on_log(unsigned char, QString)));

    lastLabel = 0;
}

MainWindow::~MainWindow()
{
    DialogWait wait(this);
    wait.show();

    scriptThread.stop();

    delete ui;
}


void MainWindow::log(unsigned char node_addr, const char *msg)
{
    emit sig_log(node_addr, QString(msg));
}


void MainWindow::on_log(unsigned char node_addr, QString msg)
{
    // Dispaly
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " - node: " << node_addr << ", msg: " << msg;
    ui->plainTextEdit_Logs->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + QObject::tr(" - node: %1, msg: %2").arg(node_addr, 2, 16, QLatin1Char('0')).arg(msg));
}


void MainWindow::localErrorCallback(uint8_t node_addr, uint8_t err)
{
    emit sig_localError(node_addr, err);
}


void MainWindow::on_localError(unsigned char node_addr, unsigned char err)
{
    // Dispaly
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " - local error: " << node_addr << ", " << err;
    ui->plainTextEdit_Logs->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + QObject::tr(" - Error when invoking API: 0x%1，node: 0x%2").arg(err, 2, 16, QLatin1Char('0')).arg(node_addr, 2, 16, QLatin1Char('0')));
}


void MainWindow::nodeErrorCallback(uint8_t node_addr, uint8_t err)
{
    emit sig_nodeError(node_addr, err);
}


void MainWindow::on_nodeError(unsigned char node_addr, unsigned char err)
{
    // Dispaly
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " - node error: " << node_addr << ", " << err;
    ui->plainTextEdit_Logs->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + QObject::tr(" - Node 0x%1 error: 0x%2").arg(node_addr, 2, 16, QLatin1Char('0')).arg(err, 2, 16, QLatin1Char('0')));
}


void MainWindow::sendDataCallback(uint8_t addr, uint8_t *data, uint8_t size)
{
    QVector<unsigned char> arr;

    for (int i=0; i<size; i++)
    {
        arr << *data;
        data++;
    }

    emit sig_serialData(addr, arr);
}


void MainWindow::on_serialDataToDevice(unsigned char addr, QVector<unsigned char> data)
{
    Q_UNUSED(addr);

    qDebug() << "Data to device: " << data;
    serialPort.write((const char*)data.constData(), data.size());
}


void MainWindow::on_serialDataFromDevice()
{
    QVector<unsigned char> arr;
    char c;

    while (serialPort.read(&c, 1) > 0)
    {
        arr << c;
        scriptThread.onSerialData(c);
    }

    qDebug() << "Data from device: " << arr;
}


void MainWindow::updateScriptLabel(short nextLabel)
{
    emit sig_updateScriptLabel(nextLabel);
}


void MainWindow::on_scriptLabel(int16_t currLabel)
{
    QAbstractItemModel *model = ui->tableView_Script->model();

    if (scriptIndexHash.find(lastLabel) != scriptIndexHash.end())
    {
        int row = scriptIndexHash[lastLabel];
        model->setData(model->index(row, 0), QBrush(QColor(0, 0, 0)), Qt::ForegroundRole);
        model->setData(model->index(row, 1), QBrush(QColor(0, 0, 0)), Qt::ForegroundRole);
    }

    if (currLabel > 0)
    {
        if (scriptIndexHash.find(currLabel) != scriptIndexHash.end())
        {
            int row = scriptIndexHash[currLabel];
            model->setData(model->index(row, 0), QBrush(QColor(255, 0, 0)), Qt::ForegroundRole);
            model->setData(model->index(row, 1), QBrush(QColor(255, 0, 0)), Qt::ForegroundRole);
        }

        lastLabel = currLabel;
    }
    else if (currLabel < 0)
    {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("Error when executing script: '%1'").arg(currLabel));
        msgBox.exec();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(QObject::tr("Script execution finished."));
        msgBox.exec();
    }
}


void MainWindow::on_pushButton_PortRefresh_clicked()
{
    ui->pushButton_PortRefresh->setEnabled(false);

    if (!serialPort.isOpen())
    {
        serialPort.setBaudRate(QSerialPort::Baud115200);

        for (int i=0; i<256; i++)
        {
            QString serialPortName = QObject::tr("COM%1").arg(i);
            serialPort.setPortName(serialPortName);

            if (serialPort.open(QIODevice::ReadOnly))
            {
                serialPort.close();

                int index = ui->comboBox_Port->findText(serialPortName);

                if (index < 0)
                    ui->comboBox_Port->addItem(serialPortName);
            }
            else
            {
                int index = ui->comboBox_Port->findText(serialPortName);

                if (index >= 0)
                    ui->comboBox_Port->removeItem(index);
            }
        }

        bool enabled = (ui->comboBox_Port->count() > 0);
        ui->comboBox_PortBaud->setEnabled(enabled);
        ui->comboBox_Port->setEnabled(enabled);
        ui->pushButton_Port->setEnabled(enabled);
    }

    ui->pushButton_PortRefresh->setEnabled(true);
}




void MainWindow::on_pushButton_Port_clicked()
{
    if (serialPort.isOpen())
    {
        serialPort.close();
        ui->pushButton_Port->setText(QObject::tr("Open"));

        ui->pushButton_PortRefresh->setEnabled(true);
        ui->comboBox_Port->setEnabled(true);
        ui->comboBox_PortBaud->setEnabled(true);

        ui->pushButton_ScriptLoad->setEnabled(false);
        ui->pushButton_ScriptExec->setEnabled(false);

       QAbstractItemModel *model = ui->tableView_Script->model();
       ui->tableView_Script->setModel(NULL);
       delete model;
    }
    else
    {
        serialPort.setPortName(ui->comboBox_Port->currentText());
        serialPort.setBaudRate(ui->comboBox_PortBaud->currentText().toInt());

        if (serialPort.open(QIODevice::ReadWrite))
        {
            ui->pushButton_Port->setText(QObject::tr("Close"));

            ui->pushButton_PortRefresh->setEnabled(false);
            ui->comboBox_Port->setEnabled(false);
            ui->comboBox_PortBaud->setEnabled(false);

            ui->pushButton_ScriptLoad->setEnabled(true);

            statusBar()->showMessage(QObject::tr("Port \"%1\" opened.").arg(ui->comboBox_Port->currentText()), 5000);
        }
        else
            statusBar()->showMessage(QObject::tr("Failed when opening port \"%1\".").arg(ui->comboBox_Port->currentText()), 5000);
    }
}


void MainWindow::on_pushButton_ScriptLoad_clicked()
{
    QMessageBox msgBox;

    QString fileName = QFileDialog::getOpenFileName(this, QObject::tr("Open script file"), "", QObject::tr("Script file (*.txt)"));

    if (fileName.length() == 0)
        return;

    QFile scriptFile(fileName);

    if (!scriptFile.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        msgBox.setText(QObject::tr("Open script file '%1' failed.").arg(fileName));
        msgBox.exec();

        return;
    }

    QStandardItemModel *model;

    model = (QStandardItemModel*)ui->tableView_Script->model();

    if (!model)
    {
        model = new QStandardItemModel();
        model->setColumnCount(2);
        model->setHeaderData(0, Qt::Horizontal, QObject::tr("Label"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("Script"));
        ui->tableView_Script->setModel(model);
    }
    else
    {
        model->clear();
    }

    QByteArray bytes;
    int i = 0;
    scriptIndexHash.clear();

    while ((bytes = scriptFile.readLine()), bytes.size() > 0)
    {
        QString line = bytes.data();
        line.remove(QRegExp("[ \\n\\t\\r]"));

        if (line.length() == 0)
            continue;

        line = bytes.data();
        line = line.trimmed();

        QStringList splitted = line.split(":");

        if (splitted.size() != 2)
        {
            msgBox.setText(QObject::tr("Script syntax error."));
            msgBox.exec();

            break;
        }

        scriptIndexHash[((QString)splitted.at(0)).toShort()] = i;
        model->setItem(i, 0, new QStandardItem(splitted.at(0).trimmed()));
        model->setData(model->index(i, 0), Qt::AlignRight, Qt::TextAlignmentRole);
        model->setItem(i, 1, new QStandardItem(splitted.at(1).trimmed()));
        model->setData(model->index(i, 1), Qt::AlignLeft, Qt::TextAlignmentRole);
        ui->tableView_Script->setRowHeight(i, 16);
        i++;
    }

    //ui->tableView_Script->resizeRowsToContents();
    ui->tableView_Script->resizeColumnsToContents();

    scriptFile.close();

    int16_t nextLabel = scriptThread.init(
                fileName,
                std::bind(&MainWindow::updateScriptLabel, this, std::placeholders::_1),
                std::bind(&MainWindow::sendDataCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                std::bind(&MainWindow::localErrorCallback, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&MainWindow::nodeErrorCallback, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&MainWindow::log, this, std::placeholders::_1, std::placeholders::_2)
    );

    if (nextLabel > 0)
    {
        ui->pushButton_ScriptExec->setEnabled(true);
    }
    else
    {
        msgBox.setText(QObject::tr("Failed when loading script: %d").arg(nextLabel));
        msgBox.exec();
    }
}


void MainWindow::on_pushButton_ScriptStop_clicked()
{
    DialogWait wait(this);
    wait.show();

    scriptThread.stop();

    //while (scriptThread.status() != ScriptThread::STOPPED)
    //    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

    QCoreApplication::processEvents();	// To process remaining UART data
    MMS_GlobalStop();
    QCoreApplication::processEvents();	// To process remaining stop data

    // Disconnect signal/slot
    QObject::disconnect(&serialPort, SIGNAL(readyRead()),
                     this, SLOT(on_serialDataFromDevice()));

    QObject::disconnect(this, SIGNAL(sig_serialData(unsigned char, QVector<unsigned char>)),
                     this, SLOT(on_serialDataToDevice(unsigned char, QVector<unsigned char>)));

    ui->pushButton_ScriptExec->setText(QObject::tr("Exec"));
    on_scriptLabel(0);

    ui->pushButton_Port->setEnabled(true);
    ui->pushButton_ScriptLoad->setEnabled(true);
    ui->pushButton_ScriptStop->setEnabled(false);
}


void MainWindow::on_pushButton_ScriptExec_clicked()
{
    if (ui->pushButton_ScriptExec->text() == QObject::tr("Exec"))
    {
        //
        // Exec

        ui->plainTextEdit_Logs->clear();

        if (scriptThread.status() == ScriptThread::INIT || scriptThread.status() == ScriptThread::STOPPED)
        {
            // Connect signal/slot
            QObject::connect(&serialPort, SIGNAL(readyRead()),
                             this, SLOT(on_serialDataFromDevice()));

            QObject::connect(this, SIGNAL(sig_serialData(unsigned char, QVector<unsigned char>)),
                             this, SLOT(on_serialDataToDevice(unsigned char, QVector<unsigned char>)));

            scriptThread.start();
        }
        else
            scriptThread.resume();

        ui->pushButton_ScriptExec->setText(QObject::tr("Pause"));
    }
    else
    {
        //
        // Pause

        scriptThread.pause();
        ui->pushButton_ScriptExec->setText(QObject::tr("Exec"));
    }

    ui->pushButton_Port->setEnabled(false);
    ui->pushButton_ScriptLoad->setEnabled(false);
    ui->pushButton_ScriptStop->setEnabled(true);
}


#include "ScriptThread.h"

#include <QTimer>
#include <QDateTime>

#include "ScriptProcessor.h"
#include "MemeServoAPI/MemeServoAPI.h"


ScriptThread* ScriptThread::_me = NULL;


ScriptThread::ScriptThread()
{
    _me = this;
}


ScriptThread::~ScriptThread()
{
    MMScript_Clean();
}


void ScriptThread::DelayMilisecondImpl(uint32_t ms)
{
    QThread::msleep(ms);
}


uint32_t ScriptThread::GetMilliSecondsImpl()
{
    static uint64_t startTime = QDateTime::currentMSecsSinceEpoch();

    return (uint32_t)(QDateTime::currentMSecsSinceEpoch() - startTime);
}


void ScriptThread::onSerialData(uint8_t data)
{
    MMS_OnData(data);
}


void ScriptThread::SendDataImpl(uint8_t addr, uint8_t *data, uint8_t size)
{
    _me->_sendDataCallback(addr, data, size);
}


void ScriptThread::OnLocalError(uint8_t addr, uint8_t err)
{
    _me->_localErrorCallback(addr, err);
}


void ScriptThread::OnNodeError(uint8_t addr, uint8_t err)
{
    _me->_nodeErrorCallback(addr, err);
}


void ScriptThread::Log(unsigned char node_addr, const char* msg)
{
    _me->_log(node_addr, msg);
}


ScriptThread::STATUS ScriptThread::status() const
{
    return _status;
}


int16_t ScriptThread::init(QString fileName, LABEL_UPDATE_CB labelUpdateCallback, SEND_DATA_CB sendDataCallback, LOCAL_ERROR_CB localErrorCallback, NODE_ERROR_CB nodeErrorCallback, LOG_FUNC log)
{
    _status = ScriptThread::INIT;

    MMS_SetProtocol(MMS_PROTOCOL_UART, 0x01, SendDataImpl, NULL);
    MMS_SetTimerFunction(GetMilliSecondsImpl, DelayMilisecondImpl);
    MMS_SetCommandTimeOut(100);

    _labelUpdateCallback = labelUpdateCallback;
    _sendDataCallback = sendDataCallback;
    _localErrorCallback = localErrorCallback;
    _nodeErrorCallback = nodeErrorCallback;
    _log = log;

    _startLabel = MMScript_ParseScript(fileName.toStdString().c_str());
    return _startLabel;
}


void ScriptThread::run()
{
    _status = ScriptThread::RUNNING;

    _stop = false;
    _pause = false;

    while (_semResume.available())
    {
        _semResume.acquire();
    }

    MMScript_Rewind();

    int16_t nextLabel = _startLabel;

    while (nextLabel > 0)
    {
        if (_pause)
        {
            _pause = false;
            _status = ScriptThread::PAUSED;
            _semResume.acquire();
            _status = ScriptThread::RUNNING;
        }

        if (_stop)
        {
            _status = ScriptThread::STOPPED;
            QThread::exit(0);
            return;
        }

        _labelUpdateCallback(nextLabel);
        nextLabel = MMScript_ExecOneStep(OnLocalError, OnNodeError, DelayMilisecondImpl, Log);
    }

    _status = ScriptThread::STOPPED;
    QThread::exit(-1);

    QString msg = QObject::tr("MMScript_ExecOneStep returned: %1").arg(nextLabel);
    Log(0, msg.toStdString().c_str());

    // Notify main thread
    _labelUpdateCallback(0);
}


void ScriptThread::pause()
{
    while (_semResume.available())
    {
        _semResume.acquire();
    }

    _pause = true;
}


void ScriptThread::resume()
{
   if (_semResume.available() == 0)
       _semResume.release();
}


void ScriptThread::stop()
{
    MMScript_Stop();

    // Soft stop
    _stop = true;
    _semResume.release();

    while (status() != ScriptThread::STOPPED)
        QThread::msleep(100);

    /*
    // Hard stop
    QThread::exit(0);
    _status = ScriptThread::STOPPED;
    */
}

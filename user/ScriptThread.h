#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QSerialPort>

#include <functional>


class ScriptThread : public QThread
{
    Q_OBJECT
    void run() Q_DECL_OVERRIDE;

public:
    enum STATUS {
        INIT,
        RUNNING,
        PAUSED,
        STOPPED
    };
    typedef std::function<void (int16_t)> LABEL_UPDATE_CB;
    typedef std::function<void (uint8_t, uint8_t*, uint8_t)> SEND_DATA_CB;
    typedef std::function<void (uint8_t, uint8_t)> LOCAL_ERROR_CB;
    typedef std::function<void (uint8_t, uint8_t)> NODE_ERROR_CB;
    typedef std::function<void (uint8_t, const char*)> LOG_FUNC;

    ScriptThread();
    virtual ~ScriptThread();

    int16_t init(QString fileName, LABEL_UPDATE_CB labelUpdateCallback, SEND_DATA_CB sendDataCallback, LOCAL_ERROR_CB localErrorCallback, NODE_ERROR_CB nodeErrorCallback, LOG_FUNC log);
    void pause();
    void resume();
    void stop();

    void onSerialData(uint8_t data);

    STATUS status() const;

private:
    static void DelayMilisecondImpl(uint32_t ms);
    static uint32_t GetMilliSecondsImpl();
    static void Log(unsigned char node_addr, const char* msg);

    static void SendDataImpl(uint8_t addr, uint8_t *data, uint8_t size);
    static void OnLocalError(uint8_t addr, uint8_t err);
    static void OnNodeError(uint8_t addr, uint8_t err);

    static ScriptThread *_me;

    volatile STATUS _status;
    int16_t _startLabel;
    QSemaphore _semResume;
    volatile bool _pause;
    volatile bool _stop;

    LABEL_UPDATE_CB _labelUpdateCallback;
    SEND_DATA_CB _sendDataCallback;
    LOCAL_ERROR_CB _localErrorCallback;
    NODE_ERROR_CB _nodeErrorCallback;
    LOG_FUNC _log;
};

#endif // SCRIPTTHREAD_H

#ifndef SERIALLINK_H
#define SERIALLINK_H

#include <QObject>
#include <QString>
#include <QStringList>

// 串口通信封装（Windows API 实现，无需 Qt SerialPort 模块）
class SerialLink : public QObject
{
    Q_OBJECT
public:
    explicit SerialLink(QObject *parent = nullptr);
    ~SerialLink() override;

    bool open(const QString& portName);    // 打开 COM 口（115200-8-N-1）
    void close();
    bool isOpen() const;
    bool sendOpen(const QString& name);    // 发送 "OPEN:名字\r\n"

private:
    void* m_handle;   // Windows HANDLE
};

#endif // SERIALLINK_H

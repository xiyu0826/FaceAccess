#include "seriallink.h"
#include <QDebug>
#define NOMINMAX
#include <windows.h>

SerialLink::SerialLink(QObject *parent)
    : QObject(parent), m_handle(nullptr)
{
}

SerialLink::~SerialLink()
{
    close();
}

bool SerialLink::open(const QString& portName)
{
    close();

    // Windows 下 COM 口用 \\.\COMx 打开
    QString path = "\\\\.\\" + portName;
    HANDLE h = CreateFileA(path.toLatin1().constData(),
                           GENERIC_READ | GENERIC_WRITE,
                           0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        qDebug() << "serial open failed:" << portName;
        return false;
    }

    // 设置波特率 115200、8 数据位、无校验、1 停止位（和 STM32 一致）
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(h, &dcb);
    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(h, &dcb);

    m_handle = h;
    qDebug() << "serial opened:" << portName;
    return true;
}

void SerialLink::close()
{
    if (m_handle)
    {
        CloseHandle((HANDLE)m_handle);
        m_handle = nullptr;
    }
}

bool SerialLink::isOpen() const
{
    return m_handle != nullptr;
}

bool SerialLink::sendOpen(const QString& name)
{
    if (!m_handle)
        return false;

    QString cmd = "OPEN:" + name + "\r\n";
    QByteArray data = cmd.toUtf8();
    DWORD written = 0;
    if (!WriteFile((HANDLE)m_handle, data.constData(), data.size(), &written, nullptr))
        return false;
    return written == (DWORD)data.size();
}

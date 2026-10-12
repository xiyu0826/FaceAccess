#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <opencv2/opencv.hpp>
#include "person.h"
#include "seriallink.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void updateFrame();                     // 定时器槽：取帧→识别→显示
    void on_pushButton_enroll_clicked();    // 录入人员：拍照→检测→提取特征→存库
    void on_pushButton_records_clicked();   // 门禁记录：刷新表格
    void on_pushButton_open_port_clicked(); // 打开串口

private:
    void refreshRecords();                  // 查询门禁记录并刷新表格

    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    QList<Person> persons;                  // 缓存的人员列表（避免每帧查库）
    QString lastRecognizedName;             // 上一次识别到的人，用于去重
    SerialLink *serial;                     // 串口通信
};

#endif // MAINWINDOW_H

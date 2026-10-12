#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mydatabase.h"
#include "faceengine.h"
#include "recorddialog.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化串口 + 填充 COM 口列表（COM1~COM16，可手动输入）
    serial = new SerialLink(this);
    for (int i = 1; i <= 16; i++)
        ui->comboBox_port->addItem("COM" + QString::number(i));
    ui->comboBox_port->setEditable(true);

    // 初始化数据库 + 载入人员列表（缓存在内存，避免每帧查库）
    MyDatabase::instance();
    persons = MyDatabase::instance()->queryPersons();

    // 打开摄像头：0 是默认摄像头。iVCam 连接后通常也是 0；
    // 如果画面黑屏或打不开，把 0 改成 1 试试
    cap.open(0);
    if (!cap.isOpened())
    {
        ui->label_result->setText("摄像头打开失败，请检查 iVCam 是否已连接");
    }

    // 定时器：每 100ms 取一帧刷新（约 10 帧/秒）
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    timer->start();
}

MainWindow::~MainWindow()
{
    cap.release();
    delete ui;
}

void MainWindow::updateFrame()
{
    cv::Mat frame;
    cap.read(frame);
    if (frame.empty())
        return;

    // 统一缩放到 640x480（和 FaceEngine 输入一致，方便画框定位）
    cv::resize(frame, frame, cv::Size(FaceEngine::INPUT_W, FaceEngine::INPUT_H));

    // 检测人脸
    cv::Mat faces = FaceEngine::instance()->detect(frame);

    if (faces.rows > 0)
    {
        // 提取第一张脸的特征（128 维）
        cv::Mat feature = FaceEngine::instance()->extract(frame, faces, 0);

        // 和库里所有人比对，取最高相似度
        double best = 0;
        QString bestName;
        for (const Person& p : persons)
        {
            double d = FaceEngine::instance()->match(feature, p.feature());
            if (d > best)
            {
                best = d;
                bestName = p.name();
            }
        }

        // 画框：绿色=识别成功，红色=陌生人
        float x = faces.at<float>(0, 0);
        float y = faces.at<float>(0, 1);
        float w = faces.at<float>(0, 2);
        float h = faces.at<float>(0, 3);
        bool matched = (best >= FaceEngine::THRESHOLD && !bestName.isEmpty());
        cv::Scalar color = matched ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        cv::rectangle(frame, cv::Rect((int)x, (int)y, (int)w, (int)h), color, 2);

        // 帧上写字（OpenCV 只支持英文，中文放 label_result 里显示）
        std::string tag = matched ? "MATCH" : "UNKNOWN";
        cv::putText(frame, tag, cv::Point((int)x, (int)y - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2);

        if (matched)
        {
            ui->label_result->setText("识别成功：" + bestName + "  ("
                                      + QString::number(best, 'f', 3) + ")");

            // 去重：同一个人连续多帧只记一次，直到他离开画面
            if (lastRecognizedName != bestName)
            {
                lastRecognizedName = bestName;
                MyDatabase::instance()->addAccessLog(bestName, "识别成功");
                refreshRecords();
                serial->sendOpen(bestName);   // 串口发开门命令
            }
        }
        else
        {
            ui->label_result->setText("陌生人  (" + QString::number(best, 'f', 3) + ")");
            lastRecognizedName = "";   // 陌生人时重置，下个人出现能重新触发
        }
    }
    else
    {
        lastRecognizedName = "";   // 没检测到脸也重置
    }

    // 显示
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step[0], QImage::Format_BGR888);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    ui->label_video->setPixmap(pixmap.scaled(ui->label_video->size(),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
}

void MainWindow::on_pushButton_enroll_clicked()
{
    // 1. 取当前一帧（点击瞬间拍一张）
    cv::Mat frame;
    cap.read(frame);
    if (frame.empty())
    {
        ui->label_result->setText("取帧失败");
        return;
    }

    // 2. 检测人脸
    cv::Mat faces = FaceEngine::instance()->detect(frame);
    if (faces.rows == 0)
    {
        ui->label_result->setText("未检测到人脸，请正对摄像头");
        return;
    }

    // 3. 提取第一张脸的特征（128 维）
    cv::Mat feature = FaceEngine::instance()->extract(frame, faces, 0);

    // 4. 弹窗输入姓名
    bool ok = false;
    QString name = QInputDialog::getText(this, "录入人员", "请输入姓名：",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
        return;

    // 5. 存入数据库
    if (MyDatabase::instance()->addPerson(name, feature))
    {
        ui->label_result->setText("录入成功：" + name);
        persons = MyDatabase::instance()->queryPersons();   // 刷新缓存
    }
    else
        ui->label_result->setText("录入失败");
}

void MainWindow::on_pushButton_records_clicked()
{
    RecordDialog d(this);
    d.exec();
}

void MainWindow::refreshRecords()
{
    QList<QStringList> logs = MyDatabase::instance()->queryAccessLogs();

    ui->tableWidget_records->setColumnCount(4);
    ui->tableWidget_records->setRowCount(logs.size());
    ui->tableWidget_records->setHorizontalHeaderLabels({"编号", "姓名", "结果", "时间"});
    ui->tableWidget_records->verticalHeader()->setVisible(false);   // 隐藏左侧行号

    for (int i = 0; i < logs.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ui->tableWidget_records->setItem(i, j, new QTableWidgetItem(logs[i][j]));
        }
    }
}

void MainWindow::on_pushButton_open_port_clicked()
{
    QString port = ui->comboBox_port->currentText();
    if (port.isEmpty())
        return;

    if (serial->open(port))
        ui->label_result->setText("串口已打开：" + port);
    else
        ui->label_result->setText("串口打开失败：" + port);
}

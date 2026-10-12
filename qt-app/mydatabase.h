#ifndef MYDATABASE_H
#define MYDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <opencv2/opencv.hpp>

#include "person.h"

// 封装数据库操作的单例类型（复用 demo11 思路）
class MyDatabase : public QObject
{
    Q_OBJECT
public:
    static MyDatabase* instance();

    // 人员管理
    bool addPerson(QString name, const cv::Mat& feature);
    bool deletePerson(int id);
    QList<Person> queryPersons();

    // 开锁记录
    bool addAccessLog(QString name, QString result);
    QList<QStringList> queryAccessLogs();   // 每行: [id, name, result, time]

private:
    explicit MyDatabase(QObject *parent = nullptr);
    MyDatabase(const MyDatabase&) = delete;
    MyDatabase& operator=(const MyDatabase&) = delete;

    QSqlDatabase db;
};

#endif // MYDATABASE_H

#include "mydatabase.h"
#include "pathutil.h"
#include <QDebug>

// cv::Mat(1x128 CV_32F) -> QByteArray(512 字节)
static QByteArray featureToBytes(const cv::Mat& feature)
{
    return QByteArray((const char*)feature.data,
                      (int)(feature.total() * feature.elemSize()));
}

// QByteArray -> cv::Mat(1x128 CV_32F)
static cv::Mat bytesToFeature(const QByteArray& ba)
{
    if (ba.size() != 128 * (int)sizeof(float))   /* 必须是 512 字节，否则库里数据损坏，防越界 */
        return cv::Mat();
    cv::Mat f(1, 128, CV_32F);
    memcpy(f.data, ba.constData(), ba.size());
    return f;
}


MyDatabase::MyDatabase(QObject *parent)
    : QObject{parent}
{
    // 初始化数据库（QSQLITE 免安装、单文件）
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(appPath("access.db", APP_ROOT_DIR "/access.db"));
    bool ok = db.open();
    if (!ok)
    {
        qDebug() << "open db error: " << db.lastError().text();
        return;
    }

    // 建两张表（IF NOT EXISTS 保证重复运行不报错）
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS personnel ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "feature BLOB NOT NULL, "
               "created_at TEXT)");
    query.exec("CREATE TABLE IF NOT EXISTS access_log ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT, "
               "result TEXT, "
               "time TEXT)");
}

MyDatabase *MyDatabase::instance()
{
    static MyDatabase d;
    return &d;
}

bool MyDatabase::addPerson(QString name, const cv::Mat& feature)
{
    QSqlQuery query;
    query.prepare("INSERT INTO personnel (name, feature, created_at) "
                  "VALUES (:name, :feature, :time)");
    query.bindValue(":name", name);
    query.bindValue(":feature", featureToBytes(feature));
    query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    bool ok = query.exec();
    if (!ok)
    {
        qDebug() << "add person error: " << query.lastError().text();
        return false;
    }
    return true;
}

bool MyDatabase::deletePerson(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM personnel WHERE id = :id");
    query.bindValue(":id", id);
    bool ok = query.exec();
    if (!ok)
    {
        qDebug() << "delete person error: " << query.lastError().text();
        return false;
    }
    return true;
}

QList<Person> MyDatabase::queryPersons()
{
    QList<Person> list;

    QSqlQuery query;
    bool ok = query.exec("SELECT id, name, feature FROM personnel");
    if (!ok)
    {
        qDebug() << "query persons error: " << query.lastError().text();
        return list;
    }

    while (query.next())
    {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        cv::Mat feature = bytesToFeature(query.value("feature").toByteArray());

        list.push_back(Person(id, name, feature));
    }
    return list;
}

bool MyDatabase::addAccessLog(QString name, QString result)
{
    QSqlQuery query;
    query.prepare("INSERT INTO access_log (name, result, time) "
                  "VALUES (:name, :result, :time)");
    query.bindValue(":name", name);
    query.bindValue(":result", result);
    query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    bool ok = query.exec();
    if (!ok)
    {
        qDebug() << "add log error: " << query.lastError().text();
        return false;
    }
    return true;
}

QList<QStringList> MyDatabase::queryAccessLogs()
{
    QList<QStringList> list;

    QSqlQuery query;
    bool ok = query.exec("SELECT id, name, result, time FROM access_log ORDER BY id DESC");
    if (!ok)
    {
        qDebug() << "query logs error: " << query.lastError().text();
        return list;
    }

    while (query.next())
    {
        QStringList row;
        row << query.value("id").toString()
            << query.value("name").toString()
            << query.value("result").toString()
            << query.value("time").toString();
        list.push_back(row);
    }
    return list;
}

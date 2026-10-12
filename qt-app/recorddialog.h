#ifndef RECORDDIALOG_H
#define RECORDDIALOG_H

#include <QDialog>

class QTableWidget;

// 开锁记录查询窗口：独立弹出，方便查看全部记录
class RecordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecordDialog(QWidget *parent = nullptr);

private slots:
    void refreshRecords();

private:
    QTableWidget *table;
};

#endif // RECORDDIALOG_H

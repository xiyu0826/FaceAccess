#include "recorddialog.h"
#include "mydatabase.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

RecordDialog::RecordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("门禁记录查询");
    resize(620, 440);

    table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"编号", "姓名", "结果", "时间"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);   // 隐藏左侧行号

    QPushButton *btnRefresh = new QPushButton("刷新", this);
    connect(btnRefresh, &QPushButton::clicked, this, &RecordDialog::refreshRecords);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(table);
    layout->addWidget(btnRefresh);

    refreshRecords();
}

void RecordDialog::refreshRecords()
{
    QList<QStringList> logs = MyDatabase::instance()->queryAccessLogs();
    table->setRowCount(logs.size());
    for (int i = 0; i < logs.size(); i++)
    {
        for (int j = 0; j < 4; j++)
            table->setItem(i, j, new QTableWidgetItem(logs[i][j]));
    }
}

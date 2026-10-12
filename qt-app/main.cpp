#include "mainwindow.h"
#include "pathutil.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载 QSS 样式表（深色主题）
    QFile styleFile(appPath("style.qss", APP_PROJECT_DIR "/style.qss"));
    if (styleFile.open(QFile::ReadOnly))
    {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}

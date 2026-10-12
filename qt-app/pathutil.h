#ifndef PATHUTIL_H
#define PATHUTIL_H

#include <QCoreApplication>
#include <QFile>
#include <QString>

// 工程目录与仓库根目录，由 .pro 编进来；
// 没定义时退回当前目录，避免换个构建方式（比如不用 qmake）就编不过
#ifndef APP_PROJECT_DIR
#define APP_PROJECT_DIR "."
#endif
#ifndef APP_ROOT_DIR
#define APP_ROOT_DIR ".."
#endif

// 资源路径解析。
//
// 优先按 exe 同目录找（发布打包后 exe 旁边就是 models/、style.qss）；
// 找不到再用 fallback —— 开发时 exe 在 build/xxx/ 底下，资源其实还在工程目录里，
// 所以 fallback 由 .pro 里的 APP_PROJECT_DIR / APP_ROOT_DIR 指到工程目录和仓库根目录。
inline QString appPath(const QString& rel, const QString& fallback)
{
    QString p = QCoreApplication::applicationDirPath() + "/" + rel;
    return QFile::exists(p) ? p : fallback;
}

#endif // PATHUTIL_H

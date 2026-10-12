QT += widgets sql
# QT += serialport   # 未安装该模块，改用 Windows API 实现串口（见 seriallink.cpp）

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    faceengine.cpp \
    main.cpp \
    mainwindow.cpp \
    mydatabase.cpp \
    person.cpp \
    recorddialog.cpp \
    seriallink.cpp

HEADERS += \
    faceengine.h \
    mainwindow.h \
    mydatabase.h \
    person.h \
    recorddialog.h \
    seriallink.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# 资源查找的回退目录。
# 开发运行时 exe 在 build/xxx/ 下，资源其实还在工程目录里，所以把这两个目录
# 编进去兜底；发布时优先找 exe 同目录（见 pathutil.h）。
DEFINES += APP_PROJECT_DIR=\\\"$$PWD\\\"
DEFINES += APP_ROOT_DIR=\\\"$$PWD/..\\\"

# 指定 OpenCV 路径。可在 qmake 命令行覆盖：
#   qmake "OPENCV_DIR=D:/opencv4.5.5-MinGw7.3.0" FaceAccess.pro
isEmpty(OPENCV_DIR): OPENCV_DIR = $$(OPENCV_DIR)
isEmpty(OPENCV_DIR): OPENCV_DIR = E:/opencv4.5.5-MinGw7.3.0

INCLUDEPATH += $$OPENCV_DIR/include
LIBS += -L$$OPENCV_DIR/x64/mingw/bin -lopencv_world455

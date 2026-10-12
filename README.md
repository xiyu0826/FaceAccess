# FaceAccess —— 人脸识别开锁系统

Qt 上位机用 OpenCV 实时识别来者身份，识别通过后经串口通知 STM32 驱动舵机开锁，
并把每一次开锁记录写进 SQLite；STM32 侧另经 MQTT 把开锁人与环境数据上报网页看板。

```
┌──────────────────── Qt 上位机（Windows）─────────────────────┐
│ 摄像头 → YuNet 人脸检测 → SFace 特征提取 → 库内比对          │
│ 人员管理（特征存 SQLite）· 开锁记录 access_log               │
│ 识别通过 → 串口发 "OPEN:姓名\r\n"                            │
└──────────────────────────────┬───────────────────────────────┘
                               │ 串口 115200 8N1（CH340 USB）
┌──────────────────────────────▼──────────── STM32 执行端 ─────┐
│ 串口中断收命令 → 舵机开锁 + 蜂鸣 + LED 提示                  │
│ 开锁人姓名存入 door_last_name → 随环境数据一起 MQTT 上报      │
└──────────────────────────────┬───────────────────────────────┘
                               │ MQTT over WiFi（ESP8266）
┌──────────────────────────────▼───────────────────────────────┐
│ dashboard.py：环境数据 + 布防/报警 +「最近开锁」姓名          │
└──────────────────────────────────────────────────────────────┘
```

## 功能

- **人脸识别开锁**：实时取帧 → 检测 → 提 128 维特征 → 与库内人员做余弦相似度比对，超过阈值判定身份
- **人员管理**：录入（拍照提取特征入库）、删除、列表查看
- **开锁记录**：每次识别结果写库，支持查询；同一人连续多帧只记一次
- **硬件执行**：舵机开锁 + 蜂鸣 + LED 提示，2 秒后自动复位
- **环境采集**：温湿度、空气质量、光照、人体感应，OLED 本地显示
- **远程看板**：MQTT 上报环境数据与开锁人，网页端可查看并远程布防/撤防

## 目录结构

```
.
├── qt-app/                 # Qt 上位机（Qt 6.8.3 MinGW 64-bit）
│   ├── FaceAccess.pro      # 工程文件：OpenCV 路径、资源回退宏
│   ├── main.cpp            # 入口：加载 QSS + 主窗口
│   ├── mainwindow.*        # 主窗口：取帧 → 识别 → 显示 → 写库 → 发串口
│   ├── faceengine.*        # 单例：YuNet 检测 + SFace 特征 / 比对
│   ├── mydatabase.*        # 单例：SQLite 两张表 + 特征 BLOB 存取
│   ├── person.*            # 人员数据类（id + 姓名 + 128 维特征）
│   ├── recorddialog.*      # 记录查询对话框
│   ├── seriallink.*        # 串口（Windows API 实现，不依赖 Qt SerialPort）
│   ├── pathutil.h          # 资源路径解析（exe 同目录优先，工程目录兜底）
│   └── style.qss           # 深色主题样式表
├── stm32/                  # STM32F103RCT6 固件（Keil MDK5 + HAL + FreeRTOS）
│   ├── SYSTEM/             # 驱动层：舵机 / 门禁逻辑 / OLED / DHT11 / MQ-135 / 光敏 / PIR / DS3231 / MQTT / WiFi
│   ├── USER/
│   │   ├── smart_home/     # 主工程：6 个 FreeRTOS 任务（src/ + keil_project/）
│   │   ├── board_test/     # 板级自检工程
│   │   ├── gpio/           # GPIO 测试工程
│   │   └── periph_test/    # 外设测试工程
│   ├── CORE/               # CMSIS 内核 + 启动文件
│   ├── STM32F1xx_HAL_Driver/
│   └── FreeRTOS/           # 内核源码（heap_4）
├── dashboard/              # 网页看板（Flask + paho-mqtt）
│   └── dashboard.py
├── hardware/原理图/         # 开发板原理图分区截图（12 张）
├── models/                 # 人脸模型（不入库，见「依赖」）
└── docs/
    └── 设计说明.md          # 技术选型、实现细节、踩坑记录
```

## 硬件

固件跑在 **STM32F103RCT6 开发板**上，板载 CH340（串口）、ESP8266（WiFi）、OLED 接口、
3 路舵机接口、蜂鸣器、8 路独立 LED 与按键、SPI-Flash、ADC 排针。
原理图分区截图见 [`hardware/原理图/`](hardware/原理图/)。

外接模块接线：

| 模块 | 供电 | 信号脚 | GPIO | 排针 |
|---|---|---|---|---|
| DHT11 温湿度 | 3.3V | DAT | PC1 | H9_1 |
| DS3231 时钟 | 3.3V | SCL / SDA | PB6 / PB7 | H6_4 / H7_3 |
| OLED 屏 | 3.3V | SCL / SDA | PB6 / PB7 | H6_4 / H7_3 |
| MQ-135 空气质量 | 5V | AO | PB0 | H9_8 |
| 光敏电阻 | 5V | DO | PA6 | H8_6 |
| HC-SR501 人体感应 | 5V | OUT | PB12 | H8_11 |
| 舵机 SG90（锁舌） | 5V | 信号 | PB1（TIM3_CH4） | SG3 |

> DS3231 与 OLED 共用 I2C（从机地址 0x68 / 0x3C 不冲突）。
> 所有模块 GND 共地。舵机必须接 5V，不要接 3.3V。

## 依赖

| 依赖 | 版本 | 用途 |
|---|---|---|
| Qt | 6.8.3（MinGW 64-bit） | 上位机界面 + SQLite |
| OpenCV | 4.5.5（MinGW） | 人脸检测与特征比对 |
| Keil MDK | 5.x | STM32 固件编译 |
| Python | 3.8+ | 运行网页看板 |
| Flask / paho-mqtt | — | 看板 Web 服务与 MQTT 客户端 |

**人脸模型不入库**（合计约 39 MB），需要自己下载后放到仓库根目录的 `models/`：

| 文件 | 来源 |
|---|---|
| `face_detection_yunet_2022mar.onnx` | [OpenCV Model Zoo](https://github.com/opencv/opencv_zoo/tree/main/models/face_detection_yunet) |
| `face_recognition_sface_2021dec.onnx` | [OpenCV Model Zoo](https://github.com/opencv/opencv_zoo/tree/main/models/face_recognition_sface) |

这两个都是 OpenCV 官方提供的**预训练**模型，本项目的比对逻辑是自己写的。

## 编译与运行

### 1. Qt 上位机

用 Qt Creator 打开 `qt-app/FaceAccess.pro`，确认 `OPENCV_DIR` 指向本机 OpenCV
（默认 `E:/opencv4.5.5-MinGw7.3.0`），直接构建运行即可。也可以在命令行覆盖：

```bash
qmake "OPENCV_DIR=D:/opencv4.5.5-MinGw7.3.0" qt-app/FaceAccess.pro
```

主界面选择串口号（COM1~COM16 下拉，可手动输入）后点「打开串口」。

> 工程只用了 `QT += widgets sql`，**没有用 Qt SerialPort 模块**——串口走 Windows API
> 实现（`seriallink.cpp`），好处是不用额外装模块，代价是串口列表得自己选。

### 2. STM32 固件

用 Keil MDK5 打开 `stm32/USER/smart_home/keil_project/STM32F1xx_HAL.uvprojx`，
编译下载。工程里引用库源码用的是相对路径，放在这个目录结构下可以直接编译。

烧录前把 `stm32/USER/smart_home/src/main.c` 顶部的 WiFi 配置改成自己的热点：

```c
#define WIFI_SSID   "your_wifi_ssid"
#define WIFI_PWD    "your_wifi_password"
```

> ESP8266 只支持 2.4G 频段。

`USER/` 下另外三个工程是开发过程中用于分步验证的：`board_test` 板级自检、
`gpio` 引脚测试、`periph_test` 外设逐个测试。

### 3. 网页看板

```bash
pip install flask paho-mqtt
python dashboard/dashboard.py
```

浏览器打开 `http://<电脑IP>:8080`。看板订阅公共 broker `broker.emqx.io`
的 `smart_home/test` 主题，STM32 每 2 秒上报一次。

## 核心实现

**人脸识别**（`faceengine.cpp`）
YuNet 检测前先把帧统一缩放到 640×480（该模型要求固定输入尺寸）；SFace 按地标裁出
112×112 的规范脸后提取 128 维特征。比对用余弦相似度，阈值 **0.363**：误识别多就调高，
认不出来就调低。模型加载约 1 秒，所以 `FaceEngine` 做成单例，全程只加载一次。

**特征存取**（`mydatabase.cpp`）
SQLite 不认识 `cv::Mat`，特征按 `1×128 CV_32F` → **512 字节** `QByteArray` 存进 BLOB。
读回来时先校验长度必须是 512，否则视为数据损坏直接返回空 Mat，避免按错误的尺寸 memcpy。

**取帧与显示**（`mainwindow.cpp`）
用 `QTimer` 每 100ms 取一帧，而不是 `while(1)` 阻塞主线程。OpenCV 是 BGR 顺序，
转 `QImage` 时要用 `Format_BGR888`。识别结果画在帧上（绿框 / 红框），
中文姓名放在 Qt 的 label 里——OpenCV 的 `putText` 画不了中文。

**同一人只记一次**
`lastRecognizedName` 记住上一帧识别到的人，连续多帧同一个人只写一条记录，
人离开画面才重置。人员特征列表启动时一次性读进内存，识别过程不查库。

**串口协议**（`door.c`）
UART1 中断接收 + **128 字节环形缓冲**，`door_task` 每 20ms 轮询取字节，攒到 `\n` 才解析。
115200 波特率下每字节间隔约 87 µs，轮询方式会丢字节，必须用中断加缓冲。
收到 `OPEN` 或 `OPEN:姓名` 后：舵机转 90° + 蜂鸣 + LED3 亮，2 秒后复位。
姓名会被提取到 `door_last_name`，供 MQTT 上报。

**舵机 PWM**（`servo.c`）
TIM3_CH4（PB1），预分频 `72-1` 把 72MHz 降到 1MHz，ARR 设 `20000-1` 得到 50Hz 周期，
CCR 直接就是脉宽微秒数：500 → 0°，1500 → 90°，2500 → 180°。

**FreeRTOS 任务**（`main.c`）
六个任务：`sensor`（采集）、`display`（OLED 刷新）、`alarm`（报警）、`mqtt`（上报）、
`key`（按键）、`door`（串口命令与开锁）。用 heap_4，堆配置为 16KB。

**MQTT 与看板**
STM32 侧 `mqtt.c` 自己拼 MQTT 报文（CONNECT / PUBLISH / SUBSCRIBE），
经 ESP8266 的 AT 指令走 WiFi；上报主题 `smart_home/test`，指令主题 `smart_home/cmd`。
看板是 Flask + paho-mqtt，把最近一次上报的数据渲染成网页，并提供布防/撤防按钮。

> 更多选型理由、实现细节与踩过的坑，见 [`docs/设计说明.md`](docs/设计说明.md)。

## 已知限制

- **只做了开锁提示，没做陌生人告警**：识别失败时上位机只是画红框，没有通知 STM32 蜂鸣
- **串口靠手动选**：没装 Qt SerialPort 模块，串口列表是 COM1~COM16 下拉，换电脑或换 USB 口要自己确认
- **摄像头索引写死**：`mainwindow.cpp` 里是 `cap.open(0)`，用虚拟摄像头或视频文件时需要改这一行
- **报警默认开启且阈值偏低**：`g_alarm_enabled` 默认为 1，室内空气质量一飘高就蜂鸣，会盖掉开锁提示音，演示前建议先撤防
- **未做 QSS 之外的界面美化**，也没有做免安装打包
- MQTT 用的是公共 broker，任何人都能订阅，仅适合演示

## 开发环境

- 上位机：Windows，Qt 6.8.3 MinGW 64-bit，OpenCV 4.5.5（MinGW 7.3.0 构建）
- 固件：STM32F103RCT6，Keil MDK5，STM32F1xx HAL，FreeRTOS（heap_4）
- 看板：Python 3 + Flask + paho-mqtt

#include "faceengine.h"
#include "pathutil.h"

FaceEngine* FaceEngine::instance()
{
    // C++11 起，静态局部对象的初始化是线程安全的，所以懒汉式单例这样写即可
    static FaceEngine s;
    return &s;
}

FaceEngine::FaceEngine()
{
    // 模型路径：优先 exe 同目录 models/（发布），开发时回退到仓库根目录
    QString fd = appPath("models/face_detection_yunet_2022mar.onnx",
                         APP_ROOT_DIR "/models/face_detection_yunet_2022mar.onnx");
    QString fr = appPath("models/face_recognition_sface_2021dec.onnx",
                         APP_ROOT_DIR "/models/face_recognition_sface_2021dec.onnx");

    detector = cv::FaceDetectorYN::create(fd.toStdString(), "", cv::Size(INPUT_W, INPUT_H));
    recognizer = cv::FaceRecognizerSF::create(fr.toStdString(), "");
}

cv::Mat FaceEngine::detect(const cv::Mat& frame)
{
    // YuNet 要求固定输入尺寸，先统一缩放到 640x480
    cv::Mat input;
    if (frame.cols != INPUT_W || frame.rows != INPUT_H)
        cv::resize(frame, input, cv::Size(INPUT_W, INPUT_H));
    else
        input = frame;

    cv::Mat faces;
    detector->detect(input, faces);
    return faces;
}

cv::Mat FaceEngine::extract(const cv::Mat& frame, const cv::Mat& faces, int i)
{
    cv::Mat input;
    if (frame.cols != INPUT_W || frame.rows != INPUT_H)
        cv::resize(frame, input, cv::Size(INPUT_W, INPUT_H));
    else
        input = frame;

    // 根据人脸地标，裁出 112x112 的规范脸
    cv::Mat aligned;
    recognizer->alignCrop(input, faces.row(i), aligned);

    // 提取 128 维特征，并深拷贝（避免被后续操作覆盖）
    cv::Mat feature;
    recognizer->feature(aligned, feature);
    feature = feature.clone();
    return feature;
}

double FaceEngine::match(const cv::Mat& f1, const cv::Mat& f2)
{
    return recognizer->match(f1, f2);
}

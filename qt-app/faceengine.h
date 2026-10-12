#ifndef FACEENGINE_H
#define FACEENGINE_H

#include <opencv2/opencv.hpp>

// 人脸引擎：封装 YuNet 人脸检测 + SFace 特征提取/比对
// 单例模式（复用 demo11 MyDatabase 的思路），onnx 模型只在第一次使用时加载一次
class FaceEngine
{
public:
    static FaceEngine* instance();

    // 检测一帧中的所有人脸，返回 N 行 14 列的 Mat
    // 每行: [x, y, w, h, 5 个关键点(x,y)]，坐标基于 640x480 输入
    cv::Mat detect(const cv::Mat& frame);

    // 提取 faces 第 i 个人脸的 128 维特征 (1x128 CV_32F)
    cv::Mat extract(const cv::Mat& frame, const cv::Mat& faces, int i);

    // 比对两个特征，返回余弦相似度，>= 0.363 视为同一人
    double match(const cv::Mat& f1, const cv::Mat& f2);

    // 固定输入尺寸 + 识别阈值
    static constexpr int INPUT_W = 640;
    static constexpr int INPUT_H = 480;
    static constexpr double THRESHOLD = 0.363;

private:
    FaceEngine();
    FaceEngine(const FaceEngine&) = delete;
    FaceEngine& operator=(const FaceEngine&) = delete;

    cv::Ptr<cv::FaceDetectorYN> detector;
    cv::Ptr<cv::FaceRecognizerSF> recognizer;
};

#endif // FACEENGINE_H

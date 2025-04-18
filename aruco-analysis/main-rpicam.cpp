#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <string>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

//1545*1393
const string MAP_PATH = R"(/home/kozumi/snap/code-server/common/aruco-analysis/11.png)";

struct ArUcoMarkerInfo 
{
    Point2d actual;  // 码标实际坐标（米）
    Point2d pixel;   // 码标像素坐标
    int id;          // 码标ID
};

Mat cameraMatrix = (Mat_<double>(3, 3) <<
    800, 0, 320,
    0, 800, 240,
    0, 0, 1);

Mat distCoeffs = (Mat_<double>(5, 1) << 0, 0, 0, 0, 0);

// 修改后的已知码标数据（确保刚性变换条件）
vector<ArUcoMarkerInfo> knownMarkers = 
{
    {Point2d(0, 0),   Point2d(100, 100), 0},  // 实际坐标原点 → 像素(100,100)
    {Point2d(1, 0),   Point2d(200, 100), 1},  // X方向缩放100像素/单位
    {Point2d(0, 1),   Point2d(100, 200), 2},  // Y方向缩放100像素/单位
    {Point2d(1, 1),   Point2d(200, 200), 3}   // 对角点
};

// 计算变换矩阵（修正参数顺序和错误处理）
Mat calculateTransformMatrix(const vector<ArUcoMarkerInfo>& markers) 
{
    if (markers.size() < 3) throw runtime_error("需至少3个码标");

    vector<Point2f> actualPoints, pixelPoints;
    for (const auto& marker : markers) {
        actualPoints.push_back(Point2f(marker.actual.x, marker.actual.y));
        pixelPoints.push_back(Point2f(marker.pixel.x, marker.pixel.y));
    }

    Mat inliers;
    Mat transformMatrix = cv::estimateAffinePartial2D(
        actualPoints, pixelPoints, inliers, cv::RANSAC, 
        5.0, static_cast<size_t>(2000), 0.99, static_cast<size_t>(10)
    );

    if (transformMatrix.empty() || transformMatrix.rows != 2 || transformMatrix.cols != 3) {
        cerr << "变换矩阵计算失败，请检查输入数据：" << endl;
        for (const auto& marker : markers) {
            cerr << "实际坐标: (" << marker.actual.x << ", " << marker.actual.y << ") "
                 << "→ 像素坐标: (" << marker.pixel.x << ", " << marker.pixel.y << ")" << endl;
        }
        throw runtime_error("矩阵计算失败");
    }
    return transformMatrix;
}

// 修正后的坐标逆变换函数
Point2d pixelToActual(const Point2d& pixel, const Mat& transformMatrix) 
{
    Mat invTransform;
    cv::invertAffineTransform(transformMatrix, invTransform); // 正确求逆

    // 构造齐次坐标 [x, y, 1]
    Mat srcPoint = (Mat_<double>(3, 1) << pixel.x, pixel.y, 1.0);

    // 执行变换 (2x3矩阵 × 3x1向量 → 2x1结果)
    Mat dstPoint = invTransform * srcPoint;
    return Point2d(dstPoint.at<double>(0), dstPoint.at<double>(1));
}

Mat loadNavigationMap(const string& path) 
{
    Mat map = imread(path, IMREAD_COLOR);
    if (map.empty()) {
        throw runtime_error("地图加载失败，请检查路径：" + path);
    }
    return map;
}

// 获取最大编号的jpg文件
string getMaxNumberedJpgFile() {
    int maxNumber = -1;
    string maxFile;
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        if (entry.is_regular_file() && entry.path().extension() == ".jpg") {
            string filename = entry.path().stem().string();
            try {
                int number = stoi(filename);
                if (number > maxNumber) {
                    maxNumber = number;
                    maxFile = entry.path().string();
                }
            } catch (const invalid_argument&) {
                continue;
            }
        }
    }
    return maxFile;
}

int main()
{
    try 
    {
        Mat mapCanvas = loadNavigationMap(MAP_PATH);

        Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::DICT_4X4_1000);
        Ptr<aruco::DetectorParameters> parameters = aruco::DetectorParameters::create();
        parameters->cornerRefinementMethod = aruco::CORNER_REFINE_SUBPIX;

        Mat transformMatrix = calculateTransformMatrix(knownMarkers);
        cout << "变换矩阵:\n" << transformMatrix << endl;

        while (true) 
        {
            string filename = getMaxNumberedJpgFile();
            if (filename.empty()) {
                cerr << "未找到jpg文件" << endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            Mat frame = imread(filename, IMREAD_COLOR);
            if (frame.empty()) {
                cerr << "图像加载失败: " << filename << endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            // 摄像头畸变校正
            Mat undistortedFrame;
            undistort(frame, undistortedFrame, cameraMatrix, distCoeffs);

            vector<int> ids;
            vector<vector<Point2f>> corners;
            aruco::detectMarkers(undistortedFrame, dictionary, corners, ids, parameters);

            if (!ids.empty()) 
            {
                aruco::drawDetectedMarkers(undistortedFrame, corners, ids);

                for (size_t i = 0; i < ids.size(); ++i) 
                {
                    // 计算标记中心像素坐标
                    Point2f center(0, 0);
                    for (const auto& corner : corners[i]) 
                    {
                        center += corner;
                    }
                    center = center / 4;

                    // 转换为实际坐标
                    Point2d actualPos = pixelToActual(center, transformMatrix);

                    // 在地图上显示位置
                    Point2i mapPos(actualPos.x * 100 + 100, actualPos.y * 100 + 100); // 假设缩放比例为100
                    circle(mapCanvas, mapPos, 5, Scalar(0, 255, 0), -1);
                }
            }

            resize(undistortedFrame, undistortedFrame, Size(800, 600));
            imshow("ArUco定位", undistortedFrame);
            resize(mapCanvas, mapCanvas, Size(800, 600));
            imshow("导航地图", mapCanvas);

            if (waitKey(1) == 27) break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } 
    catch (const exception& e) 
    {
        cerr << "系统错误: " << e.what() << endl;
        return -1;
    }
    return 0;
}    
#include "MyQImage.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QPixmap>
#include <algorithm>
#include <QRandomGenerator>
#include <QQueue>
#include <QVector>
#include <map>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <tuple>


using namespace std;

MyQImage::MyQImage() : width(0), height(0), pixels(nullptr) {}

MyQImage::~MyQImage() {
    delete[] pixels;
}

// 拷贝构造函数
MyQImage::MyQImage(const MyQImage& other)
    : width(other.width), height(other.height), rowSize(other.rowSize) {
    // 深拷贝像素数据
    if (other.pixels) {
        pixels = new unsigned char[rowSize * height];
        copy(other.pixels, other.pixels + rowSize * height, pixels);
    } else {
        pixels = nullptr;
    }
}

MyQImage& MyQImage::operator=(const MyQImage& other) {
    // 检查自赋值
    if (this == &other) {
        return *this;
    }
    // 释放当前对象已有的像素数据
    delete[] pixels;

    // 复制其他对象的数据
    width = other.width;
    height = other.height;
    rowSize = other.rowSize;

    if (other.pixels) {
        pixels = new unsigned char[rowSize * height];
        copy(other.pixels, other.pixels + rowSize * height, pixels);
    } else {
        pixels = nullptr;
    }

    // 返回当前对象的引用
    return *this;
}

// 加载 BMP 文件
bool MyQImage::load(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: Cannot open file" << filePath;
        return false;
    }

    // 读取 BMP 文件头 (14字节)
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    qDebug() << "File Header: Type" << QString::number(fileHeader.type, 16)
             << "  Size" << fileHeader.size
             << "  Offset" << fileHeader.offset;
    if (fileHeader.type != 0x4D42) {  // 'BM'
        qDebug() << "Error: Not a valid BMP file.";
        return false;
    }


    // 读取位图信息头 (40字节)
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    qDebug() << "Info Header: Size" << infoHeader.size
             << "Width" << infoHeader.width
             << "Height" << infoHeader.height
             << "Bits per pixel" << infoHeader.bitsPerPixel;

    // 获取图像宽度和高度
    width = infoHeader.width;
    height = infoHeader.height;
    qDebug() << "Bits per pixel:" << infoHeader.bitsPerPixel;

    // 确保图像的颜色深度是 24 位
    if (infoHeader.bitsPerPixel != 24) {
        qDebug() << "Error: Only 24-bit BMP files are supported.";
        return false;
    }

    // 计算每一行的字节数，BMP 行数据通常会对齐到4字节的倍数
    rowSize = (width * 3 + 3) & ~3;

    // 读取像素数据
    int dataSize = rowSize * height;
    pixels = new unsigned char[dataSize];
    file.seek(fileHeader.offset);
    file.read(reinterpret_cast<char*>(pixels), dataSize);

    file.close();
    qDebug() << "Image loaded successfully:" << filePath;
    return true;
}

// 显示图像的 RGB 数据（以调试为主）
void MyQImage::show() const {
    if (!pixels) {
        qDebug() << "Error: No image to display!";
        return;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = &pixels[(y * rowSize) + (x * 3)];
            qDebug() << "(" << (int)pixel[2] << ", " << (int)pixel[1] << ", " << (int)pixel[0] << ")";
        }
        qDebug() << "";
    }
}

// 将图像绘制到指定的 QLabel 上，保持等比例缩放
void MyQImage::drawToLabel(QLabel* label) {
    if (!pixels || !label) {
        qDebug() << "Error: Invalid input, pixels or label is null!";
        return;
    }

    // 获取 QLabel 的大小
    int maxWidth = label->width();
    int maxHeight = label->height();

    // 确保 QLabel 的宽高有效
    if (maxWidth <= 0 || maxHeight <= 0) {
        qDebug() << "Error: Invalid QLabel size, width:" << maxWidth << "height:" << maxHeight;
        return;
    }

    // 计算缩放比例，保持长宽比不变
    float scaleX = (float)maxWidth / width;
    float scaleY = (float)maxHeight / height;
    float scale = qMin(scaleX, scaleY);

    // 计算新的宽高
    int newWidth = width * scale;
    int newHeight = height * scale;

    // 确保新宽高有效
    if (newWidth <= 0 || newHeight <= 0) {
        qDebug() << "Error: Invalid new image size, newWidth:" << newWidth << "newHeight:" << newHeight;
        return;
    }

    // 创建 QPixmap 对象用于绘制
    QPixmap pixmap(newWidth, newHeight);
    pixmap.fill(Qt::white);  // 填充背景色，避免透明背景问题

    // 使用 QPainter 在 QPixmap 上绘制图像
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 计算图像缩放后每个像素的 RGB 值并绘制
    for (int y = 0; y < newHeight; ++y) {
        // 计算源图像的 Y 坐标，反转 Y 坐标，使图像正确显示
        int srcY = (newHeight - 1 - y) * height / newHeight; // 反转 Y 坐标
        for (int x = 0; x < newWidth; ++x) {
            // 计算源图像的 X 坐标，跳过必要的像素列
            int srcX = x * width / newWidth;

            // 确保 srcX 和 srcY 在有效范围内 [0, width-1] 和 [0, height-1]
            if (srcX < 0 || srcX >= width || srcY < 0 || srcY >= height) {
                qDebug() << "Error: srcX or srcY out of bounds, srcX:" << srcX << "srcY:" << srcY;
                continue;
            }

            // 获取原始图像的 RGB 值
            unsigned char* pixel = &pixels[(srcY * rowSize) + (srcX * 3)];
            QColor color(pixel[2], pixel[1], pixel[0]);

            // 在 QPixmap 上绘制每个像素点
            painter.setPen(color);
            painter.drawPoint(x, y);
        }
    }

    // 使用 QPainter 在 QLabel 上绘制 QPixmap
    label->setPixmap(pixmap);
}

void MyQImage::HistogramEqualization(){
    if (!pixels) {
        qDebug() << "Error: No image to process!";
        return;
    }

    //定义三个颜色通道的直方图
    QVector<int> histR(256, 0);
    QVector<int> histG(256, 0);
    QVector<int> histB(256, 0);

    //计算每个颜色通道的直方图
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char* pixel = &pixels[(y * rowSize) + (x * 3)];
            histB[pixel[0]]++;
            histG[pixel[1]]++;
            histR[pixel[2]]++;
        }
    }

    //计算累积分布函数（CDF）
    QVector<float> cdfR(256, 0), cdfG(256, 0), cdfB(256, 0);
    cdfR[0] = histR[0];
    cdfG[0] = histG[0];
    cdfB[0] = histB[0];

    for (int i = 1; i < 256; ++i) {
        cdfR[i] = cdfR[i - 1] + histR[i];
        cdfG[i] = cdfG[i - 1] + histG[i];
        cdfB[i] = cdfB[i - 1] + histB[i];
    }

    //归一化累计直方图
    int pixel_num=height*width;
    for(int i=0;i<256;++i){
        cdfR[i]/=pixel_num;
        cdfG[i]/=pixel_num;
        cdfB[i]/=pixel_num;
    }

    //映射到新像素值
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            unsigned char* pixel=&pixels[(y*rowSize)+(x*3)];
            pixel[0]=cdfB[pixel[0]]*255;
            pixel[1]=cdfG[pixel[1]]*255;
            pixel[2]=cdfR[pixel[2]]*255;
        }
    }
}

bool MyQImage::save(const QString &filePath){
    //打开文件进行写入
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing";
        return false;
    }

    //计算每行的填充字节数，BMP 格式要求每行字节数是 4 的倍数
    int padding = (4 - (width * 3) % 4) % 4; // 每行的填充字节数
    int rowSize = width * 3 + padding;  // 每行的总字节数

    //计算图像的总数据大小
    int imageSize = rowSize * height;

    //写入文件头
    BMPFileHeader fileHeader;
    fileHeader.size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));

    //写入信息头
    BMPInfoHeader infoHeader;
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.imageSize = imageSize;
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    //写入像素数据
    for (int y = 0; y <height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 获取当前像素的 RGB 值
            unsigned char* pixel = &pixels[(y * rowSize) + (x * 3)];
            // 写入 RGB 值
            file.write(reinterpret_cast<const char*>(&pixel[0]), 1);  //Blue
            file.write(reinterpret_cast<const char*>(&pixel[1]), 1);  //Green
            file.write(reinterpret_cast<const char*>(&pixel[2]), 1);  //Red

        }
        //填充字节（每行必须是 4 的倍数）
        for (int i = 0; i < padding; ++i) {
            file.write("\0", 1); // 使用 write 写入填充字节
        }
    }

    file.close();

    return true;
}
/*
void MyQImage::segmentImage() {
    if (!pixels) {
        qDebug() << "No pixel data available for segmentation.";
        return;
    }

    int numPixels = width * height;

    for (int i = 0; i < numPixels; ++i) {
        int row = i / width;
        int col = i % width;
        int index = (row * rowSize + col * 3);
        unsigned char r = pixels[index];
        unsigned char g = pixels[index + 1];
        unsigned char b = pixels[index + 2];

        // 将RGB转换为HSV
        float h, s, v;
        rgbToHSV(r, g, b, h, s, v);

        // 根据HSV值进行分割，这里可以根据H、S、V来定义不同区域
        if (h >= 0 && h < 30) {  // 红色区域
            h = 0; s = 1; v = 1;  // 强化红色
        } else if (h >= 30 && h < 90) {  // 黄色区域
            h = 60; s = 1; v = 1;  // 强化黄色
        } else if (h >= 90 && h < 150) {  // 绿色区域
            h = 120; s = 1; v = 1;  // 强化绿色
        } else if (h >= 150 && h < 210) {  // 蓝色区域
            h = 210; s = 1; v = 1;  // 强化蓝色
        } else if (h >= 210 && h < 270) {  // 紫色区域
            h = 270; s = 1; v = 1;  // 强化紫色
        } else {  // 其他区域
            h = 0; s = 0; v = 0;  // 转为黑色
        }

        // 将修改后的HSV转换回RGB
        hsvToRGB(h, s, v, r, g, b);

        // 将处理后的RGB值更新回像素数据
        pixels[index] = r;
        pixels[index + 1] = g;
        pixels[index + 2] = b;
    }
}*/


void MyQImage::segmentImage() {
    if (!pixels) {
        qDebug() << "No pixel data available for segmentation.";
        return;
    }

    int numPixels = width * height;

    const int maxIterations = 100;  // 最大迭代次数
    float convergenceThreshold = 1.0f;  // 收敛阈值

    //存储K个聚类中心的RGB值
    QVector<std::tuple<int, int, int>> centers(K);

    //存储每个像素的簇标签
    QVector<int> labels(numPixels);

    //初始化K个聚类中心，随机选择像素的RGB值作为初始中心
    for (int i = 0; i < K; ++i) {
        int randomIndex = rand() % (width * height);  // 随机选择一个像素
        int row = randomIndex / width;
        int col = randomIndex % width;
        int index = (row * rowSize + col * 3);
        unsigned char b = pixels[index];
        unsigned char g = pixels[index + 1];
        unsigned char r = pixels[index + 2];

        centers[i] = std::make_tuple(r, g, b);  // 初始化中心 (RGB)
    }

    //K-means算法迭代过程
    bool converged = false;
    int iteration = 0;
    while (!converged && iteration < maxIterations) {
        converged = true;

        //每个像素点分配到最近的中心点
        for (int i = 0; i < numPixels; ++i) {
            int row = i / width;
            int col = i % width;
            int index = (row * rowSize + col * 3);
            unsigned char b = pixels[index];
            unsigned char g = pixels[index + 1];
            unsigned char r = pixels[index + 2];

            //计算该像素点到每个中心点的距离
            int closestCluster = -1;
            float minDistance = std::numeric_limits<float>::max();
            for (int j = 0; j < K; ++j) {
                int centerR, centerG, centerB;
                std::tie(centerR, centerG, centerB) = centers[j];

                //计算当前像素与聚类中心的距离
                float distance = std::sqrt(std::pow(r - centerR, 2) + std::pow(g - centerG, 2) + std::pow(b - centerB, 2));
                if (distance < minDistance) {
                    minDistance = distance;
                    closestCluster = j;
                }
            }

            //如果像素的簇标签发生变化，则继续迭代
            if (labels[i] != closestCluster) {
                labels[i] = closestCluster;
                converged = false;  // 如果有像素的标签发生改变，说明还没有收敛
            }
        }

        //更新每个簇的中心点（计算每个簇内的像素的平均值）
        QVector<int> count(K, 0);
        QVector<long long> sumR(K, 0);
        QVector<long long> sumG(K, 0);
        QVector<long long> sumB(K, 0);

        //计算每个簇的像素总和，并计算每个簇内的像素数量
        for (int i = 0; i < numPixels; ++i) {
            int label = labels[i];
            int row = i / width;
            int col = i % width;
            int index = (row * rowSize + col * 3);
            unsigned char b = pixels[index];
            unsigned char g = pixels[index + 1];
            unsigned char r = pixels[index + 2];

            sumR[label] += r;
            sumG[label] += g;
            sumB[label] += b;
            count[label] += 1;
        }

        //更新聚类中心
        converged = true;
        for (int i = 0; i < K; ++i) {
            if (count[i] > 0) {
                int newR = static_cast<int>(sumR[i] / count[i]);
                int newG = static_cast<int>(sumG[i] / count[i]);
                int newB = static_cast<int>(sumB[i] / count[i]);

                int oldR, oldG, oldB;
                std::tie(oldR, oldG, oldB) = centers[i];

                // 如果中心点发生变化，说明算法没有收敛
                if (std::abs(newR - oldR) > 1 || std::abs(newG - oldG) > 1 || std::abs(newB - oldB) > 1) {
                    converged = false;
                }

                centers[i] = std::make_tuple(newR, newG, newB);  // 更新中心
            }
        }

        ++iteration;
    }

    //根据每个像素的簇标签，更新像素值为其对应的聚类中心颜色
    for (int i = 0; i < numPixels; ++i) {
        int row = i / width;
        int col = i % width;
        int index = (row * rowSize + col * 3);
        int label = labels[i];

        // 将像素值替换为其对应的聚类中心的颜色
        unsigned char r, g, b;
        std::tie(r, g, b) = centers[label];

        // 注意：存储时仍然是BGR顺序
        pixels[index] = b;
        pixels[index + 1] = g;
        pixels[index + 2] = r;
    }
}


// HSV转RGB
void MyQImage::hsvToRGB(float h, float s, float v, unsigned char& r, unsigned char& g, unsigned char& b) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float rTemp, gTemp, bTemp;
    if (h >= 0 && h < 60) {
        rTemp = c; gTemp = x; bTemp = 0;
    } else if (h >= 60 && h < 120) {
        rTemp = x; gTemp = c; bTemp = 0;
    } else if (h >= 120 && h < 180) {
        rTemp = 0; gTemp = c; bTemp = x;
    } else if (h >= 180 && h < 240) {
        rTemp = 0; gTemp = x; bTemp = c;
    } else if (h >= 240 && h < 300) {
        rTemp = x; gTemp = 0; bTemp = c;
    } else {
        rTemp = c; gTemp = 0; bTemp = x;
    }

    r = static_cast<unsigned char>((rTemp + m) * 255);
    g = static_cast<unsigned char>((gTemp + m) * 255);
    b = static_cast<unsigned char>((bTemp + m) * 255);
}

// RGB转HSV
void MyQImage::rgbToHSV(unsigned char r, unsigned char g, unsigned char b, float& h, float& s, float& v) {
    float rNorm = r / 255.0f;
    float gNorm = g / 255.0f;
    float bNorm = b / 255.0f;

    float cMax = max({rNorm, gNorm, bNorm});
    float cMin = min({rNorm, gNorm, bNorm});
    float delta = cMax - cMin;

    // 计算色相 H
    if (delta == 0) {
        h = 0; // undefined, set to 0
    } else if (cMax == rNorm) {
        h = fmod((60.0f * ((gNorm - bNorm) / delta) + 360.0f), 360.0f);
    } else if (cMax == gNorm) {
        h = fmod((60.0f * ((bNorm - rNorm) / delta) + 120.0f), 360.0f);
    } else {
        h = fmod((60.0f * ((rNorm - gNorm) / delta) + 240.0f), 360.0f);
    }

    // 计算饱和度 S
    s = (cMax == 0) ? 0 : (delta / cMax);

    // 计算亮度 V
    v = cMax;
}
/*
void MyQImage::sharpen() {
    int width = getWidth();
    int height = getHeight();
    unsigned char* newPixels = new unsigned char[width * height * 3];  //存储锐化后的图像像素

    int kernel[3][3] = {
        {-1, -1, -1 },
        { -1, 9, -1 },
        { -1, -1, -1 }
    };

    int kernelSize = 3;
    int kernelOffset = kernelSize / 2;  //核的偏移量

    //遍历图像的每个像素
    for (int y = 1; y < height-1; ++y) {
        for (int x = 1; x < width-1; ++x) {
            int r = 0, g = 0, b = 0;

            //对每个像素应用卷积核
            for (int ky = -kernelOffset; ky <= kernelOffset; ++ky) {
                for (int kx = -kernelOffset; kx <= kernelOffset; ++kx) {
                    int pixelX = x + kx;
                    int pixelY = y + ky;

                    //确保不超出图像边界
                    if (pixelX >= 0 && pixelX < width && pixelY >= 0 && pixelY < height) {
                        //获取当前邻域像素的RGB值
                        int idx = (pixelY * width + pixelX)* 3;
                        unsigned char pixelB = pixels[idx];
                        unsigned char pixelG = pixels[idx + 1];
                        unsigned char pixelR = pixels[idx + 2];

                        //对每个颜色通道进行加权（卷积）
                        r += pixelR * kernel[ky + kernelOffset][kx + kernelOffset];
                        g += pixelG * kernel[ky + kernelOffset][kx + kernelOffset];
                        b += pixelB * kernel[ky + kernelOffset][kx + kernelOffset];
                    }
                    //边界像素时，使用0填充（可以改为使用原像素值）
                    else {
                        r += 0 * kernel[ky + kernelOffset][kx + kernelOffset];
                        g += 0 * kernel[ky + kernelOffset][kx + kernelOffset];
                        b += 0 * kernel[ky + kernelOffset][kx + kernelOffset];
                    }
                }
            }

            //限制像素值范围为 0-255
            r = min(255, std::max(0, r));
            g = min(255, std::max(0, g));
            b = min(255, std::max(0, b));

            //更新锐化后的像素
            int idx = (y * width + x) * 3;
            newPixels[idx] = b;
            newPixels[idx + 1] = g;
            newPixels[idx + 2] = r;
        }
    }

    //替换旧像素数据
    delete[] pixels;
    pixels = newPixels;
}*/

void MyQImage::sharpen() {
    if (!pixels || width <= 0 || height <= 0) {
        return; // 如果像素数据为空或图像无效，直接返回
    }

    // 创建一个临时数组，用于存储锐化后的像素数据
    unsigned char* sharpenedPixels = new unsigned char[width * height * 3];

    // 定义拉普拉斯算子的卷积核（用于增强边缘）
    int kernel[3][3] = {
        { 0, -1,  0 },
        { -1,  5, -1 },
        { 0, -1,  0 }
    };

    // 遍历每个像素
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int r = 0, g = 0, b = 0;

            // 应用拉普拉斯算子
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = (x + kx);
                    int py = (y + ky);

                    // 获取当前像素的 RGB 值
                    unsigned char* currentPixel = pixels + (py * rowSize + px * 3);
                    unsigned char currentR = currentPixel[2];
                    unsigned char currentG = currentPixel[1];
                    unsigned char currentB = currentPixel[0];

                    // 卷积操作
                    r += currentR * kernel[ky + 1][kx + 1];
                    g += currentG * kernel[ky + 1][kx + 1];
                    b += currentB * kernel[ky + 1][kx + 1];
                }
            }

            // 将拉普拉斯结果叠加到原图上（锐化公式）
            int newR = r;
            int newG = g;
            int newB = b;


            // 限制 RGB 值范围到 [0, 255]
            newR = std::min(255, std::max(0, newR));
            newG = std::min(255, std::max(0, newG));
            newB = std::min(255, std::max(0, newB));

            // 保存锐化后的像素值
            unsigned char* sharpenedPixel = sharpenedPixels + (y * rowSize + x * 3);
            sharpenedPixel[2] = static_cast<unsigned char>(newR);
            sharpenedPixel[1] = static_cast<unsigned char>(newG);
            sharpenedPixel[0] = static_cast<unsigned char>(newB);
        }
    }

    // 替换原有像素数据
    delete[] pixels;
    pixels = sharpenedPixels;


}





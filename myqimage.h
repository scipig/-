#ifndef MYQIMAGE_H
#define MYQIMAGE_H

#include <QString>
#include <QSize>
#include <QLabel>
#include <QPainter>


class MyQImage {
public:
    MyQImage();
    MyQImage(const MyQImage& other);
    ~MyQImage();
    //重载=
    MyQImage& operator=(const MyQImage& other);

    // 加载 BMP 文件
    bool load(const QString& filePath);

    // 获取图像宽度
    int getWidth() const { return width; }

    // 获取图像高度
    int getHeight() const { return height; }

    // 获取图像的尺寸
    QSize getSize() const { return QSize(width, height); }

    // 获取像素数据
    const unsigned char* getPixels() const { return pixels; }

    // 显示图像的 RGB 数据
    void show() const;

    // 将图像绘制到指定的 QLabel 上，保持等比例缩放
    void drawToLabel(QLabel* label);

    // 直方图均衡化
    void HistogramEqualization();

    //保存图像
    bool save(const QString& filePath);

    // 图像分割
    void segmentImage();

    //锐化
    void sharpen();

    // 将RGB像素转换为灰度值（采用常见的加权平均法）
    int rgbToGray(unsigned char r, unsigned char g, unsigned char b) {
        return static_cast<int>(0.299 * r + 0.587 * g + 0.114 * b);
    }

    void changeK(int value){
        this->K=value;
    }

private:
    // BMP 文件头
    #pragma pack(1)
    struct BMPFileHeader {
        uint16_t type = 0x4D42;  // 'BM' 2字节
        uint32_t size = 0;//4字节
        uint16_t reserved1 = 0;//2字节
        uint16_t reserved2 = 0;//2字节
        uint32_t offset = 54;  // 54 字节偏移，跳过文件头和信息头，4字节
    } fileHeader;

    // BMP 信息头
    struct BMPInfoHeader {
        uint32_t size = 40;  // 头部大小 4字节
        int32_t width = 0;   // 图像宽度 4字节
        int32_t height = 0;  // 图像高度 4字节
        uint16_t planes = 1; // 目标设备的平面数，始终为1 2字节
        uint16_t bitsPerPixel = 24; // 24位图像 2字节
        uint32_t compression = 0; // 无压缩 4字节
        uint32_t imageSize = 0;//数据大小 4字节
        int32_t xResolution = 0;//水平分辨率 4字节
        int32_t yResolution = 0;//垂直分辨率 4字节
        uint32_t colors = 0;//颜色数 4字节
        uint32_t importantColors = 0;//重要颜色数 4字节
    } infoHeader;

    struct RGB {
        unsigned char r, g, b;
    };

    int width, height;// 图像的宽和高
    unsigned char* pixels;// 像素数据
    int rowSize;// 每行的字节数
    int K=1;//用于图像分割中的k-means算法


    void hsvToRGB(float h, float s, float v, unsigned char& r, unsigned char& g, unsigned char& b);
    void rgbToHSV(unsigned char r, unsigned char g, unsigned char b, float& h, float& s, float& v);

};

#endif // MYQIMAGE_H

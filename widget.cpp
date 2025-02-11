#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_Image_Choose_Button_clicked()
{
    // 打开文件对话框，并让用户选择文件
    QString filePath = QFileDialog::getOpenFileName(this, "Select File", "", "All Files (*.*);;Text Files (*.txt)");

    // 如果用户选择了文件，则将文件路径设置到QLineEdit中
    if (!filePath.isEmpty()) {
        ui->image_path->setText(filePath);
        if(!image.load(filePath)){
            ui->image_path->setText("图像加载失败！");
        }
        else{
            image.drawToLabel(ui->Image_show);
        }

    }
    else{
        ui->image_path->setText("文件路径读取失败！");
    }
    origin_image=image;
}

void Widget::on_origin_image_clicked()
{
    origin_image.drawToLabel(ui->Image_show);
    image=origin_image;
}

void Widget::on_image_enhancement_clicked()
{
    //image=origin_image;
    image.HistogramEqualization();
    enhanced_image=image;
    enhanced_image.drawToLabel(ui->Image_show);
}

void Widget::on_save_image_clicked()
{
    // 打开文件对话框，选择文件路径
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Image"),
        "",
        tr("BMP Files (*.bmp);;All Files (*)")
    );

    // 确保用户选择了一个有效的文件路径
    if (!filePath.isEmpty()) {
        // 使用 MyQImage 的 saveImage 函数保存图像
        bool success = image.save(filePath);
        if (success) {
            QMessageBox::information(this, tr("Save Image"), tr("Image saved successfully!"));
        } else {
            QMessageBox::warning(this, tr("Save Image"), tr("Failed to save the image."));
        }
    }
}

void Widget::on_pushButton_clicked()
{
    image.segmentImage();
    segmented_image=image;
    segmented_image.drawToLabel(ui->Image_show);
}

void Widget::on_sharpen_clicked()
{
    image.sharpen();
    image.drawToLabel(ui->Image_show);
}


void Widget::on_spinBox_valueChanged(int arg1)
{
    image.changeK(arg1);
}

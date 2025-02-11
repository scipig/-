#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "myqimage.h"
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    MyQImage image;
    MyQImage origin_image;
    MyQImage enhanced_image;
    MyQImage segmented_image;

private slots:
    void on_Image_Choose_Button_clicked();

    void on_origin_image_clicked();

    void on_image_enhancement_clicked();

    void on_save_image_clicked();

    void on_pushButton_clicked();

    void on_sharpen_clicked();

    void on_spinBox_valueChanged(int arg1);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H

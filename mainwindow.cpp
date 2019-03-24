#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/openglwidget.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QMessageBox>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    processor = new ImageProcessor();
    currentColor = QVector3D(0.0,1.0,0.7);
    currentAmbientcolor = QVector3D(1.0,1.0,1.0);

    image = QImage(":/images/sample.png");

    QPixmap pixmap(100,100);
    pixmap.fill(QColor(currentColor.x()*255,currentColor.y()*255,currentColor.z()*255));
    ui->pushButtonColor->setIcon(QIcon(pixmap));
    pixmap.fill(QColor(currentAmbientcolor.x()*255,currentAmbientcolor.y()*255,currentAmbientcolor.z()*255));
    ui->pushButtonAmbientColor->setIcon(QIcon(pixmap));
    m_raw_scene = new QGraphicsScene(this);

    ui->dockWidget->setFeatures(QDockWidget::DockWidgetMovable);
    ui->dockWidget_2->setFeatures(QDockWidget::DockWidgetMovable);

    connect_processor(processor);


    //    processingThread = new QThread();
    //    processor->moveToThread(processingThread);
    //    processingThread->start();




}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_scene(QImage image, ProcessedImage type){
    QPixmap pixmap = QPixmap::fromImage(image);
    switch (type) {

    case ProcessedImage::Raw:
        ui->openGLPreviewWidget->setImage(image);
        break;
    case ProcessedImage::Normal:
        normal = image;
        ui->openGLPreviewWidget->setNormalMap(image);
        if (ui->radioButtonNormal->isChecked())
            ui->openGLPreviewWidget->setImage(image);
        break;

    }
    ui->openGLPreviewWidget->update();
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Abrir Imagen"), "",
                                                    tr("Archivos de Imagen (*.png *.jpg *.bmp)"));

    if (fileName != nullptr){
        image = QImage(fileName);
        for(int i = 0; i < ui->listWidget->count(); i++){
            if (ui->listWidget->item(i)->text() == fileName){
                QMessageBox msgBox;
                msgBox.setText(tr("La imagen ya se encuentra abierta en laigter."));
                msgBox.exec();
                return;
            }
        }
        ImageProcessor *p = new ImageProcessor();
        processorList.append(p);
        p->copy_settings(processor);
        disconnect_processor(processor);
        processor = p;
        connect_processor(processor);
        processor->loadImage(fileName);
        if (ui->radioButtonRaw->isChecked()){
            on_radioButtonRaw_toggled(true);
        } else if (ui->radioButtonNormal->isChecked()){
            on_radioButtonNormal_toggled(true);
        } else {
            on_radioButtonPreview_toggled(true);
        }
        ui->listWidget->addItem(new QListWidgetItem(QIcon(fileName),processor->get_name()));
    }
}

void MainWindow::on_actionFitZoom_triggered()
{
    ui->openGLPreviewWidget->fitZoom();

}

void MainWindow::on_actionZoom_100_triggered()
{
    ui->openGLPreviewWidget->resetZoom();
}

void MainWindow::on_actionZoomIn_triggered()
{
    ui->openGLPreviewWidget->setZoom(1.1*ui->openGLPreviewWidget->getZoom());
}

void MainWindow::on_actionZoomOut_triggered()
{
    ui->openGLPreviewWidget->setZoom(0.9*ui->openGLPreviewWidget->getZoom());
}

void MainWindow::on_actionExport_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Guardar Imagen"), "",
                                                    tr("Archivos de Imagen (*.png *.jpg *.bmp)"));

    if (fileName != nullptr){
        normal.save(fileName);
    }

}

void MainWindow::on_radioButtonRaw_toggled(bool checked)
{
    if (checked){
        ui->openGLPreviewWidget->setLight(false);
        if (!image.isNull())
            update_scene(image,ProcessedImage::Raw);
    }
}

void MainWindow::on_radioButtonNormal_toggled(bool checked)
{
    if (checked){
        ui->openGLPreviewWidget->setLight(false);
        if (!normal.isNull())
            update_scene(normal,ProcessedImage::Raw);
    }
}

void MainWindow::on_radioButtonPreview_toggled(bool checked)
{
    if (checked){
        ui->openGLPreviewWidget->setLight(true);
        if (!image.isNull())
            update_scene(image,ProcessedImage::Raw);
    }
}

void MainWindow::openGL_initialized(){
    ui->radioButtonRaw->toggle();
    QString tmpImage = QDir::temp().path()+"/sample.png";
    qDebug() << image.save(tmpImage);
    processor->loadImage(tmpImage);
}

void MainWindow::on_pushButtonColor_clicked()
{
    QColor color = QColorDialog::getColor(QColor(currentColor.x()*255,currentColor.y()*255,currentColor.z()*255));
    if (color.isValid()){
        currentColor = QVector3D(color.redF(),color.greenF(),color.blueF());
        QPixmap pixmap(100,100);
        pixmap.fill(color);
        ui->pushButtonColor->setIcon(QIcon(pixmap));
        ui->openGLPreviewWidget->setLightColor(currentColor);
    }
}

void MainWindow::on_horizontalSliderDiffHeight_valueChanged(int value)
{
    ui->openGLPreviewWidget->setLightHeight(value/100.0);
}

void MainWindow::on_horizontalSliderDiffLight_valueChanged(int value)
{
    ui->openGLPreviewWidget->setLightIntensity(value/100.0);
}

void MainWindow::on_horizontalSliderAmbientLight_valueChanged(int value)
{
    ui->openGLPreviewWidget->setAmbientIntensity(value/100.0);
}


void MainWindow::on_pushButtonAmbientColor_clicked()
{
    QColor color = QColorDialog::getColor(QColor(currentAmbientcolor.x()*255,currentAmbientcolor.y()*255,currentAmbientcolor.z()*255));
    if (color.isValid()){
        currentAmbientcolor = QVector3D(color.redF(),color.greenF(),color.blueF());
        QPixmap pixmap(100,100);
        pixmap.fill(color);
        ui->pushButtonAmbientColor->setIcon(QIcon(pixmap));
        ui->openGLPreviewWidget->setAmbientColor(currentAmbientcolor);
    }
}

void MainWindow::on_actionLicencia_triggered()
{
    QDesktopServices::openUrl(QUrl("license.html"));
}

void MainWindow::on_actionReconocimientos_triggered()
{
    QDesktopServices::openUrl(QUrl("acknowledgements.html"));
}

void MainWindow::connect_processor(ImageProcessor *p){
    connect(p,SIGNAL(processed(QImage, ProcessedImage)),this,SLOT(update_scene(QImage, ProcessedImage)));
    connect(ui->normalDepthSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_depth(int)));
    connect(ui->normalBlurSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_blur_radius(int)));
    connect(ui->normalBevelSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_bisel_depth(int)));
    connect(ui->normalBiselDistanceSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_bisel_distance(int)));
    connect(ui->normalBiselBlurSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_bisel_blur_radius(int)));
    connect(ui->biselSoftRadio,SIGNAL(toggled(bool)),p,SLOT(set_normal_bisel_soft(bool)));
    connect(ui->normalInvertX,SIGNAL(toggled(bool)),p,SLOT(set_normal_invert_x(bool)));
    connect(ui->normalInvertY,SIGNAL(toggled(bool)),p,SLOT(set_normal_invert_y(bool)));
    connect(ui->openGLPreviewWidget,SIGNAL(initialized()),this,SLOT(openGL_initialized()));
}

void MainWindow::disconnect_processor(ImageProcessor *p){
    disconnect(p,SIGNAL(processed(QImage, ProcessedImage)),this,SLOT(update_scene(QImage, ProcessedImage)));
    disconnect(ui->normalDepthSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_depth(int)));
    disconnect(ui->normalBlurSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_blur_radius(int)));
    disconnect(ui->normalBevelSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_bisel_depth(int)));
    disconnect(ui->normalBiselDistanceSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_bisel_distance(int)));
    disconnect(ui->normalBiselBlurSlider,SIGNAL(valueChanged(int)),p,SLOT(set_normal_bisel_blur_radius(int)));
    disconnect(ui->biselSoftRadio,SIGNAL(toggled(bool)),p,SLOT(set_normal_bisel_soft(bool)));
    disconnect(ui->normalInvertX,SIGNAL(toggled(bool)),p,SLOT(set_normal_invert_x(bool)));
    disconnect(ui->normalInvertY,SIGNAL(toggled(bool)),p,SLOT(set_normal_invert_y(bool)));
    disconnect(ui->openGLPreviewWidget,SIGNAL(initialized()),this,SLOT(openGL_initialized()));
}

void MainWindow::on_listWidget_itemChanged(QListWidgetItem *item)
{

}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    for(int i = 0; i < processorList.count(); i++){
        if (processorList.at(i)->get_name() == item->text()){

            disconnect_processor(processor);
            processor = processorList.at(i);
            connect_processor(processor);

            ui->normalInvertX->setChecked(processor->get_normal_invert_x() == -1);
            ui->normalInvertY->setChecked(processor->get_normal_invert_y() == -1);
            ui->biselSoftRadio->setChecked(processor->get_normal_bisel_soft());
            ui->normalBlurSlider->setValue(processor->get_normal_blur_radius());
            ui->normalBevelSlider->setValue(processor->get_normal_bisel_depth());
            ui->normalDepthSlider->setValue(processor->get_normal_depth());
            ui->normalBiselBlurSlider->setValue(processor->get_normal_bisel_blur_radius());
            ui->normalBiselDistanceSlider->setValue(processor->get_normal_bisel_distance());
            image = QImage(processor->get_name());

            processor->update();
            if (ui->radioButtonRaw->isChecked()){
                on_radioButtonRaw_toggled(true);
            } else if (ui->radioButtonNormal->isChecked()){
                on_radioButtonNormal_toggled(true);
            } else {
                on_radioButtonPreview_toggled(true);
            }

        }
    }

}

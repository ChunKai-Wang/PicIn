#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog_invalid_path.h"
#include <QApplication>

#include <QFileDialog>
#include <QDir>

#include <QString>
#include <QStringList>

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connect
    connect(ui->button_quit, SIGNAL(clicked(bool)), this, SLOT(slot_button_quit_clicked()));
    connect(ui->button_browse_source, SIGNAL(clicked(bool)), this, SLOT(slot_button_browse_source_clicked()));
    connect(ui->button_browse_target, SIGNAL(clicked(bool)), this, SLOT(slot_button_browse_target_clicked()));
    connect(ui->button_import, SIGNAL(clicked(bool)), this, SLOT(slot_button_import_clicked()));
    connect(this, SIGNAL(signal_show_dialog(QString)), this, SLOT(slot_show_dialog(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


//
// functions
//

//name : get_file_list
//desc : Scan indicated path and return file info list of images
QFileInfoList get_file_list(QString path, QStringList exNameFilters)
{
    QDir dir(path);
    QStringList nameFilters;

    //Scan pictures
    QFileInfoList fileInfoList;

    if(exNameFilters.size() > 0){
        nameFilters.append(exNameFilters);
    }
    dir.setFilter(QDir::NoSymLinks | QDir::Files);
    dir.setNameFilters(nameFilters);
    dir.setSorting(QDir::Name);

    fileInfoList = dir.entryInfoList();

    //Scan dir
    QFileInfoList dirInfoList;

    dir.setFilter(QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);
    nameFilters.clear();
    dir.setNameFilters(nameFilters);
    dir.setSorting(QDir::Name);

    dirInfoList = dir.entryInfoList();

    for(int i = 0; i < dirInfoList.size(); i++){
        fileInfoList.append(get_file_list(dirInfoList.at(i).absoluteFilePath(), exNameFilters));
    }

    return fileInfoList;
}

//name : copy_file_as_list
//desc : Copy files to specific location as list
void copy_files_as_list(QFileInfoList fileInfoList, QString path_target)
{
    QFile file;
    QString fileName;
    QString filePath;

    for(int i = 0; i < fileInfoList.size(); i++){
        fileName.clear();
        fileName.append(fileInfoList.at(i).fileName());

        filePath.clear();
        filePath.append(path_target);
        filePath.append(fileName);
        file.copy(fileInfoList.at(i).absoluteFilePath(), filePath);
    }
}

//
// slots
//
void MainWindow::slot_button_browse_source_clicked(void)
{
    QFileDialog fileDialog;
    QString dirPath;

    dirPath = fileDialog.getExistingDirectory(
                this,
                tr("choose directory"),
                "/home/",
                QFileDialog::ShowDirsOnly |
                QFileDialog::DontResolveSymlinks |
                QFileDialog::DontUseCustomDirectoryIcons);

    if(!dirPath.isNull()){
        ui->lineEdit_path_source->setText(dirPath);
    }
}

void MainWindow::slot_button_browse_target_clicked(void)
{
    QFileDialog fileDialog;
    QString dirPath;

    dirPath = fileDialog.getExistingDirectory(
                this,
                tr("choose directory"),
                "/home/",
                QFileDialog::ShowDirsOnly |
                QFileDialog::DontResolveSymlinks |
                QFileDialog::DontUseCustomDirectoryIcons);

    if(!dirPath.isNull()){
        ui->lineEdit_path_target->setText(dirPath);
    }
}


void MainWindow::slot_button_import_clicked(void)
{
    QFileInfoList retFileInfoList;
    QDir dir_s(ui->lineEdit_path_source->text());
    QDir dir_t(ui->lineEdit_path_target->text());

    if(!dir_s.exists() || !ui->lineEdit_path_source->text().compare(tr(""), Qt::CaseInsensitive)){
        emit signal_show_dialog(tr("Invalid source path"));
        return;
    }

    if(!dir_t.exists() || !ui->lineEdit_path_target->text().compare(tr(""), Qt::CaseInsensitive)){
        emit signal_show_dialog(tr("Invalid target path"));
        return;
    }


    //
    // Get file list
    //
    QStringList nameFilters;

    nameFilters.append(tr("*.jpg"));
    retFileInfoList = get_file_list(ui->lineEdit_path_source->text(), nameFilters);
    if(retFileInfoList.size() <= 0){
        emit signal_show_dialog("No Pictures");
    }

    //
    // Import
    //

    copy_files_as_list(retFileInfoList, ui->lineEdit_path_target->text());
}

void MainWindow::slot_show_dialog(QString infoToShow)
{
    dialog_invalid_path *dialogInvalidPathUi = new dialog_invalid_path(0);
    dialogInvalidPathUi->setParent(0);
    dialogInvalidPathUi->set_label_text(infoToShow);
    dialogInvalidPathUi->show();
}

void MainWindow::slot_button_quit_clicked(void)
{
    QApplication::instance()->quit();
}

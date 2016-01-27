#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog_invalid_path.h"
#include "ui_dialog_invalid_path.h"
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
    connect(this, SIGNAL(signal_enalbe_window()), this, SLOT(slot_enable_window()));
    connect(this, SIGNAL(signal_disable_window()), this, SLOT(slot_disable_window()));
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
QFileInfoList MainWindow::get_file_list(
        QString path,
        QStringList exNameFilters
)
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
        fileInfoList.append(
                        get_file_list(dirInfoList.at(i).absoluteFilePath(),
                        exNameFilters));
    }

    return fileInfoList;
}

//name : copy_file_as_list
//desc : Copy files to specific location as list
void MainWindow::copy_files_as_list(
        QFileInfoList fileInfoList,
        QString path_target
)
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
        if(!file.exists(filePath)){
            file.copy(fileInfoList.at(i).absoluteFilePath(), filePath);
        }
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
    //
    // Check empty path
    //

    if(!ui->lineEdit_path_source->text().compare(tr(""), Qt::CaseInsensitive)){
        emit signal_show_dialog(tr("Please input source path"));
        return;
    }

    if(!ui->lineEdit_path_target->text().compare(tr(""), Qt::CaseInsensitive)){
        emit signal_show_dialog(tr("Please input target path"));
        return;
    }

    //
    // Check whether path has forward slash at last char
    //

    QSysInfo sysInfo;
    QString osVer = sysInfo.kernelType();

    QString path_source = ui->lineEdit_path_source->text();
    QString path_target = ui->lineEdit_path_target->text();

    QChar lastChar_s = path_source.at(path_source.size() - 1);
    QChar lastChar_t = path_target.at(path_target.size() - 1);
    QChar fSlash = QChar('/');

    if(!osVer.compare("linux", Qt::CaseInsensitive)){
        if(!operator ==(lastChar_s, fSlash)){
            path_source.append("/");
        }
        if(!operator ==(lastChar_t, fSlash)){
            path_target.append("/");
        }
    }

    //
    // Check path
    //

    QDir dir_s(path_source);
    QDir dir_t(path_target);

    if(!dir_s.exists()){
        emit signal_show_dialog(tr("Invalid source path"));
        return;
    }

    if(!dir_t.exists()){
        emit signal_show_dialog(tr("Invalid target path"));
        return;
    }


    //
    // Get file list
    //

    QFileInfoList retFileInfoList;
    QStringList nameFilters;

    nameFilters.append(tr("*.jpg"));
    retFileInfoList = MainWindow::get_file_list(path_source, nameFilters);
    if(retFileInfoList.size() <= 0){
        emit signal_show_dialog("No Pictures");
    }

    //
    // Import
    //

    MainWindow::copy_files_as_list(retFileInfoList, path_target);
}

void MainWindow::slot_show_dialog(QString infoToShow)
{
    dialog_invalid_path *dialogInvalidPathUi = new dialog_invalid_path(0);
    dialogInvalidPathUi->setParent(0);
    dialogInvalidPathUi->set_label_text(infoToShow);
    dialogInvalidPathUi->show();
    emit signal_disable_window();
    dialogInvalidPathUi->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialogInvalidPathUi->ui->verticalLayout, SIGNAL(destroyed(QObject*)), this, SLOT(slot_enable_window()));
}

void MainWindow::slot_button_quit_clicked(void)
{
    QApplication::instance()->quit();
}

void MainWindow::slot_enable_window(void)
{
    MainWindow::setEnabled(true);
}

void MainWindow::slot_disable_window(void)
{
    MainWindow::setEnabled(false);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog_invalid_path.h"
#include "dialogimportchecker.h"
#include "ui_dialogimportchecker.h"
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

    m_path_source = ui->lineEdit_path_source->text();
    m_path_target = ui->lineEdit_path_target->text();

    QChar lastChar_s = m_path_source.at(m_path_source.size() - 1);
    QChar lastChar_t = m_path_target.at(m_path_target.size() - 1);
    QChar fSlash = QChar('/');

    if(!osVer.compare("linux", Qt::CaseInsensitive)){
        if(!operator ==(lastChar_s, fSlash)){
            m_path_source.append("/");
        }
        if(!operator ==(lastChar_t, fSlash)){
            m_path_target.append("/");
        }
    }

    //
    // Check path
    //

    QDir dir_s(m_path_source);
    QDir dir_t(m_path_target);

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

    QStringList nameFilters;

    nameFilters.append(tr("*.jpg"));
    m_fileInfoList_img.clear();
    m_fileInfoList_img = MainWindow::get_file_list(m_path_source, nameFilters);
    if(m_fileInfoList_img.size() <= 0){
        emit signal_show_dialog("No Pictures");
    }

    //
    // Check
    //

    QString fileNum;
    fileNum.sprintf("%d files will imported", m_fileInfoList_img.size());
    MainWindow::slot_show_import_checker(fileNum);
}

void MainWindow::slot_show_dialog(QString infoToShow)
{
    dialog_invalid_path *dialogInvalidPathUi = new dialog_invalid_path(0);

    //Show dialog
    dialogInvalidPathUi->setParent(0);
    dialogInvalidPathUi->set_label_text(infoToShow);
    dialogInvalidPathUi->show();

    //Disable main window
    emit signal_disable_window();

    //Enable main window once dialog been closed
    dialogInvalidPathUi->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialogInvalidPathUi, SIGNAL(destroyed(QObject*)), this, SLOT(slot_enable_window()));
}

//name : slot_show_import_checker
//desc : show DialogImportChecker
void MainWindow::slot_show_import_checker(QString infoToShow){

    emit signal_disable_window();

    DialogImportChecker *DialogImportCheckerUi = new DialogImportChecker;
    DialogImportCheckerUi->setParent(0);
    DialogImportCheckerUi->ui->label_sts->setText(infoToShow);
    DialogImportCheckerUi->setWindowTitle(tr(" "));
    DialogImportCheckerUi->show();

    DialogImportCheckerUi->setAttribute(Qt::WA_DeleteOnClose);
    connect(DialogImportCheckerUi, SIGNAL(destroyed(QObject*)), this, SLOT(slot_enable_window()));
    connect(DialogImportCheckerUi, SIGNAL(accepted()), this, SLOT(slot_import()));
}

//name : slot_import
//desc : Copy files to specific location as list
void MainWindow::slot_import()
{
    QFile file;
    QString fileName;
    QString filePath;

    emit this->signal_disable_window();

    for(int i = 0; i < this->m_fileInfoList_img.size(); i++){
        fileName.clear();
        fileName.append(this->m_fileInfoList_img.at(i).fileName());

        filePath.clear();
        filePath.append(m_path_target);
        filePath.append(fileName);
        if(!file.exists(filePath)){
            file.copy(this->m_fileInfoList_img.at(i).absoluteFilePath(), filePath);
        }
    }

    emit this->signal_enalbe_window();
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

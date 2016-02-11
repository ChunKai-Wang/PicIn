#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("PicIn "PICIN_VER));

    //Initialize radio buttons
    ui->radioBtn_dirAsY->setEnabled(false);
    ui->radioBtn_dirAsYM->setEnabled(false);
    ui->radioBtn_dirAsYMD->setEnabled(false);

#ifdef Q_OS_WIN
    ui->radioBtn_dirAsY->setText(tr("Year\\"));
    ui->radioBtn_dirAsYM->setText(tr("Year\\Month\\"));
    ui->radioBtn_dirAsYMD->setText(tr("Year\\Month\\Day\\"));
#endif

    //Initialize variables
    m_picInCore = new PicIn_Core;
    m_flagImportCancel = false;

    //connect
    connect(ui->button_quit, SIGNAL(clicked(bool)), this, SLOT(slot_button_quit_clicked()));
    connect(ui->button_browse_source, SIGNAL(clicked(bool)), this, SLOT(slot_button_browse_source_clicked()));
    connect(ui->button_browse_target, SIGNAL(clicked(bool)), this, SLOT(slot_button_browse_target_clicked()));
    connect(ui->button_import, SIGNAL(clicked(bool)), this, SLOT(slot_button_import_clicked()));
    connect(this, SIGNAL(signal_show_dialog(QString)), this, SLOT(slot_show_dialog(QString)));
    connect(ui->checkBox_dirAsDate, SIGNAL(clicked(bool)), this, SLOT(slot_checkbox_dirAsDate_clicked()));
    connect(ui->checkBox_fileFmt_all, SIGNAL(clicked(bool)), this, SLOT(slot_fileFmt_all(bool)));

    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(slot_menu_about()));
    connect(ui->actionAbout_QT, SIGNAL(triggered(bool)), this, SLOT(slot_menu_aboutQt()));
    connect(ui->actionSource, SIGNAL(triggered(bool)), this, SLOT(slot_button_browse_source_clicked()));
    connect(ui->actionTarget, SIGNAL(triggered(bool)), this, SLOT(slot_button_browse_target_clicked()));
    connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(slot_button_import_clicked()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(slot_button_quit_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ****************************************************************************
// ****                         Public Slots:                              ****
// ****************************************************************************

/*
 * name : slot_import
 * desc : Copy files to specific location as list
 */
void MainWindow::slot_import()
{
    //
    // Prepare progress bar
    //

    Dialog_Import_Progress *dialogImportProgress = new Dialog_Import_Progress(0);
    dialogImportProgress->setParent(0);
    dialogImportProgress->setAttribute(Qt::WA_DeleteOnClose);
    dialogImportProgress->setModal(true);
    dialogImportProgress->setWindowTitle(tr(" "));
    dialogImportProgress->ui->progressBar->setRange(0, m_picInCore->getNumFilesSrc());
    dialogImportProgress->ui->progressBar->setValue(0);
    dialogImportProgress->ui->progressBar->setFormat("%v/%m");
    dialogImportProgress->show();

    connect(dialogImportProgress, SIGNAL(rejected()), this, SLOT(slot_import_canceled()));
    connect(m_picInCore, SIGNAL(signal_update_progress(int)), dialogImportProgress->ui->progressBar, SLOT(setValue(int)));

    //
    // Set import folder as date flags
    //

    if(ui->checkBox_dirAsDate->isChecked()){
        bool dirAsY = false;
        bool dirAsM = false;
        bool dirAsD = false;
        if(ui->radioBtn_dirAsY->isChecked()){
            dirAsY = true;
        }
        else if(ui->radioBtn_dirAsYM->isChecked()){
            dirAsY = true;
            dirAsM = true;
        }
        else if(ui->radioBtn_dirAsYMD->isChecked()){
            dirAsY = true;
            dirAsM = true;
            dirAsD = true;
        }
        m_picInCore->setFlagDir(dirAsY, dirAsM, dirAsD);
    }
    else{
        m_picInCore->setFlagDir(false, false, false);
    }

    //
    // Do import
    //

    m_picInCore->import_doit();

    //
    // done.
    //

    if(!m_flagImportCancel){
        dialogImportProgress->close();
    }
    m_flagImportCancel = false;
}

/*
 * name : slot_import_canceled
 * desc : Cancel button be clicked during importing
 */
void MainWindow::slot_import_canceled(void)
{
    m_picInCore->setFlagCancel_true();
    m_flagImportCancel = true;
}

// ****************************************************************************
// ****                        Private Slots:                              ****
// ****************************************************************************

/*
 * name : slot_button_browse_source_clicked
 * desc : Clicked browse button for source path
 */
void MainWindow::slot_button_browse_source_clicked(void)
{
    QFileDialog fileDialog;
    QString dirPath;
    QDir dir;

    dirPath = fileDialog.getExistingDirectory(
                  this,
                  tr("choose directory"),
                  "",
                  QFileDialog::ShowDirsOnly |
                  QFileDialog::DontResolveSymlinks |
                  QFileDialog::DontUseCustomDirectoryIcons);

    dirPath = dir.toNativeSeparators(dirPath);
    if(!dirPath.isNull()){
        ui->lineEdit_path_source->setText(dirPath);
    }
}

/*
 * name : slot_button_browse_source_clicked
 * desc : Clicked browse button for target path
 */
void MainWindow::slot_button_browse_target_clicked(void)
{
    QFileDialog fileDialog;
    QString dirPath;
    QDir dir;

    dirPath = fileDialog.getExistingDirectory(
                  this,
                  tr("choose directory"),
                  "",
                  QFileDialog::ShowDirsOnly |
                  QFileDialog::DontResolveSymlinks |
                  QFileDialog::DontUseCustomDirectoryIcons);

    dirPath = dir.toNativeSeparators(dirPath);
    if(!dirPath.isNull()){
        ui->lineEdit_path_target->setText(dirPath);
    }
}

/*
 * name : slot_button_browse_source_clicked
 * desc : Clicked import button, check path then scan files then do importing
 */
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
    // Check whether path is exist
    //

    if(m_picInCore->set_path(
         ui->lineEdit_path_source->text(),
         PicIn_Core::PT_Source) != 0)
    {
        emit signal_show_dialog("Source path isn't' exist");
        return;
    }

    if(m_picInCore->set_path(
         ui->lineEdit_path_target->text(),
         PicIn_Core::PT_Target) != 0)
    {
        emit signal_show_dialog("Target path isn't' exist");
        return;
    }

    //
    // Set file name filter
    //

    QStringList nameFilters;
    int numPic = 0;

    if(ui->checkBox_fileFmt_jpg->isChecked()){
        nameFilters.append(tr("*.jpg"));
        nameFilters.append(tr("*.jpeg"));
    }
    if(ui->checkBox_fileFmt_tif->isChecked()){
        nameFilters.append(tr("*.tif"));
        nameFilters.append(tr("*.tiff"));
    }
    if(ui->checkBox_fileFmt_gif->isChecked()){
        nameFilters.append(tr("*.gif"));
    }
    if(ui->checkBox_fileFmt_bmp->isChecked()){
        nameFilters.append(tr("*.bmp"));
    }
    if(ui->checkBox_fileFmt_png->isChecked()){
        nameFilters.append(tr("*.png"));
    }
    if(ui->checkBox_fileFmt_mov->isChecked()){
        nameFilters.append(tr("*.mov"));
    }
    if(ui->checkBox_fileFmt_avi->isChecked()){
        nameFilters.append(tr("*.avi"));
    }
    if(ui->checkBox_fileFmt_mpg->isChecked()){
        nameFilters.append(tr("*.mpg"));
        nameFilters.append(tr("*.mpeg"));
    }
    if(ui->checkBox_fileFmt_m4v->isChecked()){
        nameFilters.append(tr("*.m4v"));
    }
    if(ui->checkBox_fileFmt_mkv->isChecked()){
        nameFilters.append(tr("*.mkv"));
    }
    if(ui->checkBox_fileFmt_divx->isChecked()){
        nameFilters.append(tr("*.divx"));
    }
    if(ui->checkBox_fileFmt_mkv->isChecked()){
        nameFilters.append(tr("*.mp4"));
    }

    //
    // Get file list
    //

    m_picInCore->setFlagSubDir(ui->checkBox_includeSubDir->isChecked());

    numPic = m_picInCore->scanSrcFiles(nameFilters);
    if(numPic <= 0){
        emit signal_show_dialog("File not found");
        return;
    }

    //
    // Set overwrite flag
    //

    m_picInCore->setFlagOverwrite(ui->checkBox_overwt->isChecked());

    //
    // Confirmation for importing
    //

    QString fileNum;
    fileNum.sprintf("%d files will imported", numPic);

    DialogImportChecker *dialogImportChecker = new DialogImportChecker(0);
    dialogImportChecker->setParent(0);
    dialogImportChecker->setModal(true);
    dialogImportChecker->ui->label_sts->setText(fileNum);
    dialogImportChecker->setWindowTitle(tr(" "));
    dialogImportChecker->show();

    dialogImportChecker->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialogImportChecker, SIGNAL(accepted()), this, SLOT(slot_import()));
}

/*
 * name : slot_show_dialog
 * desc : Show dialog as indicated string
 */
void MainWindow::slot_show_dialog(QString infoToShow)
{
    dialog_invalid_path *dialogInvalidPath = new dialog_invalid_path(0);

    //Show dialog
    dialogInvalidPath->setParent(0);
    dialogInvalidPath->set_label_text(infoToShow);
    dialogInvalidPath->window()->setWindowTitle(" ");
    dialogInvalidPath->setModal(true);
    dialogInvalidPath->show();

    //Enable main window once dialog been closed
    dialogInvalidPath->setAttribute(Qt::WA_DeleteOnClose);
}

/*
 * name : slot_button_quit_clicked
 * desc : Clicked quit button, close program
 */
void MainWindow::slot_button_quit_clicked(void)
{
    QApplication::instance()->quit();
}

/*
 * name : slot_checkbox_dirAsDate_clicked
 * desc : Clicked checkBox_dirAsDate
 */
void MainWindow::slot_checkbox_dirAsDate_clicked(void)
{
    bool checked = false;
    checked = ui->checkBox_dirAsDate->isChecked();
    ui->radioBtn_dirAsY->setEnabled(checked);
    ui->radioBtn_dirAsYM->setEnabled(checked);
    ui->radioBtn_dirAsYMD->setEnabled(checked);

    //
    // Set year to default
    //

    if(checked &&
       !ui->radioBtn_dirAsY->isChecked() &&
       !ui->radioBtn_dirAsYM->isChecked() &&
       !ui->radioBtn_dirAsYMD->isChecked())
    {
        ui->radioBtn_dirAsY->setChecked(true);
    }
}

/*
 * name : slot_menu_about
 * desc : show about info
 */
void MainWindow::slot_menu_about(void)
{
    QString aboutTitle("About PicIn");
    QString aboutContent("PicIn "PICIN_VER"\n"
                         "Copyright (C) 2016, ChunKai-Wang, "
                         "kaiw1982@gmail.com\n"
                         "\n"
                         "PicIn is a tool be created through QT 5.5.1"
                         " to import pictures from somewhere"
                         "(usb, sd card, a folder...) to specific location.\n");
    QMessageBox::about(0, aboutTitle, aboutContent);
}

/*
 * name : slot_menu_aboutQt
 * desc : show about QT info
 */
void MainWindow::slot_menu_aboutQt(void)
{
    QMessageBox::aboutQt(0);
}

/*
 * name : slot_fileFmt_all
 * desc : Change all fileFmt checkboxes status
 */
void MainWindow::slot_fileFmt_all(bool checked)
{
    ui->checkBox_fileFmt_avi->setChecked(checked);
    ui->checkBox_fileFmt_bmp->setChecked(checked);
    ui->checkBox_fileFmt_divx->setChecked(checked);
    ui->checkBox_fileFmt_gif->setChecked(checked);
    ui->checkBox_fileFmt_jpg->setChecked(checked);
    ui->checkBox_fileFmt_m4v->setChecked(checked);
    ui->checkBox_fileFmt_mkv->setChecked(checked);
    ui->checkBox_fileFmt_mov->setChecked(checked);
    ui->checkBox_fileFmt_mp4->setChecked(checked);
    ui->checkBox_fileFmt_mpg->setChecked(checked);
    ui->checkBox_fileFmt_png->setChecked(checked);
    ui->checkBox_fileFmt_tif->setChecked(checked);
}

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("PicIn"));

    //Initialize radio buttons
    ui->radioBtn_dirAsY->setEnabled(false);
    ui->radioBtn_dirAsYM->setEnabled(false);
    ui->radioBtn_dirAsYMD->setEnabled(false);

    //declare PicIn_core
    m_picInCore = new PicIn_Core;

    //connect
    connect(ui->button_quit, SIGNAL(clicked(bool)), this, SLOT(slot_button_quit_clicked()));
    connect(ui->button_browse_source, SIGNAL(clicked(bool)), this, SLOT(slot_button_browse_source_clicked()));
    connect(ui->button_browse_target, SIGNAL(clicked(bool)), this, SLOT(slot_button_browse_target_clicked()));
    connect(ui->button_import, SIGNAL(clicked(bool)), this, SLOT(slot_button_import_clicked()));
    connect(this, SIGNAL(signal_show_dialog(QString)), this, SLOT(slot_show_dialog(QString)));
    connect(this, SIGNAL(signal_enalbe_window()), this, SLOT(slot_enable_window()));
    connect(this, SIGNAL(signal_disable_window()), this, SLOT(slot_disable_window()));
    connect(ui->checkBox_dirAsDate, SIGNAL(clicked(bool)), this, SLOT(slot_checkbox_dirAsDate_clicked()));
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
    emit this->signal_disable_window();

    //
    // Prepare progress bar
    //

    Dialog_Import_Progress *dialogImportProgress = new Dialog_Import_Progress(0);
    dialogImportProgress->setParent(0);
    dialogImportProgress->setAttribute(Qt::WA_DeleteOnClose);
    dialogImportProgress->setWindowTitle(tr(" "));
    dialogImportProgress->ui->progressBar->setRange(0, m_picInCore->getNumFilesSrc());
    dialogImportProgress->ui->progressBar->setValue(0);
    dialogImportProgress->show();

    connect(dialogImportProgress, SIGNAL(rejected()), this, SLOT(slot_import_canceled()));
    connect(m_picInCore, SIGNAL(signal_update_progress(int)), dialogImportProgress->ui->progressBar, SLOT(setValue(int)));

    //
    // Import files
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
    m_picInCore->import_doit();

    //
    // done.
    //

    emit this->signal_enalbe_window();
}


/*
 * name : slot_enable_window
 * desc : Enable window
 */
void MainWindow::slot_import_canceled(void)
{
    m_picInCore->setFlagCancel_true();
}

/*
 * name : slot_enable_window
 * desc : Enable window
 */
void MainWindow::slot_enable_window(void)
{
    MainWindow::setEnabled(true);
}

/*
 * name : slot_disable_window
 * desc : Disable window
 */
void MainWindow::slot_disable_window(void)
{
    MainWindow::setEnabled(false);
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

    dirPath = fileDialog.getExistingDirectory(
                  this,
                  tr("choose directory"),
                  "",
                  QFileDialog::ShowDirsOnly |
                  QFileDialog::DontResolveSymlinks |
                  QFileDialog::DontUseCustomDirectoryIcons);

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

    dirPath = fileDialog.getExistingDirectory(
                  this,
                  tr("choose directory"),
                  "",
                  QFileDialog::ShowDirsOnly |
                  QFileDialog::DontResolveSymlinks |
                  QFileDialog::DontUseCustomDirectoryIcons);

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
    // Get file list
    //

    QStringList nameFilters;
    int numPic = 0;

    nameFilters.append(tr("*.jpg"));
    nameFilters.append(tr("*.jpeg"));

    numPic = m_picInCore->scanSrcFiles(nameFilters);
    if(numPic <= 0){
        emit signal_show_dialog("No Pictures");
        return;
    }

    //
    // Confirmation for importing
    //

    QString fileNum;
    fileNum.sprintf("%d files will imported", numPic);

    emit signal_disable_window();

    DialogImportChecker *dialogImportChecker = new DialogImportChecker(0);
    dialogImportChecker->setParent(0);
    dialogImportChecker->ui->label_sts->setText(fileNum);
    dialogImportChecker->setWindowTitle(tr(" "));
    dialogImportChecker->show();

    dialogImportChecker->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialogImportChecker, SIGNAL(rejected()), this, SLOT(slot_enable_window()));
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
    dialogInvalidPath->show();

    //Disable main window
    emit signal_disable_window();

    //Enable main window once dialog been closed
    dialogInvalidPath->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialogInvalidPath, SIGNAL(destroyed(QObject*)), this, SLOT(slot_enable_window()));
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

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
    ui->checkBox_exifDate->setEnabled(false);

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

    //Read config
    readCfg();
}

MainWindow::~MainWindow()
{
    updateCfg();
    delete ui;
}

// ****************************************************************************
// ****                       Public functions:                            ****
// ****************************************************************************

/*
 * name : getSelectedPathFromView
 * desc : Get selected path of file/folder from a view
 * in   :
 *   *view, The point of view
 *   *fileList, Pointer of buffer to store selected file path list
 *   *dirList, Pointer of buffer to store selected folder path list
 * out  :
 *   *fileList, Selected file path QString list
 *   *dirList, Selected folder path QString list
 * ret  :
 *    A QStringList included all path list
 */
QStringList
MainWindow::getSelectedPathFromView(
    QAbstractItemView *view,
    QStringList *fileList,
    QStringList *dirList
)
{
    QStringList selList;
    QFileSystemModel *fsModel = (QFileSystemModel *)view->model();

    for(int i = 0; i < view->selectionModel()->selectedRows().size(); i++){
        QString path;
        QModelIndex index = view->selectionModel()->selectedRows().at(i);

        path = fsModel->filePath(index);
        if(path.size() > 0){
            if(fsModel->type(index).compare(tr(MODEL_FS_INDEX_TYPE_FOLDER)) && fileList){
                fileList->append(path);
            }
            else if(!fsModel->type(index).compare(tr(MODEL_FS_INDEX_TYPE_FOLDER)) && dirList){
                dirList->append(path);
            }
            selList.append(path);
        }
    }

    return selList;
}

/*
 * name : rmOverlapParentPath
 * desc : Remove unneeded selected parent path,
 *        example, select folder "/A/1/" and "/A/2/" by QFileDialog,
 *        it sometimes get "/A/", "/A/1/", "/A/2/",
 *        this function removes the unneeded "/A/"
 * in   :
 *   pathList, Selected path list
 * ret  :
 *    A QStringList of result
 */
QStringList MainWindow::rmOverlapParentPath(QStringList pathList)
{
    QStringList newPathList;
    bool overlap = true;

    newPathList.append(pathList);

    while(overlap){
        overlap = false;

        for(int i = 0; i < newPathList.size(); i++){
            QString rmPath = newPathList.at(i);

            for(int j = 0; j < newPathList.size(); j++){
                QString cmpPath = newPathList.at(j);

                if(i == j){
                    continue;
                }
                if(cmpPath.contains(rmPath, Qt::CaseSensitive)){
                    // rmPath is a part of other path,
                    // that means it is somebody's parent path,
                    // remove it.
                    overlap = true;
                    newPathList.removeAt(i);
                    break;
                }
            }

            if(overlap){
                break;
            }
        }
    }

    return newPathList;
}

// ****************************************************************************
// ****                      Private functions:                            ****
// ****************************************************************************

/*
 * name : updateCfg
 * desc : update text configuration file content
 */
void MainWindow::updateCfg(void)
{
    QString cfgFilePath = QDir::homePath();
    cfgFilePath.append("/"CFG_FILE_NAME);

    QFile cfgFile(cfgFilePath);

    if(cfgFile.exists()){
        cfgFile.remove();
    }

    if(!cfgFile.open(QIODevice::ReadWrite | QIODevice::Text)){
        return;
    }

    QTextStream out(&cfgFile);

    //
    // file path
    //

    if(ui->lineEdit_path_source->text().length() > 0){
        out << "source=" << ui->lineEdit_path_source->text() << "\n";
    }
    if(ui->lineEdit_path_target->text().length() > 0){
        out << "target=" << ui->lineEdit_path_target->text() << "\n";
    }

    //
    // file format
    //

    if(ui->checkBox_fileFmt_jpg->isChecked()){
        out << "fmt_jpg=1\n";
    }
    if(ui->checkBox_fileFmt_tif->isChecked()){
        out << "fmt_tif=1\n";
    }
    if(ui->checkBox_fileFmt_gif->isChecked()){
        out << "fmt_gif=1\n";
    }
    if(ui->checkBox_fileFmt_bmp->isChecked()){
        out << "fmt_bmp=1\n";
    }
    if(ui->checkBox_fileFmt_png->isChecked()){
        out << "fmt_png=1\n";
    }
    if(ui->checkBox_fileFmt_mov->isChecked()){
        out << "fmt_mov=1\n";
    }
    if(ui->checkBox_fileFmt_avi->isChecked()){
        out << "fmt_avi=1\n";
    }
    if(ui->checkBox_fileFmt_mpg->isChecked()){
        out << "fmt_mpg=1\n";
    }
    if(ui->checkBox_fileFmt_m4v->isChecked()){
        out << "fmt_m4v=1\n";
    }
    if(ui->checkBox_fileFmt_mkv->isChecked()){
        out << "fmt_mkv=1\n";
    }
    if(ui->checkBox_fileFmt_divx->isChecked()){
        out << "fmt_divx=1\n";
    }
    if(ui->checkBox_fileFmt_mp4->isChecked()){
        out << "fmt_mp4=1\n";
    }

    //
    // options
    //

    if(ui->checkBox_includeSubDir->isChecked()){
        out << "subdir=1\n";
    }
    if(ui->checkBox_overwt->isChecked()){
        out << "overwrite=1\n";
    }

    //
    // directory struct as date
    //

    if(ui->checkBox_dirAsDate->isChecked()){
        if(ui->checkBox_exifDate->isChecked()){
            out << "exifdate=1\n";
        }

        if(ui->radioBtn_dirAsY->isChecked()){
            out << "diry=1\n";
        }
        else if(ui->radioBtn_dirAsYM->isChecked()){
            out << "dirym=1\n";
        }
        else if(ui->radioBtn_dirAsYMD->isChecked()){
            out << "dirymd=1";
        }
    }

    cfgFile.close();

#ifdef Q_OS_WIN
    //
    // Set config file to hidden for windows os
    //

    wchar_t *wWinCfgFileName = NULL;
    int attr = 0;

    // the +1 is for null char
    wWinCfgFileName = new wchar_t[cfgFilePath.length() + 1];
    if(wWinCfgFileName == NULL){
        return;
    }

    wWinCfgFileName[cfgFilePath.length()] = 0;
    cfgFilePath.toWCharArray(wWinCfgFileName);

    attr = GetFileAttributes(wWinCfgFileName);
    SetFileAttributes(wWinCfgFileName, attr | FILE_ATTRIBUTE_HIDDEN);
    delete wWinCfgFileName;
#endif
}

/*
 * name : readCfg
 * desc : read and apply text configuration file content
 */
void MainWindow::readCfg(void)
{
    QString cfgFilePath = QDir::homePath();
    cfgFilePath.append("/"CFG_FILE_NAME);

    QFile cfgFile(cfgFilePath);
    if (!cfgFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }

    QTextStream cfgTextStream(&cfgFile);
    while (!cfgTextStream.atEnd()) {
        QString cfgLine;
        QString cfgValue;

        cfgLine = cfgTextStream.readLine();
        cfgValue = cfgLine.section('=', 1, 1);

        //
        // file path
        //

        if(cfgLine.startsWith(tr("source="))){
            ui->lineEdit_path_source->setText(cfgValue);
        }
        else if(cfgLine.startsWith(tr("target="))){
            ui->lineEdit_path_target->setText(cfgValue);
        }

        //
        // file format
        //

        else if(cfgLine.startsWith(tr("fmt_jpg="))){
            ui->checkBox_fileFmt_jpg->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_tif="))){
            ui->checkBox_fileFmt_tif->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_gif="))){
            ui->checkBox_fileFmt_gif->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_bmp="))){
            ui->checkBox_fileFmt_bmp->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_png="))){
            ui->checkBox_fileFmt_png->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_mov="))){
            ui->checkBox_fileFmt_mov->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_avi="))){
            ui->checkBox_fileFmt_avi->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_mpg="))){
            ui->checkBox_fileFmt_mpg->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_m4v="))){
            ui->checkBox_fileFmt_m4v->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_mkv="))){
            ui->checkBox_fileFmt_mkv->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_divx="))){
            ui->checkBox_fileFmt_divx->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("fmt_mp4="))){
            ui->checkBox_fileFmt_mp4->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("subdir="))){
            ui->checkBox_includeSubDir->setChecked(cfgValue.toInt(0, 10));
        }
        else if(cfgLine.startsWith(tr("overwrite="))){
            ui->checkBox_overwt->setChecked(cfgValue.toInt(0, 10));
        }

        //
        // directory struct as date
        //

        else if(cfgLine.startsWith(tr("exifdate="))){
            ui->checkBox_exifDate->setChecked(cfgValue.toInt(0, 10));
        }

        else if(cfgLine.startsWith(tr("diry="))){
            if(cfgValue.toInt(0, 10) == 1){
                ui->checkBox_dirAsDate->setChecked(false);
                ui->checkBox_dirAsDate->click();
                ui->radioBtn_dirAsY->setChecked(true);
            }
        }
        else if(cfgLine.startsWith(tr("dirym="))){
            if(cfgValue.toInt(0, 10) == 1){
                ui->checkBox_dirAsDate->setChecked(false);
                ui->checkBox_dirAsDate->click();
                ui->radioBtn_dirAsYM->setChecked(true);
            }
        }
        else if(cfgLine.startsWith(tr("dirymd="))){
            if(cfgValue.toInt(0, 10) == 1){
                ui->checkBox_dirAsDate->setChecked(false);
                ui->checkBox_dirAsDate->click();
                ui->radioBtn_dirAsYMD->setChecked(true);
            }
        }
    }

    cfgFile.close();
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
        m_picInCore->setFlagExifDate(ui->checkBox_exifDate->isChecked());
    }
    else{
        m_picInCore->setFlagDir(false, false, false);
        m_picInCore->setFlagExifDate(false);
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
    QStringList dirList;
    QDir dir;

    //
    // Set fileDialog
    //

    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
    fileDialog.setOption(QFileDialog::DontResolveSymlinks, true);
    fileDialog.setOption(QFileDialog::DontUseCustomDirectoryIcons, true);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    fileDialog.setParent(0);
    fileDialog.setWindowTitle(tr("Choose directory"));
    fileDialog.setDirectory(QDir::home());

    //
    // Get treeView from fileDialog and set multi selection
    //

    QTreeView *treeView = fileDialog.findChild<QTreeView*>("treeView");
    if (treeView) {
       treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    QListView *listView = fileDialog.findChild<QListView*>("listView");
    if (listView) {
       listView->setSelectionMode(QAbstractItemView::MultiSelection);
    }

    fileDialog.exec();

    //
    // Get data(string list of selection path) from view
    //

    getSelectedPathFromView(treeView, NULL, &dirList);
    dirList = rmOverlapParentPath(dirList);

    //
    // Update ui
    //

    dirPath.clear();
    for(int i = 0; i < dirList.size(); i++){
        dirPath.append(dir.toNativeSeparators(dirList.at(i)));
        if(i+1 < dirList.size()){
            dirPath.append(tr(";"));

        }
    }
    ui->lineEdit_path_source->setText(dirPath);
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

    if(ui->lineEdit_path_source->text().length() == 0){
        emit signal_show_dialog(tr("Please input source path"));
        return;
    }

    if(ui->lineEdit_path_target->text().length() == 0){
        emit signal_show_dialog(tr("Please input target path"));
        return;
    }

    //
    // Check whether path is exist
    //

    int ret = 0;
    ret = m_picInCore->set_path(
                ui->lineEdit_path_source->text().split(";"),
                PicIn_Core::PT_Source);
    if(ret != 0){
        QString errStr;

        if(ret > 0){
            errStr.sprintf("Source path %d isn't exist", ret);
        }
        else{
            errStr.sprintf("Source path isn't' exist");
        }
        emit signal_show_dialog(errStr);
        return;
    }

    // Currently, only support single target path
    if(ui->lineEdit_path_target->text().split(";").size() > 1){
        emit signal_show_dialog("Only support single target path");
        return;
    }

    ret = m_picInCore->set_path(
               ui->lineEdit_path_target->text().split(";"),
               PicIn_Core::PT_Target);
    if(ret != 0){
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

    if(nameFilters.size() == 0){
        emit signal_show_dialog("Please choose least one file type");
        return;
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
    Dialog_Msg *dialogMsg = new Dialog_Msg(0);

    //Show dialog
    dialogMsg->setParent(0);
    dialogMsg->set_label_text(infoToShow);
    dialogMsg->window()->setWindowTitle(" ");
    dialogMsg->setModal(true);
    dialogMsg->setAttribute(Qt::WA_DeleteOnClose);
    dialogMsg->exec();
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
    ui->checkBox_exifDate->setEnabled(checked);

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QTextStream>
#include <QTreeView>
#include <QListView>
#include <QFileSystemModel>

#include "ui_mainwindow.h"
#include "dialog_msg.h"
#include "dialogimportchecker.h"
#include "dialog_import_progress.h"
#include "ui_dialogimportchecker.h"
#include "ui_dialog_msg.h"
#include "ui_dialog_import_progress.h"
#include "picin_core.h"

#ifdef Q_OS_WIN
#include "Windows.h"
#endif

#define PICIN_VER "v0.1"
#define CFG_FILE_NAME ".picin.cfg"

//
// OS specic define
//

#ifdef Q_OS_WIN
#define MODEL_FS_INDEX_TYPE_FOLDER "File Folder"
#endif

#ifdef Q_OS_LINUX
#define MODEL_FS_INDEX_TYPE_FOLDER "Folder"
#endif

//
// declare class
//

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QStringList getSelectedPathFromView(QAbstractItemView *view, QStringList *fileList, QStringList *dirList);
    QString modelIndexToFullPath(QFileSystemModel *model, QModelIndex index);
    QStringList rmOverlapParentPath(QStringList pathList);

private:
    Ui::MainWindow *ui;
    PicIn_Core *m_picInCore;
    bool m_flagImportCancel;
    void readCfg(void);
    void updateCfg(void);

signals:
    void signal_show_dialog(QString);

public slots:
    void slot_import();
    void slot_import_canceled();

private slots:
    void slot_button_browse_source_clicked(void);
    void slot_button_browse_target_clicked(void);
    void slot_button_import_clicked(void);
    void slot_button_quit_clicked(void);
    void slot_show_dialog(QString);
    void slot_checkbox_dirAsDate_clicked(void);
    void slot_menu_about(void);
    void slot_menu_aboutQt(void);
    void slot_fileFmt_all(bool checked);
};

#endif // MAINWINDOW_H

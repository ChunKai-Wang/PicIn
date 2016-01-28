#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfoList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QFileInfoList get_file_list(QString path, QStringList exNameFilters);

    QFileInfoList m_fileInfoList_img;
    QString m_path_source;
    QString m_path_target;

signals:
    void signal_show_dialog(QString);
    void signal_enalbe_window(void);
    void signal_disable_window(void);

public slots:
    void slot_enable_window(void);
    void slot_disable_window(void);
    void slot_import();

private slots:
    void slot_button_browse_source_clicked(void);
    void slot_button_browse_target_clicked(void);
    void slot_button_import_clicked(void);
    void slot_button_quit_clicked(void);
    void slot_show_dialog(QString);
    void slot_show_import_checker(QString);
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

signals:
    void signal_show_dialog(QString);

private slots:
    void slot_button_browse_source_clicked(void);
    void slot_button_browse_target_clicked(void);
    void slot_button_import_clicked(void);
    void slot_button_quit_clicked(void);
    void slot_show_dialog(QString);
};

#endif // MAINWINDOW_H

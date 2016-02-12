#ifndef DIALOG_INVALID_PATH_H
#define DIALOG_INVALID_PATH_H

#include <QDialog>

namespace Ui {
class Dialog_Msg;
}

class Dialog_Msg : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Msg(QWidget *parent = 0);
    ~Dialog_Msg();
    void set_label_text(QString);

    Ui::Dialog_Msg *ui;
};

#endif // DIALOG_INVALID_PATH_H

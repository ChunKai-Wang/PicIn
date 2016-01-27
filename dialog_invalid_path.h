#ifndef DIALOG_INVALID_PATH_H
#define DIALOG_INVALID_PATH_H

#include <QDialog>

namespace Ui {
class dialog_invalid_path;
}

class dialog_invalid_path : public QDialog
{
    Q_OBJECT

public:
    explicit dialog_invalid_path(QWidget *parent = 0);
    ~dialog_invalid_path();
    void set_label_text(QString);

private:
    Ui::dialog_invalid_path *ui;
};

#endif // DIALOG_INVALID_PATH_H

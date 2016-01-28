#ifndef DIALOG_IMPORT_PROGRESS_H
#define DIALOG_IMPORT_PROGRESS_H

#include <QDialog>

namespace Ui {
class Dialog_Import_Progress;
}

class Dialog_Import_Progress : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Import_Progress(QWidget *parent = 0);
    ~Dialog_Import_Progress();

    Ui::Dialog_Import_Progress *ui;
};

#endif // DIALOG_IMPORT_PROGRESS_H

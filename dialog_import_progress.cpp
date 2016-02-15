#include "dialog_import_progress.h"
#include "ui_dialog_import_progress.h"

Dialog_Import_Progress::Dialog_Import_Progress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Import_Progress)
{
    ui->setupUi(this);
}

Dialog_Import_Progress::~Dialog_Import_Progress()
{
    delete ui;
}

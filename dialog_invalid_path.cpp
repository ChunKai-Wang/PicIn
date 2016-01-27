#include "dialog_invalid_path.h"
#include "ui_dialog_invalid_path.h"

dialog_invalid_path::dialog_invalid_path(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_invalid_path)
{
    ui->setupUi(this);
    connect(ui->button_invalid_path_ok, SIGNAL(clicked(bool)),this,SLOT(close()));
}

dialog_invalid_path::~dialog_invalid_path()
{
    delete ui;
}

void dialog_invalid_path::set_label_text(QString label_text)
{
    ui->label_text_invalid_path->setText(label_text);
}

#include "dialog_msg.h"
#include "ui_dialog_msg.h"

Dialog_Msg::Dialog_Msg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Msg)
{
    ui->setupUi(this);
    connect(ui->button_invalid_path_ok, SIGNAL(clicked(bool)),this,SLOT(close()));
}

Dialog_Msg::~Dialog_Msg()
{
    delete ui;
}

//
// Public functions
//

void Dialog_Msg::set_label_text(QString label_text)
{
    ui->label_text_invalid_path->setText(label_text);
}

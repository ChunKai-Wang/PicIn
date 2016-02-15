#include "dialogimportchecker.h"
#include "ui_dialogimportchecker.h"

DialogImportChecker::DialogImportChecker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImportChecker)
{
    ui->setupUi(this);
}

DialogImportChecker::~DialogImportChecker()
{
    delete ui;
}

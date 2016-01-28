#ifndef DIALOGIMPORTCHECKER_H
#define DIALOGIMPORTCHECKER_H

#include <QDialog>

namespace Ui {
class DialogImportChecker;
}

class DialogImportChecker : public QDialog
{
    Q_OBJECT

public:
    explicit DialogImportChecker(QWidget *parent = 0);
    ~DialogImportChecker();

    Ui::DialogImportChecker *ui;
};

#endif // DIALOGIMPORTCHECKER_H

#include "dialogwait.h"
#include "ui_dialogwait.h"

DialogWait::DialogWait(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogWait)
{
    Q_ASSERT_X(parent != NULL, "DialogWait::DialogWait", "parent for DialogWait should not be NULL");
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setEnabled(false);
}

DialogWait::~DialogWait()
{
    delete ui;
}

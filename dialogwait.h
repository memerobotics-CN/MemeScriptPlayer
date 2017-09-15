#ifndef DIALOGWAIT_H
#define DIALOGWAIT_H

#include <QDialog>

namespace Ui {
class DialogWait;
}

class DialogWait : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWait(QWidget *parent = 0);
    ~DialogWait();

private:
    Ui::DialogWait *ui;
};

#endif // DIALOGWAIT_H

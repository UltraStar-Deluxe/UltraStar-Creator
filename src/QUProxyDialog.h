#ifndef QUPROXYDIALOG_H
#define QUPROXYDIALOG_H

#include <QDialog>

#include "ui_QUProxyDialog.h"

class QUProxyDialog: public QDialog, private Ui::QUProxyDialog
{
    Q_OBJECT

public:
    QUProxyDialog();

public slots:
        virtual void accept();

private slots:
    void on_checkBox_useProxy_clicked(bool checked);
};

#endif // QUPROXYDIALOG_H

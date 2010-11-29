#include "QUProxyDialog.h"
#include "ui_QUProxyDialog.h"

#include <QSettings>

QUProxyDialog::QUProxyDialog()
{
    setupUi(this);

    connect(acceptBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(rejectBtn, SIGNAL(clicked()), this, SLOT(reject()));

    QSettings settings;

    if(settings.contains("useProxy")) {
        bool proxyUsed = settings.value("useProxy").toBool();
        checkBox_useProxy->setChecked(proxyUsed);
        lineEdit_ProxyServer->setEnabled(proxyUsed);
        spinBox_ProxyPort->setEnabled(proxyUsed);
    }
    if(settings.contains("proxyServer")) {
        lineEdit_ProxyServer->setText(settings.value("proxyServer").toString());
    }
    if(settings.contains("proxyPort")) {
        spinBox_ProxyPort->setValue(settings.value("proxyPort").toUInt());
    }
}

void QUProxyDialog::on_checkBox_useProxy_clicked(bool checked)
{
    label_ProxyServer->setEnabled(checked);
    lineEdit_ProxyServer->setEnabled(checked);
    label_ProxyPort->setEnabled(checked);
    spinBox_ProxyPort->setEnabled(checked);
}

void QUProxyDialog::accept() {
        QSettings settings;
        settings.setValue("useProxy", checkBox_useProxy->isChecked());
        settings.setValue("proxyServer", lineEdit_ProxyServer->text());
        settings.setValue("proxyPort", spinBox_ProxyPort->value());

        QDialog::accept();
}

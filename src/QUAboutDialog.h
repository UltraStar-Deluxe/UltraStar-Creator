#ifndef QUABOUTDIALOG_H_
#define QUABOUTDIALOG_H_

#include <QDialog>

#include "ui_QUAboutDialog.h"

class QUAboutDialog: public QDialog, private Ui::QUAboutDialog {
	Q_OBJECT

public:
	QUAboutDialog(QWidget *parent = 0);

private slots:
	void scrollDown();
	void resetText();
};

#endif /* QUABOUTDIALOG_H_ */

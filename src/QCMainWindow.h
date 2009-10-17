#ifndef QCMAINWINDOW_H
#define QCMAINWINDOW_H

#include <QMainWindow>
#include "ui_QCMainWindow.h"

class QCMainWindow: public QMainWindow, private Ui::QCMainWindow {
	Q_OBJECT

public:
	QCMainWindow(QWidget *parent = 0);
};

#endif // QCMAINWINDOW_H

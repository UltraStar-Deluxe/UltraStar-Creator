#include "QCMainWindow.h"

#include "QULogService.h"

QCMainWindow::QCMainWindow(QWidget *parent): QMainWindow(parent) {
	setupUi(this);
	logSrv->add(tr("Ready."), QU::Information);
}

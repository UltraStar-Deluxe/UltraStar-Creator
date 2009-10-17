#include "QULogService.h"

QULogService::QULogService(QObject *parent): QObject(parent) {
}

QULogService* QULogService::_instance = 0;
QULogService* QULogService::instance() {
	if(_instance == 0)
		_instance = new QULogService(0);

	return _instance;
}

void QULogService::add(const QString &message, QU::MessageTypes type) {
	emit messageAdded(message, type);
}

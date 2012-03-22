#ifndef QUMONTYAREA_H_
#define QUMONTYAREA_H_

#include "QU.h"

#include <QWidget>

#include "ui_QUMontyArea.h"

class QUMontyArea: public QWidget, public Ui::QUMontyArea {
	Q_OBJECT

public:
	QUMontyArea(QWidget *parent);
};

#endif /* QUMONTYAREA_H_ */

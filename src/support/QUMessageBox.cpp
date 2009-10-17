#include "QUMessageBox.h"

#include <QIcon>
#include <QPixmap>
#include <QVariant>

QUMessageBox::QUMessageBox(QWidget *parent): QDialog(parent) {
	setupUi(this);

	this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

	_choice = -1;

	connect(&_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(makeChoice(int)));
}

void QUMessageBox::closeEvent(QCloseEvent *event) {
	if(_choice == -1) {
		event->ignore();
	} else {
		event->accept();
	}
}

int QUMessageBox::information(QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons, int widthHint, int defaultIndex) {
	QUMessageBox *dlg = new QUMessageBox(parent);

	dlg->setIcon(":/marks/information.png");

	int result = dlg->showMessage(title, msg, buttons, defaultIndex, widthHint);
	delete dlg;
	return result;
}

int QUMessageBox::question(QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons, int widthHint, int defaultIndex) {
	QUMessageBox *dlg = new QUMessageBox(parent);

	dlg->setIcon(":/marks/help.png");

	int result = dlg->showMessage(title, msg, buttons, defaultIndex, widthHint);
	delete dlg;
	return result;
}

int QUMessageBox::warning(QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons, int widthHint, int defaultIndex) {
	QUMessageBox *dlg = new QUMessageBox(parent);

	dlg->setIcon(":/marks/error.png");

	int result = dlg->showMessage(title, msg, buttons, defaultIndex, widthHint);
	delete dlg;
	return result;
}

int QUMessageBox::critical(QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons, int widthHint, int defaultIndex) {
	QUMessageBox *dlg = new QUMessageBox(parent);

	dlg->setIcon(":/marks/cancel.png");

	int result = dlg->showMessage(title, msg, buttons, defaultIndex, widthHint);
	delete dlg;
	return result;
}

int QUMessageBox::showMessage(const QString &title, const QString &msg, const QStringList &buttons, int defaultIndex, int widthHint) {
	/* debug start */
	delete one;
	delete two;
	delete three;
	/* debug end */

	if(defaultIndex == -1)
		defaultIndex = (buttons.size() / 2 - 1);

	for(int i = 1; i < buttons.size(); i += 2) {
		buttonLayout->addWidget(createButton(
				QIcon(buttons.at(i - 1)),
				buttons.at(i),
				(i - 1) / 2 == defaultIndex));
	}

	setWindowTitle(title);
	message->setText(msg);

	resize(widthHint > -1 ? widthHint : width(), minimumSizeHint().height());

	exec();

	return _choice;
}

QPushButton* QUMessageBox::createButton(const QIcon &icon, const QString &text, bool isDefault) {
	QPushButton *btn = new QPushButton(icon, text, this);

	if(isDefault)
		btn->setFocus();

	_buttonGroup.addButton(btn, _buttonGroup.buttons().size());
	btn->setShortcut(QVariant(_buttonGroup.buttons().size()).toString());

	return btn;
}

void QUMessageBox::setIcon(const QString &fileName) {
	icon->setPixmap(QPixmap(fileName));
}

void QUMessageBox::makeChoice(int id) {
	_choice = id;
	this->accept();
}

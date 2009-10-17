#ifndef QUMESSAGEBOX_H_
#define QUMESSAGEBOX_H_

// just vor convenience with message boxes
#define BTN QStringList()

#include <QDialog>
#include <QCloseEvent>
#include <QStringList>
#include <QPushButton>
#include <QMessageBox>

#include "QU.h"

#include "ui_QUMessageBox.h"

class QUMessageBox: public QDialog, private Ui::QUMessageBox {
	Q_OBJECT

public:
	static int information(QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons = QStringList() << ":/marks/accept.png" << "OK", int widthHint = -1, int defaultIndex = -1);
	static int question   (QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons = QStringList() << ":/marks/accept.png" << "OK", int widthHint = -1, int defaultIndex = -1);
	static int warning    (QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons = QStringList() << ":/marks/accept.png" << "OK", int widthHint = -1, int defaultIndex = -1);
	static int critical   (QWidget *parent, const QString &title, const QString &msg, const QStringList &buttons = QStringList() << ":/marks/accept.png" << "OK", int widthHint = -1, int defaultIndex = -1);

public slots:
	virtual void reject() { /* cannot reject */ }

protected:
	QUMessageBox(QWidget *parent = 0);

	int choice() const { return _choice; }
	virtual void closeEvent(QCloseEvent *event);

private slots:
	void makeChoice(int id);

private:
	QButtonGroup _buttonGroup;
	int _choice;

	int showMessage(const QString &title, const QString &msg, const QStringList &buttons, int defaultIndex, int widthHint);
	QPushButton* createButton(const QIcon &icon, const QString &text, bool isDefault = false);
	void setIcon(const QString &fileName);
};

#endif /*QUMESSAGEBOX_H_*/

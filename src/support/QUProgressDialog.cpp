#include "QUProgressDialog.h"
#include <QPixmap>
#include <QTimer>

#define HIDE_LIMIT       2 // only show this dialog above HIDE_LIMIT
#define MINIMUM_DURATION 500 // show after milliseconds
#define STEP_SIZE        500 // show every milliseconds

/*!
 * \param thread The thread which is executed. Will be deleted after finishing.
 * \param parent parent object
 */
QUProgressDialog::QUProgressDialog(
		const QString &info,
		int maximum,
		QWidget *parent,
		bool beResponsive): QDialog(parent), _cancelled(false), _beResponsive(beResponsive), _progress(0)
{
	setupUi(this);

	this->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

	progress->setRange(0, maximum);
	progress->setValue(0);

	infoTextLbl->setText(info);

	if(beResponsive) {
		connect(cancelBtn, SIGNAL(clicked()), this, SLOT(cancel()));
//		timeLbl->setText("00:00:00");
//		QTimer::singleShot(1000, this, SLOT(updateTime()));
	} else {
		cancelBtn->setEnabled(false);
//		timeLbl->setText(tr("n/a"));
	}
}

void QUProgressDialog::show() {
	_startTime = QTime::currentTime();
//	if(progress->maximum() > HIDE_LIMIT)
//		QDialog::show();
}

/*!
 * Updates the information of which items is being processed and increases the
 * counter by one.
 */
void QUProgressDialog::update(const QString &itemText, bool forceUpdate) {
	// update internal data
	_progress += 1;
	_label = itemText;

	// defer dialog appearance
	if(!isVisible()) {
		if(_startTime.msecsTo(QTime::currentTime()) >= MINIMUM_DURATION) {
			QDialog::show();
		} else
			return;
	}

	// only show update in certain steps
	if(!forceUpdate and _lastStep.msecsTo(QTime::currentTime()) < STEP_SIZE)
		return;

	progress->setValue(_progress);
	currentSongLbl->setText(_label);
	setWindowTitle(QString(tr("Progress (%1 of %2)")).arg(_progress).arg(progress->maximum()));
	updateTime();

	if(_beResponsive)
		QCoreApplication::processEvents(); // <-- more responsive, less performance
	else
		QWidget::repaint(); // <-- more performance, less responsive

	// reset step timer
	_lastStep = QTime::currentTime();
}

void QUProgressDialog::setPixmap(const QString &fileName) {
	currentSongIcon->setPixmap(fileName);
}

void QUProgressDialog::setLabel(const QString &text) {
	infoTextLbl->setText(text);
}

void QUProgressDialog::cancel() {
	_cancelled = true;
}

void QUProgressDialog::updateTime() {
	int secondsElapsed = _startTime.secsTo(QTime::currentTime());
	double speed = secondsElapsed > 0 ? (double)_progress / (double)secondsElapsed : 0.0;
	int secondsEstimated = speed > 0 ? qRound((double)progress->maximum() / speed) : 0;

	int secondsRemaining = (secondsEstimated - secondsElapsed) % 60;
	int minutesRemaining = (secondsEstimated - secondsElapsed) / 60;

	if(minutesRemaining == 0) {
		secondsRemaining = qRound(secondsRemaining / 5.0) * 5;
		if(secondsRemaining == 0) {
			secondsRemaining = 5;
		}
	} else if(minutesRemaining < 4){
		secondsRemaining = qRound(secondsRemaining / 10.0) * 10;
	} else {
		secondsRemaining = qRound(secondsRemaining / 20.0) * 20;
	}

	QString timeLblText;

	if(minutesRemaining > 0) {
		if(minutesRemaining == 1) {
			timeLblText.append(tr("<b>1</b> minute"));
		} else {
			timeLblText.append(tr("<b>%1</b> minutes").arg(minutesRemaining));
		}
		if(secondsRemaining > 0) {
			timeLblText.append(", ");
		}
	}

	if(secondsRemaining > 0) {
		if(secondsRemaining == 1) {
			timeLblText.append(tr("<b>1</b> second"));
		} else {
			timeLblText.append(tr("<b>%1</b> seconds").arg(secondsRemaining));
		}
	}

	if(secondsElapsed > 1 && secondsEstimated > 0) {
		timeLbl->setText(tr("about %1").arg(timeLblText));
	} else {
		timeLbl->setText("Calculating...");
	}

	// show performance of current operation
	int iSpeed = qRound(speed);
	if(secondsElapsed > 0)
		speedLbl->setText(QString(tr("<b>%1%2</b> items/sec")).arg(iSpeed == 0 ? "&lt;" : "").arg(qMax(1, iSpeed)));
	else
		speedLbl->setText("-");
}

#include "QUAutoCue.h"
#include "QULogService.h"

#include <QColor>
#include <QScrollBar>

QUAutoCue::QUAutoCue(QWidget *parent):
		QTextBrowser(parent),
		_waitedMilliseconds(0),
		_stopRequested(false),
		_pauseRequested(false),
		_cueListIndex(-1)
{
	setFontFamily("Tahoma");
	setFontPointSize(10);
}

/*!
 * Prepare autocue for new song.
 */
void QUAutoCue::reset(const QList<QUSongLineInterface*> &lines, double bpm, double gap, double isRelative) {
	clear();
	_cueList.clear();
	_cueListIndex = -1;

	int time = 0; // absolute time after beginning (milliseconds)
	int pos = 0; // absolute cursor position in text/lyrics

	int relTime = qRound(gap);

	int lineNumber = 0;
	foreach(QUSongLineInterface *line, lines) {
		int last = 0;
		foreach(QUSongNoteInterface *note, line->notes()) {
			if(note != line->notes().first()) {
				if(last == note->timestamp()) // no space
					_cueList.pop_back();
			}
			last = note->timestamp() + note->duration();

			if(isRelative)
				time = relTime + qRound((note->timestamp() / bpm) * 60000);
			else
				time = qRound(gap) + qRound((note->timestamp() / bpm) * 60000);

			if(note->type() == QUSongNoteInterface::golden)
				setFontWeight(QFont::Bold);
			else if(note->type() == QUSongNoteInterface::freestyle)
				setFontItalic(true);

			setAlignment(Qt::AlignHCenter);
			insertPlainText(note->syllable());

			setFontWeight(QFont::Normal);
			setFontItalic(false);

			_cueList << QUCueInfo(time, pos, note->syllable().size(), lineNumber);

			pos += note->syllable().size();

			time += qRound((note->duration() / bpm) * 60000);
			_cueList << QUCueInfo(time, pos, 0, lineNumber);
		}
		insertPlainText("\n"); pos++;

		if(isRelative) {
			relTime += qRound((line->inTime() / bpm) * 60000);
		}
		lineNumber += 1;
	}

	QTextCursor tc = textCursor();
	tc.setPosition(0);
	setTextCursor(tc);
}

void QUAutoCue::play() {
	seek(0);
}

void QUAutoCue::stop() {
	_stopRequested = true;
	_pauseRequested = false;
	clear();
}

void QUAutoCue::pause() {
	_pauseTime = QTime::currentTime();
	_pauseRequested = true;
	_stopRequested = false;
}

void QUAutoCue::resume(double position) {
	if(_cueList.isEmpty())
		return;

	_cueListIndex = 0;
	int targetTime = (int)(position * 1000);

	while(_cueListIndex < _cueList.size() && _cueList.at(_cueListIndex).time < targetTime)
		_cueListIndex++;

	if(_cueListIndex >= _cueList.size())
		return;

	_waitedMilliseconds += _pauseTime.elapsed();

	_pauseRequested = false;
	_stopRequested = false;
	update();
}

void QUAutoCue::seek(double position) {
	_startTime = QTime::currentTime();
	_startTime.start();
	_waitedMilliseconds = (int)(-position * 1000);
	_cueListIndex = 0;
	_stopRequested = false;
	_pauseRequested = false;
	update();
}

/*!
 * Highlight the correct syllable. Handle scrolling and so on.
 */
void QUAutoCue::update() {
	int time = _startTime.elapsed() - _waitedMilliseconds;
	int maxDelay = 30; // milliseconds
	int frequency = 10; // milliseconds

	forever{
		if(_cueListIndex >= _cueList.size())
			return;

		if(_cueList.at(_cueListIndex).time < time) { // you are too late
//			logSrv->add("Too late!", QU::Information);
			_cueListIndex++;
			continue;
		}

		int delay = _cueList.at(_cueListIndex).time - time;
		if(delay >= maxDelay) { // you are too early, come back later
//			logSrv->add("Too early!", QU::Information);
			if(!_stopRequested)
				QTimer::singleShot(frequency, this, SLOT(update()));
			return;
		} else {
//			logSrv->add(QString::number(delay), QU::Information);
			break;
		}
	}

	mark(_cueList.at(_cueListIndex).pos, _cueList.at(_cueListIndex).width);
	_cueListIndex++;

	if(!_stopRequested && !_pauseRequested)
		QTimer::singleShot(frequency, this, SLOT(update()));
}

void QUAutoCue::requestSongEdit() {
	QAction *a = qobject_cast<QAction*>(sender());
	if(a)
		emit editSongRequested(a->data().toInt());
}

void QUAutoCue::contextMenuEvent(QContextMenuEvent *event) {
	if(_cueListIndex < 0 || _cueListIndex >= _cueList.size()) {
		event->ignore();
		return;
	}
	QMenu *m = new QMenu(this);

	int lineIndex = _cueList.at(_cueListIndex).line;

	QAction *a = new QAction(QIcon(":/control/text_edit.png"), tr("Edit lyrics at line %1...").arg(lineIndex + 1), this);
	a->setData(lineIndex);
	connect(a, SIGNAL(triggered()), this, SLOT(requestSongEdit()));

	m->addAction(a);
	m->exec(QCursor::pos());
	delete m;

	event->accept();
}

void QUAutoCue::mark(int pos, int width) {
	QScrollBar *bar = verticalScrollBar();
	bar->setValue(bar->maximum());

	// remove any highlighting
	QTextCursor tc = textCursor();	
	tc.select(QTextCursor::Document);
	setTextCursor(tc);
	setTextColor(Qt::black);
	setTextBackgroundColor(Qt::white);

	// select current syllable
	tc.setPosition(pos);
	tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, width);
	setTextCursor(tc);

	// highlight selection
	setTextColor(Qt::white);
	setTextBackgroundColor(Qt::black);

	// remove selection
	tc.setPosition(pos);
	setTextCursor(tc);
}

#include "QUSongLine.h"
#include "QULogService.h"

QUSongNote::QUSongNote(Types ty, int t, int d, int p, const QString &s, QObject *parent):
		QUSongNoteInterface(parent),
		_type(ty),
		_timestamp(t),
		_duration(d),
		_pitch(p),
		_syllable(s)
{}

/*!
 * Reset the number of leading and following spacse.
 */
void QUSongNote::resetTrailingSpaces(int prefixCount, int suffixCount) {
	// the simple way
	if(prefixCount != -1 and suffixCount != -1) {
		setSyllable(syllable().trimmed());

		for(int i = 0; i < prefixCount; i++)
			_syllable.prepend(" ");
		for(int i = 0; i < suffixCount; i++)
			_syllable.append(" ");

		return;
	}

	// the hard way
	if(prefixCount == -1) {
		// prefix unchanged

		for(int i = 0; i < syllable().length(); i++) {
			if(!syllable().endsWith(" "))
				break;

			_syllable.remove(syllable().length() - 1, 1);
		}

		for(int i = 0; i < suffixCount; i++)
			_syllable.append(" ");
	}

	if(suffixCount == -1) {
		// suffix unchanged

		for(int i = 0; i < syllable().length(); i++) {
			if(!syllable().startsWith(" "))
				break;

			_syllable.remove(0, 1);
		}

		for(int i = 0; i < prefixCount; i++)
			_syllable.prepend(" ");
	}
}

// --------------------------------------------------------------------------------------

QUSongLine::QUSongLine(QObject *parent):
		QUSongLineInterface(parent),
		_out(0),
		_in(0),
		_useOutTime(false),
		_useInTime(false),
		_singer(QUSongLineInterface::undefined)
{}

QUSongLine::~QUSongLine() {
	qDeleteAll(_notes);
	_notes.clear();
}

void QUSongLine::addNote(QUSongNoteInterface *note) {
	if(!note)
		return;

	_notes.append(note);
}

QString QUSongLine::toString() const {
	QString line;
	foreach(QUSongNoteInterface *note, notes())
		line.append(note->syllable());
	return line;
}

QStringList QUSongLine::syllables() const {
	QStringList s;
	foreach(QUSongNoteInterface *note, this->notes())
		s << note->syllable();
	return s;
}

void QUSongLine::setSyllables(const QStringList &s) {
	QList<QUSongNoteInterface*> notes = this->notes();

	if(notes.size() != s.size()) {
		logSrv->add(tr("Cannot set syllables. Count does not match."), QU::Warning);
		return;
	}

	for(int i = 0; i < s.size(); i++)
		notes.at(i)->setSyllable(s.at(i));
}

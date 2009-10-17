#ifndef QUSONGLINE_H_
#define QUSONGLINE_H_

#include <QObject>
#include <QString>
#include <QStringList>

#include "QUSongInterface.h"

class QUSongNote: public QUSongNoteInterface {
	Q_OBJECT

public:
	QUSongNote(Types ty, int t, int d, int p, const QString &s, QObject *parent = 0);

	virtual int timestamp() const { return _timestamp; }
	virtual void setTimestamp(int t) { _timestamp = t; }

	virtual int duration() const { return _duration; }
	virtual void setDuration(int d) { _duration = d; }

	virtual int pitch() const { return _pitch; }
	virtual void setPitch(int p) { _pitch = p; }

	virtual Types type() const { return _type; }
	virtual void setType(Types t) { _type = t; }

	virtual QString syllable() const { return _syllable; }
	virtual void setSyllable(const QString &s) { _syllable = s; }

	virtual void resetTrailingSpaces(int prefixCount, int suffixCount);

private:
	// ": 200 5 10 foo" => "type timestamp duration pitch syllable"
	Types   _type;
	int     _timestamp;
	int     _duration;
	int     _pitch;
	QString _syllable;
};

// --------------------------------------------------------------------------

class QUSongLine: public QUSongLineInterface {
	Q_OBJECT

public:
	QUSongLine(QObject *parent = 0);
	~QUSongLine();

	virtual void addNote(QUSongNoteInterface *note);
	virtual QList<QUSongNoteInterface*> notes() const { return _notes; }

	virtual int outTime() const { return _out; }
	virtual void setOutTime(int out) { _out = out; _useOutTime = true; }
	virtual int inTime() const { return _in; }
	virtual void setInTime(int in) { _in = in; _useInTime = true; }

	virtual bool useOutTime() const { return _useOutTime; }
	virtual bool useInTime() const { return _useInTime; }
	virtual void removeInTime() { _useInTime = false; }

	virtual Singers singer() const { return _singer; }
	virtual void setSinger(Singers s) { _singer = s; }

	virtual QString toString() const;
	virtual QStringList syllables() const;
	virtual void setSyllables(const QStringList &s);

private:
	QList<QUSongNoteInterface*> _notes;

	int _out; // timestamp this line disappears
	int _in; // timestamp next line appears

	bool _useOutTime;
	bool _useInTime; // special line break

	Singers _singer;
};

#endif /* QUSONGLINE_H_ */

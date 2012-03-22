#ifndef QUMONTY_H_
#define QUMONTY_H_

#define monty (QUMonty::instance())

#include <QPixmap>
#include <QString>
#include <QLabel>
#include <QDir>

#include <QFileSystemWatcher>

class QUMonty {
public:
	~QUMonty();

	enum Status {
		normal = 0,
		seated = 1,
		back = 2,
		happy = 3
	};

	static QUMonty* instance();
	void initMessages();

	QPixmap pic(QUMonty::Status status = QUMonty::normal);
	QString welcomeMsg();

	void talk(QLabel *montyLbl, QLabel *msgLbl);
	void answer(QLabel *montyLbl, QLabel *msgLbl, const QString &question = QString(), bool prev = false);

	void addUnsupportedTag(const QString &tag) { _unsupportedTags.append(tag); }
	const QStringList& unsupportedTags() { _unsupportedTags.removeDuplicates(); return _unsupportedTags; }

	bool autoSaveEnabled() const;

	QString useImageFromResource(const QString &item, QDir dest);

	QFileSystemWatcher* watcher() const { return _watcher; }

protected:
	QUMonty();

private:
	static QUMonty* _instance;

	QStringList messages;
	QStringList _unsupportedTags; // keep track of all unsupported tags found to far

	QFileSystemWatcher *_watcher;
};

#endif /*QUMONTY_H_*/

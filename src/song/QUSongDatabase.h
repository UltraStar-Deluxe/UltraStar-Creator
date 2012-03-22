#ifndef QUSONGDATABASE_H
#define QUSONGDATABASE_H

#define songDB (QUSongDatabase::instance())

#include "QU.h"
#include "QUSongFile.h"

#include <QObject>
#include <QWidget>
#include <QList>

class QUSongDatabase: public QObject {
	Q_OBJECT

public:
	void setParentWidget(QWidget *w) { _parent = w; }
	QWidget* parentWidget() const { return _parent; }

	void addSong(QUSongFile *song);
	void deleteSong(QUSongFile *song);
	void reload();
	void clear();
	void swapSongWithFriend(QUSongFile *song, const QString &filename);

	const QList<QUSongFile*>& songs() const { return _songs; }
	int songCount() const;
	int songCountWithoutFriends() const { return _songs.size(); }
	int songFriendsCount() const;

	void ignoreChangesForSong(QUSongFile *song);
	void processChangesForSong(QUSongFile *song);

protected slots:
	void signalSongChanged();

signals:
	void songAdded(QUSongFile*);
	void songDeleted(QUSongFile*); // DO NOT de-reference this pointer!
	void songChanged(QUSongFile*);
	void databaseCleared();
	void databaseReloaded();
	void songWithFriendSwapped(QUSongFile *oldSong, QUSongFile *oldFriend);

protected:
	void readSongDir(QList<QDir> &dirList);

private:
	QList<QUSongFile*> _songs;
	QWidget		   *_parent; // used for dialogs like progress bars

/*!
 * Use the singleton pattern because we need only one
 * database at a time.
 */
public:
	static QUSongDatabase* instance();
protected:
	QUSongDatabase(QObject *parent = 0);
private:
	static QUSongDatabase *_instance;
};

#endif // QUSONGDATABASE_H

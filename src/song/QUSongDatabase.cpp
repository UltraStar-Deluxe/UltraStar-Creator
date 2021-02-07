#include "QUSongDatabase.h"
#include "QULogService.h"
#include "QUSongSupport.h"
#include "QUStringSupport.h"
#include "QUMonty.h"

#include <QDir>
#include <QFileInfo>
#include <QString>

QUSongDatabase::QUSongDatabase(QObject *parent): QObject(parent) {}

QUSongDatabase* QUSongDatabase::_instance = 0;
QUSongDatabase* QUSongDatabase::instance() {
	if(_instance == 0)
		_instance = new QUSongDatabase(0);

	return _instance;
}

void QUSongDatabase::signalSongChanged() {
	emit songChanged(qobject_cast<QUSongFile*>(sender()));
}

void QUSongDatabase::addSong(QUSongFile *song) {
	_songs.append(song);
	connect(song, SIGNAL(dataChanged()), this, SLOT(signalSongChanged()));
	emit songAdded(song);
}

/*!
 * Deletes the given song (the whole directory). Song item has to be removed
 * from song tree before you call this function!
 */
void QUSongDatabase::deleteSong(QUSongFile *song) {
	QDir dir(song->songFileInfo().dir());
	QString artist = song->artist();
	QString title = song->title();

	QFileInfoList fiList = dir.entryInfoList(QStringList("*.*"), QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDir::Name);

	foreach(QFileInfo fi, fiList) {
		if(!QFile::remove(fi.filePath()))
			logSrv->add(QString(tr("Could NOT delete file: \"%1\"")).arg(fi.filePath()), QU::Warning);
		else
			logSrv->add(QString(tr("File was deleted successfully: \"%1\"")).arg(fi.filePath()), QU::Information);
	}

	QString dirName = dir.dirName();
	dir.cdUp();

	if(!dir.rmdir(dirName))
		logSrv->add(QString(tr("Could NOT delete directory: \"%1\". Maybe it is not empty.")).arg(QDir::toNativeSeparators(song->songFileInfo().path())), QU::Warning);
	else
		logSrv->add(QString(tr("Directory was deleted successfully: \"%1\"")).arg(QDir::toNativeSeparators(song->songFileInfo().path())), QU::Information);

	_songs.removeAll(song);

	emit songDeleted(song);
	foreach(QUSongFile *friendSong, song->friends())
		emit songDeleted(friendSong);

	delete song;

	logSrv->add(QString(tr("Song was deleted successfully: \"%1 - %2\"")).arg(artist, title), QU::Information);
}

void QUSongDatabase::clear() {
	emit databaseCleared();
	qDeleteAll(_songs);
	_songs.clear();
}

void QUSongDatabase::swapSongWithFriend(QUSongFile *song, const QString &filename) {
	if(!_songs.contains(song))
		return;

	QUSongFile *friendSong = song->friendAt(filename);
	if(friendSong && song->swapWithFriend(friendSong)) {
		_songs.replace(_songs.indexOf(song), friendSong);
		emit songWithFriendSwapped(song, friendSong);
	}
}

/*!
 * Scan the given directory and create song file objects. Database will be cleared first.
 */
void QUSongDatabase::reload() {
	clear();

	QList<QDir> dirList;
	dirList.append(QDir(QU::BaseDir));

	// create a list of all sub-directories
	readSongDir(dirList);

	foreach(QDir dir, dirList) {
		QFileInfoList songFiList = dir.entryInfoList(QUSongSupport::allowedSongFiles(), QDir::Files, QDir::Name);
		//MB: commented out as it prefers duets to normal song files
		//qStableSort(songFiList.begin(), songFiList.end(), QU::fileTypeLessThan);

		QUSongFile *newSong = 0;

		// preference for text that already equals the folder name without folder tags
		foreach(QFileInfo fi, songFiList) {
			if(fi.completeBaseName() ==  QUStringSupport::withoutFolderTags(dir.dirName()))
				songFiList.move(songFiList.indexOf(fi), 0);
		}

		foreach(QFileInfo fi, songFiList) {
			if( QUSongSupport::allowedLicenseFiles().contains(fi.fileName(), Qt::CaseInsensitive) )
				continue; // skip license files (txt)

			if(!newSong)
				newSong = new QUSongFile(fi.filePath());
			else { // found a friend!
				QUSongFile *friendSong = new QUSongFile(fi.filePath());
				newSong->addFriend(friendSong);
				// see "this->addSong(...)"
				connect(friendSong, SIGNAL(dataChanged()), this, SLOT(signalSongChanged()));
			}
		}

		if(newSong) // some song found (not friend)
			addSong(newSong);
	}

	emit databaseReloaded();
}

int QUSongDatabase::songCount() const {
	return songFriendsCount() + _songs.size();
}

int QUSongDatabase::songFriendsCount() const {
	int result = 0;
	foreach(QUSongFile *song, _songs)
		result += song->friends().size();
	return result;
}

/*!
 * Do not pass song changes to the ouside. This could be useful if many changes
 * are made to a song und unnessecary updates should be skipped.
 */
void QUSongDatabase::ignoreChangesForSong(QUSongFile *song) {
	foreach(QUSongFile *friendSong, song->friends())
		disconnect(friendSong, SIGNAL(dataChanged()), this, SLOT(signalSongChanged()));
	disconnect(song, SIGNAL(dataChanged()), this, SLOT(signalSongChanged()));
}

void QUSongDatabase::processChangesForSong(QUSongFile *song) {
	foreach(QUSongFile *friendSong, song->friends())
		connect(friendSong, SIGNAL(dataChanged()), this, SLOT(signalSongChanged()));
	connect(song, SIGNAL(dataChanged()), this, SLOT(signalSongChanged()));
}

/*!
 * Reads recursively all available directories and puts them in the dirList.
 */
void QUSongDatabase::readSongDir(QList<QDir> &dirList) {
	if(dirList.isEmpty())
		return;

	if(dirList.last().entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks, QDir::Name).isEmpty()) {
		return;
	} else {
		QDir thisDir(dirList.last());
		QStringList subDirs = dirList.last().entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::NoSymLinks, QDir::Name);

		foreach(QString dir, subDirs) {
			QDir newDir(thisDir);
			newDir.cd(dir);

			dirList.append(newDir);
			this->readSongDir(dirList);
		}
	}
}

#include "QUSongFile.h"
#include "QULogService.h"

#include <QByteArray>
#include <QVariant>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QFileInfoList>
#include <QApplication>
#include <QFile>
#include <QChar>
#include <QMap>

#include "audioproperties.h"
#include "fileref.h"
#include "tag.h"
#include "tstring.h"

#include "math.h"

#include "QUMetaphoneString.h"
#include "QUStringSupport.h"
#include "QUSongSupport.h"

/*!
 * Creates a new song file object.
 * \param file an existing US song file (normally a *.txt)
 * \param parent parent for Qt object tree
 *
 * \sa allowedSongFiles()
 */
QUSongFile::QUSongFile(const QString &filePath, QObject *parent):
	QUSongInterface(parent),
	_hasUnsavedChanges(false),
	_songLengthCacheValid(false),
	_songSpeedCacheValid(false)
{
	_fiTags = QUStringSupport::extractTags(QFileInfo(filePath).fileName()); // save []-tags of song file name to recognize karaoke and duet songs later

	this->setFile(filePath);
}

QUSongFile::~QUSongFile() {
	qDeleteAll(_friends);
	_friends.clear();

	disconnect(this, 0, 0, 0);
}

/*!
 * This is a wrapper to allow external plugins access to the log service.
 */
void QUSongFile::log(const QString &message, int type) {
	logSrv->add(message, (QU::MessageTypes) type);
}

/*!
 * Here you can set up a new file that will be used for this song.
 */
void QUSongFile::setFile(const QString &filePath, bool update) {
	// setup new file
	_fi.setFile(filePath);
	if(update)
		updateCache();
}

/*!
 * Checks whether this is a valid UltraStar song file.
 */
bool QUSongFile::isValid() {
	return (length() > 0) and (artist() != N_A) and (title() != N_A);
}

/* SORTING FUNCTIONS BEGIN */

bool QUSongFile::artistLessThan (QUSongFile *s1, QUSongFile *s2)   { return QString::compare(s1->artist(), s2->artist(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::titleLessThan (QUSongFile *s1, QUSongFile *s2)    { return QString::compare(s1->title(), s2->title(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::languageLessThan (QUSongFile *s1, QUSongFile *s2) { return QString::compare(s1->language(), s2->language(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::editionLessThan (QUSongFile *s1, QUSongFile *s2)  { return QString::compare(s1->edition(), s2->edition(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::genreLessThan (QUSongFile *s1, QUSongFile *s2)    { return QString::compare(s1->genre(), s2->genre(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::yearLessThan (QUSongFile *s1, QUSongFile *s2)     { return QString::compare(s1->year(), s2->year(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::creatorLessThan (QUSongFile *s1, QUSongFile *s2)  { return QString::compare(s1->creator(), s2->creator(), Qt::CaseInsensitive) < 0; }

bool QUSongFile::pathLessThan (QUSongFile *s1, QUSongFile *s2)             { return QString::compare(s1->path(), s2->path(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::filePathLessThan (QUSongFile *s1, QUSongFile *s2)         { return QString::compare(s1->filePath(), s2->filePath(), Qt::CaseInsensitive) < 0; }
bool QUSongFile::relativeFilePathLessThan (QUSongFile *s1, QUSongFile *s2) { return QString::compare(s1->relativeFilePath(), s2->relativeFilePath(), Qt::CaseInsensitive) < 0; }

bool QUSongFile::hasMp3LessThan (QUSongFile *s1, QUSongFile *s2)         { return !s1->hasMp3() && s2->hasMp3(); }
bool QUSongFile::hasCoverLessThan (QUSongFile *s1, QUSongFile *s2)       { return !s1->hasCover() && s2->hasCover(); }
bool QUSongFile::hasBackgroundLessThan (QUSongFile *s1, QUSongFile *s2)  { return !s1->hasBackground() && s2->hasBackground(); }
bool QUSongFile::hasVideoLessThan (QUSongFile *s1, QUSongFile *s2)       { return !s1->hasVideo() && s2->hasVideo(); }

/* SORTING FUNCTIONS END */

/* COMPARING FUNCTINOS BEGIN */

/*!
 * Used to check whether two songs are nearly equal -> duplicate detection!
 *
 * Artist and title of the songs are being normalized (e.g. lower case, remove
 * special characters and fill-words, ...)
 *
 * soundex, metaphone
 *
 * remove any vocals
 *
 * QChar::decompositionTag()
 * QChar::decomposition()
 */
bool QUSongFile::equal(QUSongFile *s1, QUSongFile *s2) {
	return ( QUMetaphoneString::equal(s1->artist(), s2->artist()) and QUMetaphoneString::equal(s1->title(), s2->title(), true) );
}

/* COMPARING FUNCTIONS END */

/*!
 * \returns The relative file path of this song to the base dir.
 */
QString QUSongFile::relativeFilePath() const {
	return QU::BaseDir.relativeFilePath(_fi.filePath());
}

void QUSongFile::invalidateCaches() const {
	_songLengthCacheValid = false;
	_songSpeedCacheValid = false;
}

/*!
 * Reads the US data file and loads all data into memory. This is needed to be done
 * before any changes can be made. You cannot send event messages here because a song
 * is connected _after_ the first update of the internal cache.
 * \returns True on success, otherwise false.
 */
bool QUSongFile::updateCache() {
	QFile _file;
	_file.setFileName(_fi.filePath());

	if(!_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		logSrv->add(QString(tr("Could NOT open song file: \"%1\"")).arg(_fi.filePath()), QU::Warning);
		return false;
	}

	// clear contents
	_footer.clear();
	_lyrics.clear();
	_info.clear();
	_foundUnsupportedTags.clear();
	invalidateCaches();
	clearMelody();

	/*
	 * Read the header and write all tags into memory. The headers end is assumed
	 * when a line starts with ':', '*', 'F', 'E' or '-'. See the UltraStar documentation
	 * about information to these lines.
	 */
	QString line;
	while( !(QRegExp("([:\\*FE\\-].*)|(P [123].*)").exactMatch(line) || _file.atEnd()) ) {
		line = QUStringSupport::withoutLeadingBlanks(QString::fromLocal8Bit(_file.readLine()));

		// read supported tags
		bool isSupported = false;
		foreach(QString tag, QUSongSupport::availableTags()) {
			if(line.startsWith("#" + tag + ":", Qt::CaseInsensitive)) {
				setInfo(tag, line.section("#" + tag + ":", 0, 0, QString::SectionSkipEmpty | QString::SectionCaseInsensitiveSeps).trimmed());
				isSupported = true;
			}
		}

		// read unsupported tags
		if(!isSupported and line.startsWith("#")) {
			QString uTag(line.section(":", 0, 0).remove("#").trimmed());
			QString uValue(line.section("#" + uTag + ":", 0, 0, QString::SectionSkipEmpty).trimmed());

			if(!uTag.isEmpty() and !uValue.isEmpty()) {
				setInfo(uTag, uValue);
				_foundUnsupportedTags << uTag;
			}
		}
	}

	// read lyrics + other stuff (distinct them)
	while( !_file.atEnd() ) {
		if(QRegExp("([:\\*F\\-].*)|(P [123].*)").exactMatch(line))
			_lyrics << line;
		else if(QString::compare(line.trimmed(), "E", Qt::CaseInsensitive) != 0 && !line.isEmpty())
			_footer << line;

		line = QUStringSupport::withoutLeadingBlanks(QString::fromLocal8Bit(_file.readLine()));
	}

	// use last line buffer
	// TODO: a little bit dirty here (duplicate code)
	if(QRegExp("[:\\*F\\-].*").exactMatch(line))
		_lyrics << line;
	else if(QString::compare(line.trimmed(), "E", Qt::CaseInsensitive) != 0 && !line.isEmpty())
		_footer << line;

	_file.close();

	return true;
}

/*!
 * \param tag The tag you want to set/overwrite.
 * \param value The desired value for the tag.
 * \sa tags()
 */
void QUSongFile::setInfo(const QString &tag, const QString &value) {
	if(value == "") {
		_info.take(tag.toUpper());
	} else {
		_info[tag.toUpper()] = value.trimmed();
	}

	emit dataChanged();
	emit dataChanged(tag, value);
}

/*!
 * Checks whether the configuration information about the
 * tag order is correct (especially the count of the tags).
 */
void QUSongFile::verifyTags(QStringList &tags) {
	QSettings settings;

	if(tags.size() != QUSongSupport::availableTags().size()) {
		settings.setValue("tagOrder", QVariant(QUSongSupport::availableTags()));

		logSrv->add(tr("Deprecated tag information detected."), QU::Information);
		logSrv->add(tr("The number of available tags in your configuration and that one this application offers are different."), QU::Information);
		logSrv->add(tr("The tag order was reset to its default order. Check out the options to set up your custom order again."), QU::Information);

		tags = QUSongSupport::availableTags();
	}
}

/*!
 * Checks whether the mp3 file really exists and can be used by US.
 * \returns True if the mp3 specified by the song file really exists.
 */
bool QUSongFile::hasMp3() const {
	return !mp3().isEmpty() && mp3FileInfo().exists();
}

/*!
 * Checks whether the cover picture file really exists and can be used by US.
 * \returns True if the cover specified by the song file really exists.
 */
bool QUSongFile::hasCover() const {
	return !cover().isEmpty() && coverFileInfo().exists();
}

/*!
 * Checks whether the background picture file really exists and can be used by US.
 * \returns True if the background specified by the song file really exists.
 */
bool QUSongFile::hasBackground() const {
	return !background().isEmpty() && backgroundFileInfo().exists();
}

/*!
 * Checks whether the video file really exists and can be used by US.
 * \returns True if the video specified by the song file really exists.
 */
bool QUSongFile::hasVideo() const {
	return !video().isEmpty() && videoFileInfo().exists();
}

/*!
 * Checks whether the song file is "checked" (SC).
 */
bool QUSongFile::isSongChecked() const {
	return edition().contains("[SC]", Qt::CaseInsensitive);
}

bool QUSongFile::isSingStar() const {
	return edition().contains("singstar", Qt::CaseInsensitive);
}

/*!
 * Try to decide if this song is a duet. This feature will be new in USdx 1.1.
 * Actually there are three possibilities:
 *
 * > song file extension is 'txd'
 * > song has a friend with extension 'txt' and []-tag for the role
 * > song is based on special XML format which tells its type (maybe later)
 */
bool QUSongFile::isDuet() const {
	if( QString::compare(songFileInfo().suffix(), "txd", Qt::CaseInsensitive) == 0 )
		return true;

	QRegExp rxTag("kar(aoke)?", Qt::CaseInsensitive);
	foreach(QString fiTag, _fiTags) { // look for a []-tag that could be the singer name
		if(!rxTag.exactMatch(fiTag))
			return true;
	}

	return false;
}

bool QUSongFile::isKaraoke() const {
	QRegExp rxTag("kar(aoke)?", Qt::CaseInsensitive);

	foreach(QString fiTag, _fiTags) { // look for a []-tag that could be the singer name
		if(fiTag.contains(rxTag))
			return true;
	}

	return QUStringSupport::extractTags(title()).join(" ").contains(rxTag) || edition().contains(rxTag);
}

bool QUSongFile::isRelative() const {
	return QString::compare(relative(), "yes", Qt::CaseInsensitive) == 0;
}

/*!
 * \returns The #TITLE-tag without []-tags
 */
QString QUSongFile::titleCompact() const {
	return QUStringSupport::withoutFolderTags(title());
}

/*!
 * \returns the length of this song accoring to BPM and length of lyrics, in seconds
 */
int QUSongFile::length() const {
	if(!_songLengthCacheValid) {
		_songLengthCache = calculateSongLength();
		_songLengthCacheValid = true;
	}

	return _songLengthCache;
}

int QUSongFile::calculateSongLength() const {
	double bpm = QVariant(this->bpm().replace(",", ".")).toDouble();

	if(_lyrics.isEmpty() || bpm == 0.0)
		return 0;

	int beats = 0;
	if(isRelative()) {
		foreach(QString line, _lyrics) {
			QString l = line; l.insert(1, " "); // fix "-33 22"
			if(l.startsWith("-"))
				beats += QVariant(l.section(" ", 2, 2, QString::SectionSkipEmpty)).toInt(); // e.g. "- 48 64"
		}
	} else {
		QString l = _lyrics.last(); l.insert(1, " "); // fix "-44"
		beats = QVariant(l.section(" ", 1, 1, QString::SectionSkipEmpty)).toInt(); // use the number in the last line, e.g. ": 2000 5 60 boo", "- 2000"
	}

	double gap = QVariant(this->gap().replace(",", ".")).toDouble() / 1000;

	return qMax(0, (int)((beats / (bpm * 4)) * 60 + gap)); // result in seconds
}

/*!
 * \returns the length of the audio file, if one exists. That length can be reset by the #END tag.
 */
int QUSongFile::lengthMp3() const {
	TagLib::FileRef ref(this->mp3FileInfo().absoluteFilePath().toLocal8Bit().data());

	if(ref.isNull() or !ref.audioProperties())
		return 0;

	if(this->end() != N_A)
		return qMax(
				0,
				qMin(
						(int)(QVariant(this->end()).toDouble() / 1000),
						ref.audioProperties()->length()) );
	else
		return ref.audioProperties()->length();
}

int QUSongFile::lengthEffective() const {
	return qMax(0, this->lengthMp3() - QVariant(this->start()).toInt());
}

/*!
 * \returns the length of the audio file, if one exists. #END tag will be ignored.
 */
int QUSongFile::lengthAudioFile() const {
	TagLib::FileRef ref(this->mp3FileInfo().absoluteFilePath().toLocal8Bit().data());

	if(ref.isNull() or !ref.audioProperties())
		return 0;

	return ref.audioProperties()->length();
}

/*!
 * Like lengthEffective() but with formatted output.
 */
QString QUSongFile::lengthEffectiveFormatted() const {
	int l = lengthEffective();
	return QString("%1:%2").arg(l / 60).arg(l % 60, 2, 10, QChar('0'));
}

/*!
 * Like syllablesPerSecond() but with formatted output.
 */
QString QUSongFile::speedFormatted() const {
	double s = syllablesPerSecond(false);
	return QString("%1").arg(s, 2, 'f', 1, QChar('0'));
}

/*!
 * Try to give a useful answer to the hardness of a song. For this, several parts
 * are analysed and the fastest one will be returned.
 *
 * This is a little bit better than an average value of the whole song because some
 * tricky ones (e.g. Nightwish) got a long slow end but a very fast start or middle
 * part.
 *
 * I hope it will be useful. (saiya_mg)
 *
 * \returns the number of singable syllables per second (no freestyle or pauses)
 */
double QUSongFile::syllablesPerSecond(bool bypassCache) const {
	if(!_songSpeedCacheValid) {
		if(bypassCache)
			return -1.0;

		_songSpeedCache = calculateSongSpeed();
		_songSpeedCacheValid = true;
	}

	return _songSpeedCache;
}

double QUSongFile::calculateSongSpeed() const {
	double bpm = QVariant(this->bpm().replace(",", ".")).toDouble();

	if(_lyrics.isEmpty() || bpm == 0.0)
		return 0.0;

	QList<int> durations;

	foreach(QString line, _lyrics) {
		if(line.trimmed().startsWith(":") or line.trimmed().startsWith("*")) { // only singable notes (normal, golden)
			QStringList syllable = line.split(" ", QString::SkipEmptyParts);

			// singable syllables have a duration as second number
			if(syllable.size() < 3)
				continue;

			durations.append(QVariant(syllable.at(2)).toInt()); // should be the duration of the syllable
		}
	}

	if(durations.isEmpty())
		return 0.0;

	int beatCount1 = 0;
	int beatCount2 = 0;
	int beatCount3 = 0;
	for(int i = 0; i < durations.size() / 2; i++) {
		beatCount1 += durations.at(i);
		beatCount2 += durations.at(i + durations.size() / 4);
		beatCount3 += durations.at(qMin(i + durations.size() / 2, durations.size())); // qMin to avoid an access violation
	}

	double result1 = ((durations.size() / 2.) / (beatCount1 / (bpm * 4))) / 60;
	double result2 = ((durations.size() / 2.) / (beatCount2 / (bpm * 4))) / 60;
	double result3 = ((durations.size() / 2.) / (beatCount3 / (bpm * 4))) / 60;

	return qMax(result1, qMax(result2, result3));
}

/*!
 * \returns the song lyrics in a human readable format
 */
QStringList QUSongFile::lyrics() const {
	QStringList _lyricsCopy = _lyrics;
	QStringList result; result << QString();

	QRegExp lineBreak("\\s*-.*");
	QRegExp lineSinger("\\s*P [123].*");
	QRegExp linePrefix("\\s*[:\\*F]\\s*-?\\d+\\s+\\d+\\s+-?\\d+\\s");

	int lastBeat = -1;

	foreach(QString line, _lyricsCopy) {
		if(lineSinger.exactMatch(line))
			continue;

		if(lineBreak.exactMatch(line)) {
			result << QString();

			line = line.remove("-").trimmed();
			lastBeat = QVariant(line.section(" ", 0, 0, QString::SectionSkipEmpty)).toInt();

			// insert an empty line?
			if(isRelative()) {
				int nextBeat = QVariant(line.section(" ", 1, 1, QString::SectionSkipEmpty)).toInt();
				if(nextBeat - lastBeat > 20)
					result << QString();
				lastBeat = -1; // reset beat counter
			}
		} else {
			if(lastBeat != -1) {
				// insert an empty line?
				QString lineCopy = line;
				lineCopy.remove(QRegExp("[:\\*F]"));
				int nextBeat = QVariant(lineCopy.section(" ", 0, 0, QString::SectionSkipEmpty)).toInt();

				if(nextBeat - lastBeat > 20)
					result << QString();
				lastBeat = -1; // reset beat counter
			}

			line.remove("\n");

			if(QRegExp("\\s*\\*.*").exactMatch(line)) // golden note
				result.last() += line.remove(linePrefix).append("</b>").prepend("<b>");
			else if(QRegExp("\\s*F.*").exactMatch(line)) // freestyle note
				result.last() += line.remove(linePrefix).append("</i>").prepend("<i>");
			else
				result.last() += line.remove(linePrefix);
		}
	}

	return result;
}

/*!
 * Creates a complete new song file for US. Any old data will be overwritten.
 * \param force Indicates whether to save the file although automatic file save was
 * disabled.
 * \returns True on success, otherwise false.
 */
bool QUSongFile::save(bool force) {
	foreach(QUSongFile *song, _friends)
		song->save(force);

	if(!force) {
		_hasUnsavedChanges = true;
		return true;
	}

	QFile _file;
	_file.setFileName(_fi.filePath());

	if(!_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		logSrv->add(QString(tr("Save error! The song file \"%1\" was NOT saved.")).arg(this->songFileInfo().fileName()), QU::Warning);
		return false;
	}

	QStringList tags;
	QSettings settings;
	tags = settings.value("tagOrder", QUSongSupport::availableTags()).toStringList();

	QUSongFile::verifyTags(tags);

	// write supported tags
	foreach(QString tag, tags) {
		if(_info.value(tag.toUpper()) != "") { // do not write empty tags
			_file.write("#");
			_file.write(tag.toUpper().toLocal8Bit());
			_file.write(":");
			_file.write(_info.value(tag.toUpper()).toLocal8Bit());
			_file.write("\n");
		}
	}

	// write unsupported tags
	foreach(QString uTag, _foundUnsupportedTags) {
		_file.write("#");
		_file.write(uTag.toLocal8Bit());
		_file.write(":");
		_file.write(_info.value(uTag).toLocal8Bit());
		_file.write("\n");
	}

	// write lyrics
	foreach(QString line, _lyrics) {
		_file.write(line.toLocal8Bit());
	}

	// write song ending
	_file.write("E\n");

	// write footer
	foreach(QString line, _footer) {
		_file.write(line.toLocal8Bit());
	}

	_file.close();

	logSrv->add(QString(tr("The song file \"%1\" was saved successfully.")).arg(this->songFileInfo().fileName()), QU::Saving);
	_hasUnsavedChanges = false;

	return true;
}

/*!
 * Rename a file or a directory. Try to enable case-sensitive renaming under
 * Windows by inserting an additional step with a preceding "_".
 */
bool QUSongFile::rename(QDir dir, const QString &oldName, const QString &newName) {
	bool result = true;

	if(QString::compare(oldName, newName.trimmed(), Qt::CaseSensitive) == 0) {
		logSrv->add(QString(tr("Old name and new name match: \"%1\"")).arg(oldName), QU::Warning);
		return false;
	}

	if(oldName.length() == newName.trimmed().length()) {
		dir.rename(oldName, "_" + oldName);
		result = dir.rename("_" + oldName, newName.trimmed());
	} else {
		result = dir.rename(oldName, newName.trimmed());
	}

	return result;
}

/*!
 * Try to rename the directory of the current song file.
 * \param newName the new name (avoid illegal characters for your OS)
 */
void QUSongFile::renameSongDir(const QString &newName) {
	QDir dir(_fi.dir());
	dir.cdUp();

	QString oldName(_fi.dir().dirName());

	if(!rename(dir, oldName, newName)) {
		logSrv->add(QString(tr("Could NOT rename the song directory \"%1\" to \"%2\".")).arg(oldName).arg(newName), QU::Warning);
		return;
	}

	dir.cd(newName);
	setFile(dir.absoluteFilePath(_fi.fileName()), false);
	emit songPathChanged(_fi.filePath());

	logSrv->add(QString(tr("Song directory renamed from: \"%1\" to: \"%2\".")).arg(oldName).arg(newName), QU::Information);
}

/*!
 * Try to rename the song file itself.
 * \param newName the new name (avoid illegal characters for your OS)
 */
void QUSongFile::renameSongTxt(const QString &newName) {
	QString oldName(_fi.fileName());

	if(!rename(_fi.dir(), oldName, newName)) {
		logSrv->add(QString(tr("Could NOT rename the song file \"%1\" to \"%2\".")).arg(oldName).arg(newName), QU::Warning);
		return;
	}

	setFile(_fi.dir().absoluteFilePath(newName), false);
	emit songRenamed(newName);

	logSrv->add(QString(tr("Song file renamed from: \"%1\" to: \"%2\".")).arg(oldName).arg(newName), QU::Information);
}

/*!
 * Try to rename the mp3 file of the current song file. The information about the
 * current mp3 file has to be correct or the file cannot be found and renamed.
 * \param newName the new name (avoid illegal characters for your OS)
 */
void QUSongFile::renameSongMp3(const QString &newName) {
	QString oldName(mp3());

	if(!rename(_fi.dir(), oldName, newName)) {
		logSrv->add(QString(tr("Could NOT rename the audio file \"%1\" to \"%2\".")).arg(oldName).arg(newName), QU::Warning);
		return;
	}

	setInfo(MP3_TAG, newName);

	logSrv->add(QString(tr("Audio file renamed from: \"%1\" to: \"%2\".")).arg(oldName).arg(newName), QU::Information);
}

/*!
 * Try to rename the cover picture file of the current song file. The information about the
 * current cover has to be correct or the file cannot be found and renamed.
 * \param newName the new name (avoid illegal characters for your OS)
 */
void QUSongFile::renameSongCover(const QString &newName) {
	QString oldName(cover());

	if(!rename(_fi.dir(), oldName, newName)) {
		logSrv->add(QString(tr("Could NOT rename the cover picture \"%1\" to \"%2\".")).arg(oldName).arg(newName), QU::Warning);
		return;
	}

	setInfo(COVER_TAG, newName);

	logSrv->add(QString(tr("Cover picture renamed from: \"%1\" to: \"%2\".")).arg(oldName).arg(newName), QU::Information);
}

/*!
 * Try to rename the background picture file of the current song file. The information about the
 * current background has to be correct or the file cannot be found and renamed.
 * \param newName the new name (avoid illegal characters for your OS)
 */
void QUSongFile::renameSongBackground(const QString &newName) {
	QString oldName(background());

	if(!rename(_fi.dir(), oldName, newName)) {
		logSrv->add(QString(tr("Could NOT rename the background picture \"%1\" to \"%2\".")).arg(oldName).arg(newName), QU::Warning);
		return;
	}

	setInfo(BACKGROUND_TAG, newName);

	logSrv->add(QString(tr("Background picture renamed from: \"%1\" to: \"%2\".")).arg(oldName).arg(newName), QU::Information);
}

/*!
 * Try to rename the video file of the current song file. The information about the
 * current video has to be correct or the file cannot be found and renamed.
 * \param newName the new name (avoid illegal characters for your OS)
 */
void QUSongFile::renameSongVideo(const QString &newName) {
	QString oldName(video());

	if(!rename(_fi.dir(), oldName, newName)) {
		logSrv->add(QString(tr("Could NOT rename the video file \"%1\" to \"%2\".")).arg(oldName).arg(newName), QU::Warning);
		return;
	}

	setInfo(VIDEO_TAG, newName);

	logSrv->add(QString(tr("Video file renamed from: \"%1\" to: \"%2\".")).arg(oldName).arg(newName), QU::Information);
}

/*!
 * Reads the ID3 tag from the specified MP3-File (info has to be correct) and uses
 * artist and title information for the song file.
 */
void QUSongFile::useID3TagForArtist() {
	if(!hasMp3()) {
		logSrv->add(QString(tr("The song \"%1 - %2\" has no audio file assigned. Cannot use ID3 tag for artist.")).arg(this->artist()).arg(this->title()), QU::Warning);
		return;
	}

	// TOFIX: Possible App Crash!!! Check NULL Pointer!
	TagLib::FileRef ref(mp3FileInfo().absoluteFilePath().toLocal8Bit().data());

	QString oldArtist(this->artist());
	QString newArtist(TStringToQString(ref.tag()->artist()));

	if(newArtist.isEmpty()) {
		logSrv->add(QString(tr("The audio file %1 does not contain ID3 tag information about an artist.")).arg(this->mp3()), QU::Warning);
		return;
	}

	setInfo(ARTIST_TAG, newArtist);
	logSrv->add(QString(tr("ID3 tag of \"%1\" used for artist. Changed from: \"%2\" to: \"%3\".")).arg(this->mp3()).arg(oldArtist).arg(this->artist()), QU::Information);
}

void QUSongFile::useID3TagForTitle() {
	if(!hasMp3()) {
		logSrv->add(QString(tr("The song \"%1 - %2\" has no audio file assigned. Cannot use ID3 tag for title.")).arg(this->artist()).arg(this->title()), QU::Warning);
		return;
	}

	TagLib::FileRef ref(mp3FileInfo().absoluteFilePath().toLocal8Bit().data());

	QString oldTitle(this->title());
	QString newTitle(TStringToQString(ref.tag()->title()));

	if(newTitle.isEmpty()) {
		logSrv->add(QString(tr("The audio file %1 does not contain ID3 tag information about a title.")).arg(this->mp3()), QU::Warning);
		return;
	}

	setInfo(TITLE_TAG, newTitle);
	logSrv->add(QString(tr("ID3 tag of \"%1\" used for title. Changed from: \"%2\" to: \"%3\".")).arg(this->mp3()).arg(oldTitle).arg(this->title()), QU::Information);
}

void QUSongFile::useID3TagForGenre() {
	if(!hasMp3()) {
		logSrv->add(QString(tr("The song \"%1 - %2\" has no audio file assigned. Cannot use ID3 tag for genre.")).arg(this->artist()).arg(this->title()), QU::Warning);
		return;
	}

	TagLib::FileRef ref(mp3FileInfo().absoluteFilePath().toLocal8Bit().data());

	QString oldGenre(this->genre());
	QString newGenre(TStringToQString(ref.tag()->genre()));

	if(newGenre.isEmpty()) {
		logSrv->add(QString(tr("The audio file %1 does not contain ID3 tag information about a genre.")).arg(this->mp3()), QU::Warning);
		return;
	}

	setInfo(GENRE_TAG, newGenre);
	logSrv->add(QString(tr("ID3 tag of \"%1\" used for genre. Changed from: \"%2\" to: \"%3\".")).arg(this->mp3()).arg(oldGenre).arg(this->genre()), QU::Information);
}

void QUSongFile::useID3TagForYear() {
	if(!hasMp3()) {
		logSrv->add(QString(tr("The song \"%1 - %2\" has no audio file assigned. Cannot use ID3 tag for year.")).arg(this->artist()).arg(this->title()), QU::Warning);
		return;
	}

	TagLib::FileRef ref(mp3FileInfo().absoluteFilePath().toLocal8Bit().data());

	QString oldYear(this->year());
	QString newYear(QVariant(ref.tag()->year()).toString());

	if(newYear == "0") {
		logSrv->add(QString(tr("The audio file %1 does not contain ID3 tag information about a year.")).arg(this->mp3()), QU::Warning);
		return;
	}

	setInfo(YEAR_TAG, newYear);
	logSrv->add(QString(tr("ID3 tag of \"%1\" used for year. Changed from: \"%2\" to: \"%3\".")).arg(this->mp3()).arg(oldYear).arg(this->year()), QU::Information);
}

void QUSongFile::removeUnsupportedTags(const QStringList &filter, bool useFilter) {
	if(!this->unsupportedTagsFound()) {
		logSrv->add(QString(tr("The song \"%1 - %2\" has no unsupported tags.")).arg(this->artist()).arg(this->title()), QU::Information);
		return;
	}

	foreach(QString uTag, _foundUnsupportedTags) {
		if(!useFilter || filter.contains(uTag)) {
			_info.remove(uTag);
			_foundUnsupportedTags.removeAll(uTag);
			logSrv->add(QString(tr("Unsupported tag removed: #%1.")).arg(uTag), QU::Information);
		} else {
			logSrv->add(QString(tr("Unsupported tag skipped: #%1.")).arg(uTag), QU::Information);
		}
	}
}

/*!
 * Uses the filename of the given QFileInfo object to assign it to a proper tag.
 * \param file That file will be used for auto-set.
 * \param force Use the file although the present tag is correct/valid.
 */
void QUSongFile::autoSetFile(const QFileInfo &fi, bool force, const QString &coverPattern, const QString &backgroundPattern) {
	QString fileScheme("*." + fi.suffix());

	if(QUSongSupport::allowedAudioFiles().contains(fileScheme, Qt::CaseInsensitive)) {
		if(!this->hasMp3() || force) {
			this->setInfo(MP3_TAG, fi.fileName());
			logSrv->add(QString("Assigned \"%1\" as audio file for \"%2 - %3\".").arg(mp3()).arg(artist()).arg(title()), QU::Information);
		}
	} else if(QUSongSupport::allowedVideoFiles().contains(fileScheme, Qt::CaseInsensitive)) {
		if(!this->hasVideo() || force) {
			this->setInfo(VIDEO_TAG, fi.fileName());
			logSrv->add(QString(tr("Assigned \"%1\" as video file for \"%2 - %3\".")).arg(video()).arg(artist()).arg(title()), QU::Information);
		}
	} else if(QUSongSupport::allowedPictureFiles().contains(fileScheme, Qt::CaseInsensitive)) {
		QRegExp reCover(coverPattern, Qt::CaseInsensitive);
		QRegExp reBackground(backgroundPattern, Qt::CaseInsensitive);

		if(fi.fileName().contains(reCover) && (!this->hasCover() || force) ) {
			this->setInfo(COVER_TAG, fi.fileName());
			logSrv->add(QString(tr("Assigned \"%1\" as cover picture for \"%2 - %3\".")).arg(cover()).arg(artist()).arg(title()), QU::Information);
		} else if(fi.fileName().contains(reBackground) && (!this->hasBackground() || force) ) {
			this->setInfo(BACKGROUND_TAG, fi.fileName());
			logSrv->add(QString(tr("Assigned \"%1\" as background picture for \"%2 - %3\".")).arg(background()).arg(artist()).arg(title()), QU::Information);
		}
	}
}

/*!
 * Copy a file to the song folder and force its usage for a proper
 * tag. (e.g. blubb.mp3 will be used as #MP3).
 * \sa autoSetFiles()
 */
void QUSongFile::useExternalFile(const QString &filePath) {
	QFileInfo source(filePath);
	QFileInfo destination(this->songFileInfo().dir(), source.fileName());

	if(!QFile::copy(source.filePath(), destination.filePath())) {
		logSrv->add(QString(tr("Could not copy the file \"%1\" to \"%2\".")).arg(source.fileName()).arg(destination.path()), QU::Warning);
		return;
	}

	logSrv->add(QString(tr("The file \"%1\" was successfully copied to \"%2\".")).arg(source.fileName()).arg(destination.path()), QU::Saving);
	this->autoSetFile(destination, true);
}

/*!
 * Deletes any file in the song directory which is not used by the song itself or its friends.
 */
void QUSongFile::deleteUnusedFiles(const QStringList &filter, const QString &pattern, bool usePattern) {
	QFileInfoList fiList = _fi.dir().entryInfoList(QDir::Files, QDir::Name);

	QStringList usedFileNames;
	usedFileNames << mp3();
	usedFileNames << cover();
	usedFileNames << background();
	usedFileNames << video();
	usedFileNames << songFileInfo().fileName();

	foreach(QUSongFile *song, friends()) {
		usedFileNames << song->mp3();
		usedFileNames << song->cover();
		usedFileNames << song->background();
		usedFileNames << song->video();
		usedFileNames << song->songFileInfo().fileName();
	}

	foreach(QFileInfo fi, fiList) {
		if(!filter.contains("*." + fi.suffix()))
			if(!usePattern || !fi.fileName().contains(QRegExp(pattern)))
				continue;

		if(!usedFileNames.contains(fi.fileName(), Qt::CaseInsensitive))
			if(QFile::remove(fi.filePath())) {
				logSrv->add(QString(tr("File removed successfully: \"%1\".")).arg(fi.filePath()), QU::Information);
			} else {
				logSrv->add(QString(tr("File COULD NOT be removed: \"%1\".")).arg(fi.filePath()), QU::Warning);
			}
	}
}

void QUSongFile::clearInvalidFileTags() {
	if( (this->mp3() != N_A || this->start() != N_A || this->end() != N_A) && !this->hasMp3()) {
		this->setInfo(MP3_TAG, "");
		this->setInfo(START_TAG, "");
		this->setInfo(END_TAG, "");
		logSrv->add(QString(tr("Audio file tag removed for \"%1 - %2\".")).arg(this->artist()).arg(this->title()), QU::Information);
	}
	if(this->cover() != N_A && !this->hasCover()) {
		this->setInfo(COVER_TAG, "");
		logSrv->add(QString(tr("Cover tag removed for \"%1 - %2\".")).arg(this->artist()).arg(this->title()), QU::Information);
	}
	if(this->background() != N_A && !this->hasBackground()) {
		this->setInfo(BACKGROUND_TAG, "");
		logSrv->add(QString(tr("Background tag removed for \"%1 - %2\".")).arg(this->artist()).arg(this->title()), QU::Information);
	}
	if( (this->video() != N_A || this->videogap() != N_A) && !this->hasVideo()) {
		this->setInfo(VIDEO_TAG, "");
		this->setInfo(VIDEOGAP_TAG, "");
		logSrv->add(QString(tr("Video tag removed for \"%1 - %2\".")).arg(this->artist()).arg(this->title()), QU::Information);
	}
}

/*!
 * Move all files in a song directory to a new location. This includes also files which
 * are not currently used by the song itself.
 * \param newRelativePath new relative path for the song; is relative to the UltraStar song folder
 */
void QUSongFile::moveAllFiles(const QString &newRelativePath) {
	QString source = QU::BaseDir.relativeFilePath(_fi.path());
	QString destination = newRelativePath;

	if(QString::compare(source, destination, Qt::CaseInsensitive) == 0) {
		logSrv->add(QString(tr("Old path and new path match! Cannot change song path to: \"%1\"")).arg(destination), QU::Warning);
		return;
	}

	if(!QU::BaseDir.mkpath(newRelativePath)) {
		logSrv->add(QString(tr("Could not create new song path: \"%1\"")).arg(newRelativePath), QU::Warning);
		return;
	}

	// move files to new location
	bool allFilesCopied = true;
	foreach(QFileInfo fi, _fi.dir().entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot)) {
		QString from = fi.filePath();
		QString to = QFileInfo(QU::BaseDir, destination + "/" + fi.fileName()).filePath();

		if(!QFile::rename(from, to)) {
			logSrv->add(QString(tr("Failed to move \"%1\" to \"%2\".")).arg(from).arg(to), QU::Warning);
			allFilesCopied = false;
		} else
			logSrv->add(QString(tr("The file \"%1\" was successfully moved to \"%2\".")).arg(from).arg(to), QU::Saving);
	}

	if(!allFilesCopied) {
		logSrv->add(QString(tr("Could NOT move all files of the song \"%2\" to a new location. Check out \"%1\" for the files which were copied.")).arg(newRelativePath).arg(QString("%1 - %2").arg(artist()).arg(title())), QU::Warning);
		return;
	}

	// remove empty folders
	QDir oldDir = _fi.dir();
	while(!oldDir.isRoot() and oldDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).isEmpty() and oldDir != QU::BaseDir) {
		QString dirName = oldDir.dirName();
		oldDir.cdUp();
		if(!oldDir.rmdir(dirName))
			logSrv->add(QString(tr("Could not remove old, empty folder \"%1\". Please do it manually.")).arg(dirName), QU::Warning);
	}

	// change internal song location
	setFile(QFileInfo(QU::BaseDir, destination + "/" + _fi.fileName()).filePath(), false);
	emit songPathChanged(_fi.filePath());

	logSrv->add(QString(tr("Location of song \"%1 - %2\" successfully changed to \"%3\" in your UltraStar song folder.")).arg(artist()).arg(title()).arg(newRelativePath), QU::Information);
}

/*!
 * Uses the #END tag to close the gap between length() and lengthMp3(). But only if an
 * audio file is present and longer than the song.
 *
 * A little gap is appended.
 */
void QUSongFile::fixAudioLength(int buffer) {
	if(!hasMp3()) {
		logSrv->add(QString(tr("Could not fix audio length because no audio file is present: \"%1 - %2\"")).arg(artist()).arg(title()), QU::Warning);
		return;
	}

	// remove present #END value
	setInfo(END_TAG, "");

	int length = this->length();
	int lengthMp3 = this->lengthMp3(); // length of audio file because #END was set to 0

	if(length > lengthMp3) {
		logSrv->add(QString(tr("Could not fix audio length because audio file is shorter than song: \"%1 - %2\"")).arg(artist()).arg(title()), QU::Warning);
		return;
	}

	int end = length + buffer; // calculate the new #END value

	if(end > lengthMp3) {
		logSrv->add(QString(tr("Could not fix audio length because new value for #END would be greater than length of audio file: \"%1 - %2\"")).arg(artist()).arg(title()), QU::Warning);
		return;
	}

	// set new value for #END, in milliseconds
	setInfo(END_TAG, QVariant(end * 1000).toString());

	logSrv->add(QString(tr("Audio length was fixed for song \"%1 - %2\". #END changed to: %3")).arg(artist()).arg(title()).arg(end * 1000), QU::Information);
}

/*!
 * It is not useful to have high precision in the #GAP value because the value
 * is in milliseconds.
 *
 * That's why this method rounds the value.
 */
void QUSongFile::roundGap() {
	double gap = QVariant(this->gap().replace(",", ".")).toDouble();
	QString oldGap = this->gap();

	this->setInfo(GAP_TAG, QVariant(qRound(gap)).toString());

	logSrv->add(QString(tr("#GAP changed from %1 to %2 for \"%3 - %4\".")).arg(oldGap).arg(this->gap()).arg(artist()).arg(title()), QU::Information);
}

/*!
 * Clear the #END tag.
 */
void QUSongFile::removeEndTag() {
	if(end() != N_A) {
		setInfo(END_TAG, "");
		logSrv->add(QString(tr("The tag #END was removed for: \"%1 - %2\"")).arg(artist()).arg(title()), QU::Information);
	}
}

/*!
 * React to external change of the loaded song file.
 */
void QUSongFile::songFileChanged(const QString &filePath) {
	if(_fi.filePath() != filePath)
		return;

	if(hasUnsavedChanges()) {
		logSrv->add(QString("INCONSISTENT STATE! The song \"%1 - %2\" has unsaved changes and its persistent song file \"%3\" was modified externally. Save your changes or rebuild the tree manually.").arg(artist()).arg(title()).arg(songFileInfo().filePath()), QU::Warning);
		return;
	}

	updateCache();

	logSrv->add(QString("Song file changed: \"%1\"").arg(songFileInfo().filePath()), QU::Information);
	emit dataChanged();
}

/*!
 * Load the melody of a song to enable convenient modification
 * of the song's lyrics.
 *
 * You need to save any changes with QUSongFile::saveMelody().
 */
QList<QUSongLineInterface*>& QUSongFile::loadMelody() {
	if(_melody.isEmpty())
		convertLyricsFromRaw();
	return _melody;
}

void QUSongFile::clearMelody() {
	qDeleteAll(_melody);
	_melody.clear();
}

/*!
 * Persist any changes in the melody to internal raw data format which
 * will then be saved to the file.
 */
void QUSongFile::saveMelody() {
	convertLyricsToRaw();
}

/*!
 * \returns Lyrics/Melody for the given singer of this song.
 */
QList<QUSongLineInterface*> QUSongFile::melodyForSinger(QUSongLineInterface::Singers s) {
	QList<QUSongLineInterface*> result;
	foreach(QUSongLineInterface *line, loadMelody())
		if(line->singer() == s || line->singer() == QUSongLineInterface::both)
			result << line;
	return result;
}

/*!
 * Set up all connections to inform the song friend in case of changed data.
 */
void QUSongFile::addFriend(QUSongFile *song) {
	_friends << song;
	connect(this, SIGNAL(dataChanged(QString,QString)), _friends.last(), SLOT(changeData(QString,QString)));
	connect(this, SIGNAL(songRenamed(QString)), _friends.last(), SLOT(renameSong(QString)));
	connect(this, SIGNAL(songPathChanged(QString)), _friends.last(), SLOT(changeSongPath(QString)));
}

bool QUSongFile::isFriend(const QFileInfo &fi) {
	foreach(QUSongFile *song, _friends) {
		if(song->songFileInfo() == fi)
			return true;
	}

	return false;
}

/*!
 * Assume that the friend is in the same directory. (should always be?)
 */
bool QUSongFile::isFriend(const QString &fileName) {
	return isFriend(QFileInfo(songFileInfo().dir(), fileName));
}

bool QUSongFile::isFriend(QUSongFile *song) {
	foreach(QUSongFile *fsong, _friends) {
		if(fsong == song)
			return true;
	}

	return false;
}

QUSongFile* QUSongFile::friendAt(const QFileInfo &fi) {
	foreach(QUSongFile *song, _friends) {
		if(song->songFileInfo() == fi)
			return song;
	}

	return 0;
}

QUSongFile* QUSongFile::friendAt(const QString &fileName) {
	return friendAt(QFileInfo(songFileInfo().dir(), fileName));
}

/*!
 * React if your friend was changed.
 */
void QUSongFile::changeData(const QString &tag, const QString &value) {
	QUSongFile *senderSong = qobject_cast<QUSongFile*>(sender());

	// restore all []-tags for the title
	if(QString::compare(tag, TITLE_TAG, Qt::CaseInsensitive) == 0) {
		setInfo(tag, QString("%1%2")
				.arg(QUStringSupport::withoutFolderTags(value)) // ignore other []-tags
				.arg(_fiTags.isEmpty() ? "" : _fiTags.join("] [").prepend(" [").append("]"))); // just use your own []-tags
	} else if(QString::compare(tag, MP3_TAG, Qt::CaseInsensitive) == 0
			  && (isKaraoke() || (senderSong && senderSong->isKaraoke()))) {
		; // karaoke songs use their own audio files - do nothing
	} else if(QString::compare(tag, GAP_TAG, Qt::CaseInsensitive) != 0
			  && QString::compare(tag, EDITION_TAG, Qt::CaseInsensitive) != 0) {
		// share all values except: #GAP, #EDITION
		setInfo(tag, value);
	}
}

/*!
 * Rename yourself if your friend was renamed.
 */
void QUSongFile::renameSong(const QString &fileName) {
	renameSongTxt(QString("%1 %2.%3")
		.arg(QUStringSupport::withoutFolderTags(QFileInfo(songFileInfo().dir(), fileName).baseName()))
		.arg(_fiTags.join("] [").prepend("[").append("]"))
		.arg(songFileInfo().suffix()));
}

/*!
 * Change your path if your friend did.
 */
void QUSongFile::changeSongPath(const QString &filePath) {
	setFile(QFileInfo(QFileInfo(filePath).dir(), songFileInfo().fileName()).filePath(), false);
}

/*!
 * Find the friend that is your duet partner to support duets that a spread across two song files.
 */
QUSongFile* QUSongFile::duetFriend() const {
	if(!isDuet())
		return 0;

	foreach(QUSongFile *song, _friends)
		if(song->isDuet())
			return song;

	return 0;
}

/*!
 * \returns True, if a friend has the same tag value as requested.
 */
bool QUSongFile::friendHasTag(const QString &tag, const QString &value) const {
	foreach(QUSongFile *song, _friends)
		if(QString::compare(song->customTag(tag), value, Qt::CaseSensitive) == 0)
			return true;
	return false;
}

/*!
 * Prepare the given song friend to be used as a primary song.
 *
 * \returns True, if the change completed successfully.
 */
bool QUSongFile::swapWithFriend(QUSongFile *song) {
	if(!isFriend(song))
		return false; // only friends can be used here

	foreach(QUSongFile *friendSong, _friends) {
		disconnect(this, SIGNAL(dataChanged(QString,QString)), friendSong, SLOT(changeData(QString,QString)));
		disconnect(this, SIGNAL(songRenamed(QString)), friendSong, SLOT(renameSong(QString)));
		disconnect(this, SIGNAL(songPathChanged(QString)), friendSong, SLOT(changeSongPath(QString)));

		if(friendSong != song)
			song->addFriend(friendSong);
	}

	song->addFriend(this);
	_friends.clear();

	logSrv->add(tr("Primary song file changed from \"%1\" to \"%2\".")
				.arg(songFileInfo().fileName())
				.arg(song->songFileInfo().fileName()), QU::Information);

	return true;
}

/*!
 * Coverts the raw lyrics to another format for further processing.
 */
void QUSongFile::convertLyricsFromRaw() {
	if(!_melody.isEmpty())
		return;

	foreach(QString line, _lyrics) {
		lyricsAddNote(line);
	}
}

void QUSongFile::convertLyricsToRaw() {
	if(_melody.isEmpty())
		return;

	_lyrics.clear();

	foreach(QUSongLineInterface *line, _melody) {
		if(line->singer() != QUSongLineInterface::undefined)
			_lyrics.append(QString("P %1\n").arg((int)line->singer()));

		foreach(QUSongNoteInterface *note, line->notes()) {
			QStringList out;
				 if(note->type() == QUSongNoteInterface::freestyle) out.append("F");
			else if(note->type() == QUSongNoteInterface::golden)    out.append("*");
			else                                                    out.append(":");

			out.append(QVariant(note->timestamp()).toString());
			out.append(QVariant(note->duration()).toString());
			out.append(QVariant(note->pitch()).toString());

			out.append(note->syllable());

			_lyrics.append(QString("%1\n").arg(out.join(" ")));
		}

		if(line->useOutTime()) {
			QStringList out;
			out << "-" << QVariant(line->outTime()).toString();
			if(line->useInTime())
				out << QVariant(line->inTime()).toString();

			_lyrics.append(QString("%1\n").arg(out.join(" ")));
		}
	}
}

/*!
 * Takes a raw lyrics line (e.g. ": 2345 10 90 blubb ") and converts that to an internal format.
 */
void QUSongFile::lyricsAddNote(QString line) {
	if(!QRegExp("([:\\*F\\-].*)|(P [123].*)").exactMatch(line)) {
		logSrv->add(QString(tr("Error while preparing lyrics for %1 - %2. Could not parse the following line: %3"))
					.arg(artist())
					.arg(title())
					.arg(line.trimmed()), QU::Warning);
		return; // no valid line
	}

	if(_melody.isEmpty()) {
		_melody.append(new QUSongLine(this));
	}

	QUSongLineInterface *songLine = _melody.last();

	if(line.startsWith("-")) { // line break
		line.remove(0, 1);
		QStringList lineSplit = line.trimmed().split(" ", QString::SkipEmptyParts);

		if(lineSplit.size() > 0) songLine->setOutTime(QVariant(lineSplit.at(0).trimmed()).toInt());
		if(lineSplit.size() > 1) songLine->setInTime(QVariant(lineSplit.at(1).trimmed()).toInt());

		_melody.append(new QUSongLine(this));
	} else if(line.startsWith("P", Qt::CaseInsensitive)) {
		// singer information found
		line.insert(1, " "); // to avoid: "P2"
		songLine->setSinger((QUSongLineInterface::Singers)QVariant(line.split(" ", QString::SkipEmptyParts).at(1).trimmed()).toInt());
	} else {
		line.insert(1, " "); // to avoid: ":22 33 90 blubb"
		line.remove("\n");

		QStringList lineSplit = line.trimmed().split(" ", QString::SkipEmptyParts);

		if(lineSplit.size() < 4)
			logSrv->add(QString(tr("Line too short: \"%1\"")).arg(line), QU::Warning);
		else {
			QUSongNoteInterface::Types t = line.startsWith("F") ? QUSongNoteInterface::freestyle : (line.startsWith("*") ? QUSongNoteInterface::golden : QUSongNoteInterface::normal);

			// extract the lyric/syllable
			line.remove(QRegExp("[:\\*F]\\s*\\-?\\d+\\s+\\d+\\s+\\-?\\d+\\s"));
			line.remove(QRegExp("[:\\*F]\\s*\\-?\\d+\\s+\\d+\\s+\\-?\\d+")); // if no syllable is present

			QUSongNote *note = new QUSongNote(
					t,
					QVariant(lineSplit.at(1).trimmed()).toInt(),
					QVariant(lineSplit.at(2).trimmed()).toInt(),
					QVariant(lineSplit.at(3).trimmed()).toInt(),
					line);

			songLine->addNote(note);
		}
	}
}

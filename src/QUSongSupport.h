#ifndef QUSONGSUPPORT_H_
#define QUSONGSUPPORT_H_

#include <QObject>
#include <QStringList>

class QUSongSupport: QObject {
    Q_OBJECT
    
public:
    QUSongSupport(QObject *parent = 0);

    static QStringList availableTags();
	static QStringList availableCustomTags();
	static QStringList availableNoteTypes();

	static QStringList allowedSongFiles(); // duet too
	static QStringList allowedLicenseFiles();
	static QStringList allowedMidiFiles();
	static QStringList allowedKaraokeFiles(); // MIDI + Lyrics
	static QStringList allowedScoreFiles();

	static QStringList allowedAudioFiles();
	static QStringList allowedPictureFiles();
	static QStringList allowedVideoFiles();
	static QStringList allowedPlaylistFiles();

private:
	static QStringList registryKey(const QString &key, const QString &defaultValue);

};

#endif // QUSTRINGSUPPORT_H_

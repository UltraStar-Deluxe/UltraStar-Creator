#include "QU.h"
#include "QUSongSupport.h"
#include "QUSongInterface.h"

#include <QMap>
#include <QSettings>

QUSongSupport::QUSongSupport(QObject *parent): QObject(parent) {}

/*!
 * \returns A list of strings with all available tags. Custom tags are included.
 */
QStringList QUSongSupport::availableTags() {
	QStringList result;

	result << ENCODING_TAG;
	result << TITLE_TAG;
	result << ARTIST_TAG;
	result << LANGUAGE_TAG;
	result << EDITION_TAG;
	result << GENRE_TAG;
	result << YEAR_TAG;
	result << CREATOR_TAG;
	result << MP3_TAG;
	result << COVER_TAG;
	result << BACKGROUND_TAG;
	result << VIDEO_TAG;
	result << VIDEOGAP_TAG;
	result << START_TAG;
	result << END_TAG;
	result << RELATIVE_TAG;
	result << PREVIEWSTART_TAG;
	result << CALCMEDLEY_TAG;
	result << MEDLEYSTARTBEAT_TAG;
	result << MEDLEYENDBEAT_TAG;
	result << BPM_TAG;
	result << GAP_TAG;

	// all custom tags that will be supported
	result << availableCustomTags();

	return result;
}

QStringList QUSongSupport::availableCustomTags() {
	QSettings settings;
	return settings.value("customTags").toString().split(" ", QString::SkipEmptyParts);
}

QStringList QUSongSupport::availableNoteTypes() {
	QStringList result;

	result << ":"; // normal
	result << "*"; // golden note
	result << "F"; // freestyle
	result << "-"; // pause

	return result;
}

QStringList QUSongSupport::allowedSongFiles() {
	return registryKey("allowedSongFiles", "*.txt *.txd");
}

QStringList QUSongSupport::allowedLicenseFiles() {
	return registryKey("allowedLicenseFiles", "license.txt license.html license.htm");
}

QStringList QUSongSupport::allowedMidiFiles() {
	return registryKey("allowedMidiFiles", "*.mid *.midi");
}

QStringList QUSongSupport::allowedKaraokeFiles() {
	return registryKey("allowedKaraokeFiles", "*.kar");
}

QStringList QUSongSupport::allowedScoreFiles() {
	return registryKey("allowedScoreFiles", "*.sco");
}

QStringList QUSongSupport::allowedAudioFiles() {
	return registryKey("allowedAudioFiles", "*.mp3 *.ogg *.avi *.divx *.flv *.m2v *.m4v *.mkv *.mov *.mp4 *.mpeg *.mpg *.ogm *.ts *.vob *.webm *.wmv");
}

QStringList QUSongSupport::allowedImageFiles() {
	return registryKey("allowedImageFiles", "*.jpg *.jpeg *.png");
}

QStringList QUSongSupport::allowedVideoFiles() {
	return registryKey("allowedVideoFiles", "*.avi *.divx *.flv *.m2v *.m4v *.mkv *.mov *.mp4 *.mpeg *.mpg *.ogm *.ts *.vob *.webm *.wmv");
}

QStringList QUSongSupport::allowedPlaylistFiles() {
	return registryKey("allowedPlaylistFiles", "*.upl");
}

QStringList QUSongSupport::allowedEncodingTypes() {
	QStringList result;

	result << "CP1252"; // windows-1252 (Western European)
	result << "CP1250"; // windows-1250 (Central/Eastern European)
	result << "UTF8";   // UTF-8

	return result;
}

QStringList QUSongSupport::availableSongLanguages() {
	QStringList result;

	result << "Arabic";
	result << "Chinese";
	result << "Croatian";
	result << "Czech";
	result << "Danish";
	result << "Dutch";
	result << "English";
	result << "Finnish";
	result << "French";
	result << "German";
	result << "Hindi";
	result << "Italian";
	result << "Japanese";
	result << "Korean";
	result << "Latin";
	result << "Norwegian";
	result << "Polish";
	result << "Portuguese";
	result << "Romanian";
	result << "Russian";
	result << "Slovak";
	result << "Slowenian";
	result << "Spanish";
	result << "Swedish";
	result << "Turkish";

	return result;
}

QStringList QUSongSupport::availableSongEditions() {
	QStringList result;

	result << "";

	return result;
}

QStringList QUSongSupport::availableSongGenres() {
	QStringList result;

	result << "A capella";
	result << "Acid";
	result << "Acid Jazz";
	result << "Acid Punk";
	result << "Acoustic";
	result << "AlternRock";
	result << "Alternative";
	result << "Ambient";
	result << "Avantgarde";
	result << "Ballad";
	result << "Bass";
	result << "Bebob";
	result << "Big Band";
	result << "Bluegrass";
	result << "Blues";
	result << "Booty Bass";
	result << "Cabaret";
	result << "Celtic";
	result << "Chamber Music";
	result << "Chanson";
	result << "Chorus";
	result << "Christian Rap";
	result << "Classic Rock";
	result << "Classical";
	result << "Club";
	result << "Comedy";
	result << "Country";
	result << "Cult";
	result << "Dance";
	result << "Dance Hall";
	result << "Darkwave";
	result << "Death Metal";
	result << "Disco";
	result << "Dream";
	result << "Duet";
	result << "Easy Listening";
	result << "Electronic";
	result << "Ethnic";
	result << "Euro-House";
	result << "Euro-Techno";
	result << "Eurodance";
	result << "Fast Fusion";
	result << "Folk";
	result << "Folk-Rock";
	result << "Folklore";
	result << "Freestyle";
	result << "Funk";
	result << "Fusion";
	result << "Game";
	result << "Gangsta";
	result << "Gospel";
	result << "Gothic";
	result << "Gothic Rock";
	result << "Grunge";
	result << "Hard Rock";
	result << "Hip-Hop";
	result << "House";
	result << "Humour";
	result << "Industrial";
	result << "Instrumental";
	result << "Instrumental Pop";
	result << "Instrumental Rock";
	result << "Jazz";
	result << "Jazz+Funk";
	result << "Jungle";
	result << "Latin";
	result << "Lo-Fi";
	result << "Meditative";
	result << "Metal";
	result << "Musical";
	result << "National Folk";
	result << "Native American";
	result << "New Age";
	result << "New Wave";
	result << "Noise";
	result << "Oldies";
	result << "Opera";
	result << "Other";
	result << "Polka";
	result << "Pop";
	result << "Pop-Folk";
	result << "Pop/Funk";
	result << "Porn Groove";
	result << "Power Ballad";
	result << "Pranks";
	result << "Primus";
	result << "Progressive Rock";
	result << "Psychadelic";
	result << "Psychedelic Rock";
	result << "Punk";
	result << "Punk Rock";
	result << "R&B";
	result << "Rap";
	result << "Rave";
	result << "Reggae";
	result << "Retro";
	result << "Revival";
	result << "Rhythmic Soul";
	result << "Rock";
	result << "Rock & Roll";
	result << "Samba";
	result << "Satire";
	result << "Showtunes";
	result << "Ska";
	result << "Slow Jam";
	result << "Slow Rock";
	result << "Sonata";
	result << "Soul";
	result << "Sound Clip";
	result << "Soundtrack";
	result << "Southern Rock";
	result << "Space";
	result << "Speech";
	result << "Swing";
	result << "Symphonic Rock";
	result << "Symphony";
	result << "Tango";
	result << "Techno";
	result << "Techno-Industrial";
	result << "Top";
	result << "Trailer";
	result << "Trance";
	result << "Tribal";
	result << "Trip-Hop";
	result << "Vocal";

	return result;
}

QString QUSongSupport::defaultInputEncoding() {
	return registryKey("defaultInputEncoding", "CP1252").first();
}

QString QUSongSupport::defaultOutputEncoding() {
	return registryKey("defaultOutputEncoding", "CP1252").first();
}

QStringList QUSongSupport::availableDefaultPitches() {
	QStringList result;

	result << "0 (C4)";
	result << QString("1 (C#4, Db4)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "2 (D4)";
	result << QString("3 (D#4, Eb4)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "4 (E4)";
	result << "5 (F4)";
	result << QString("6 (F#4, Gb4)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "7 (G4)";
	result << QString("8 (G#4, Ab4)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "9 (A4)";
	result << QString("10 (A#4, Bb4)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "11 (B4)";
	result << "12 (C5)";
	result << QString("13 (C#5, Db5)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "14 (D5)";
	result << QString("15 (D#5, Eb5)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "16 (E5)";
	result << "17 (F5)";
	result << QString("18 (F#5, Gb5)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "19 (G4)";
	result << QString("20 (G#5, Ab5)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "21 (A5)";
	result << QString("22 (A#5, Bb5)").replace('#',QString::fromUtf8(CHAR_UTF8_SHARP)).replace('b', QString::fromUtf8(CHAR_UTF8_FLAT));
	result << "23 (B5)";
	result << "24 (C6)";

	return result;
}

/*!
 * Looks for a value in the registry and sets the default, if key not present.
 */
QStringList QUSongSupport::registryKey(const QString &key, const QString &defaultValue) {
// MB: map-related stuff commented out because of update problems with defaultInput/OutputEncoding
//	static QMap<QString, QStringList> map;

//	if(map.contains(key))
//		return map.value(key);

	QSettings settings;
	if(!settings.contains(key))
		settings.setValue(key, defaultValue);

//	map.insert(key, settings.value(key, defaultValue).toString().split(" ", QString::SkipEmptyParts));

//	return map.value(key);
	return settings.value(key, defaultValue).toString().split(" ", QString::SkipEmptyParts);
}

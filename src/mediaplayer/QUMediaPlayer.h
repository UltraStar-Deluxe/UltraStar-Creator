#ifndef QUMEDIAPLAYER_H_
#define QUMEDIAPLAYER_H_

#include "QUSongFile.h"

#include <QObject>
#include <QTimer>

#include "bass.h"
#include "ui_QUMediaPlayer.h"

class QUSongInfo {
public:
	QUSongInfo(QUSongFile *song);
	bool isValid() const { return _isValid; }

	QList<QUSongLineInterface*> melody1;
	QList<QUSongLineInterface*> melody2;
	double bpm;
	double gap1;
	double gap2;
	bool isRelative;

	// properties from usdx song
	QString filePath;
	QString artist;
	QString title;
	int length; // from audio file, can be reset through #END tag
	int lengthAudio; // #END tag ignored

	// properties from id3tag
	QString album;
	int bitrate;
	int channels;
	int sampleRate;

private:
	bool _isValid;
};

class QUMediaPlayer: public QWidget, private Ui::QUMediaPlayer {
	Q_OBJECT

public:
	enum Source {
		NoChange = -1,
		SelectedSongs = 0,
		AllSongs = 1,
		VisibleSongs = 2,
		CurrentPlaylist = 3
	};
	QUMediaPlayer(QWidget *parent = 0);
	~QUMediaPlayer();

	void setSongs(const QList<QUSongFile*> &songs, Source src = NoChange);

	enum State {
		stopped,
		paused,
		playing
	};
	Q_DECLARE_FLAGS(States, State)

public slots:
	void play();
	void stop();
	void pause();
	void prev();
	void next();
	void resume();
	void seek();

	void requestSongs();

private slots:
	void updateTime();
	void updatePlayerControls(QUMediaPlayer::States state);
	void updateInfoLabel(QUMediaPlayer::States state);
	void updateTimeSlider(QUMediaPlayer::States state);

	void loopShuffleToggled();
	void requestSongEdit(int line);

signals:
	void selectedSongsRequested();
	void allSongsRequested();
	void visibleSongsRequested();
	void currentPlaylistRequested();
	void editSongRequested(QUSongFile*, int line);

	void stateChanged(QUMediaPlayer::States state);

private:
	HSTREAM _mediaStream;
	QList<QUSongInfo> _songs;
	int _currentSongIndex;
	QList<int> _lastIndices;
	QList<int> _freeIndices;

	QUMediaPlayer::States _state;
	void setState(QUMediaPlayer::States newState);
	QUMediaPlayer::States state() const { return _state; }

	void BASS_StopAndFree();
	void BASS_Play();
	void BASS_Pause();
	void BASS_Resume();
	double BASS_Position();
	void BASS_SetPosition(int seconds);
};

#endif /* QUMEDIAPLAYER_H_ */

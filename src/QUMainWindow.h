#ifndef QCMAINWINDOW_H
#define QCMAINWINDOW_H

#include "ui_QUMainWindow.h"

#include "taglib.h"
#include "fileref.h"

#include <QMainWindow>
#include <QTime>
#include <QProgressBar>
#include <QFileInfo>
#include <QMediaPlayer>

class QURibbonBar;

namespace Ui {
	class QUMainWindow;
}

class QUMainWindow: public QMainWindow, private Ui::QUMainWindow {
	Q_OBJECT

public:
	QUMainWindow(QWidget *parent = 0);

protected:
	virtual void closeEvent(QCloseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private slots:
	void initConfig();
	void initWindow();
	void initRibbonBar();
	void initEventLog();
	void initMonty();
	void initStatusBar();

	void enableEnglish();
	void enableSpanish();
	void enableGerman();
	void enableFrench();
	void enablePolish();
	void enablePortuguese();

	void generateFreestyleTextFiles();

	void montyTalk(bool force = false);
	void montyTalkNow();
	void montyAsk();
	void montyAnswer();
	void montyPrev();
	void montyNext();

	void on_pushButton_ShowWebSite_clicked();
	void on_doubleSpinBox_BPM_valueChanged(double BPMValue);
	void on_pushButton_SyllabificateTeX_clicked();
	void on_comboBox_Year_activated(QString );
	void on_actionHelp_triggered();
	void on_horizontalSlider_PreviewMP3_sliderMoved(int position);
	void on_pushButton_SyllabificateRules_clicked();
	void on_pushButton_startYass_clicked();
	void on_pushButton_PreviewPlayPause_clicked();
	void on_comboBox_Video_editTextChanged(QString video);
	void on_pushButton_startUltraStar_clicked();
	void on_horizontalSlider_MP3_sliderMoved(int position);
	void on_pushButton_UndoTap_clicked();
	void on_pushButton_Reset_clicked();
	void setPlaybackSpeed(int value);
	void on_plainTextEdit_InputLyrics_textChanged();
	void on_pushButton_OutputLyricsDecreaseFontSize_clicked();
	void on_pushButton_OutputLyricsIncreaseFontSize_clicked();
	void on_pushButton_InputLyricsDecreaseFontSize_clicked();
	void on_pushButton_InputLyricsIncreaseFontSize_clicked();
	void on_pushButton_BrowseLyrics_clicked();
	void on_comboBox_Genre_editTextChanged(QString );
	void on_lineEdit_Creator_textChanged(QString );
	void on_lineEdit_Edition_textChanged(QString );
	void on_comboBox_Language_currentIndexChanged(QString );
	void on_lineEdit_Title_textChanged(QString );
	void on_lineEdit_Artist_textChanged(QString );

	void on_actionQuit_USC_triggered();
	void on_pushButton_BrowseMP3_clicked();
	void on_pushButton_Stop_clicked();
	void on_pushButton_PasteFromClipboard_clicked();
	void on_pushButton_Tap_released();
	void on_pushButton_Tap_pressed();
	void on_pushButton_PlayPause_clicked();
	bool on_pushButton_SaveToFile_clicked();
	void updateTime();
	void updatePreviewTime();

	void aboutQt();
	void aboutUSC();
	void aboutTagLib();
	void checkForUpdate(bool silent);

	void on_comboBox_Cover_currentIndexChanged(const QString &cover);
	void on_comboBox_Background_currentIndexChanged(const QString &background);
	void on_comboBox_Video_currentIndexChanged(const QString &video);

private:
	QURibbonBar *_menu;
	Ui::QUMainWindow *ui;
	qint32 numSyllables;
	QString currentOutputTextLine;
	qint32 currentSyllableIndex;
	qint32 currentCharacterIndex;
	qint32 firstNoteStartBeat;
	double currentNoteStartTime;
	qint32 currentNoteStartBeat;
	qint32 currentNoteBeatLength;
	qint32 previousNoteEndBeat;
	QString lyricsString;
	QStringList lyricsStringList;
	QClipboard *clipboard;
	float BPMFromMP3;
	float BPM;
	float sampleRate;
	bool firstNote;
	bool isFirstKeyPress;
	QString cleanLyrics(QString);
	void splitLyricsIntoSyllables();
	void handleMP3();
	QFileInfo* fileInfo_MP3;
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	bool isVowel(QChar character, QString lang);
	bool isUmlaut(QChar character, QString lang);
	bool isDiphthong(QChar character1, QChar character2, QString lang);
	bool isConsonant(QChar character, QString lang);
	bool isInseparable(QChar character1, QChar character2, QString lang);
	bool isStrongVowel(QChar character, QString lang);
	bool isHiatus(QChar character1, QChar character2, QString lang);

	enum State {
		uninitialized,
		initialized,
		stopped,
		paused,
		playing
	} state, previewState;
	void updateSyllableButtons();
	QStringList lyricsSyllableList;
	void changeLanguage(QString);
	QProgressBar *lyricsProgressBar;
	double MP3LengthTime;
	QString defaultDir;
	QMap<double, QString> timeLineMap;
	void updateOutputLyrics();
	QMediaPlayer* _player;
};

#endif // QCMAINWINDOW_H

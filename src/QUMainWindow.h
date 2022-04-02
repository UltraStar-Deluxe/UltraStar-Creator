#ifndef QCMAINWINDOW_H
#define QCMAINWINDOW_H

#include "ui_QUMainWindow.h"
#include "bass.h"
#include "bass_fx.h"

#include "taglib.h"
#include "fileref.h"

#include <QMainWindow>
#include <QTime>
#include <QProgressBar>
#include <QFileInfo>

//class QURibbonBar;

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
	void initMenuBar();
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

	void on_doubleSpinBox_BPM_valueChanged(double BPMValue);
	void on_comboBox_Year_activated(QString );
	void on_actionHelp_triggered();
	void on_horizontalSlider_PreviewMP3_sliderMoved(int position);
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
	void on_pushButton_GetLyrics_clicked();
	void on_pushButton_BrowseSubtitles_clicked();
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
	void aboutUltraStarCreator();
	void aboutTagLib();
	void aboutBASS();
	void aboutCLD2();
	void checkForUpdate();

	void on_comboBox_Cover_currentIndexChanged(const QString &cover);
	void on_comboBox_Background_currentIndexChanged(const QString &background);
	void on_comboBox_Video_currentIndexChanged(const QString &video);
	
	void setDefaultPitch(int pitch);

private:
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
	bool addLinebreak = false;
	QString cleanLyrics(QString);
	void splitLyricsIntoSyllables();
	HSTREAM _mediaStream;
	void BASS_Stop();
	void BASS_Free();
	void BASS_StopAndFree();
	void BASS_Play();
	void BASS_Pause();
	void BASS_Resume();
	double BASS_Position();
	void BASS_SetPosition(double seconds);
	void handleMP3();
	QFileInfo* fileInfo_MP3;
	QFileInfo* fileInfo_SRT;
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	QString getResourcesPath();
	QString syllabifyLyrics(QString lyrics, QString language);
	void handleLyrics(QString lyrics);
	bool determineLanguage(QString lyrics);
	void cleanAndSyllabifyLyrics(QString lyrics);
	void getYear();
	
	QActionGroup *uiLanguageGroup;
	QAction *enableEnglishAction;
	QAction *enableSpanishAction;
	QAction *enableGermanAction;
	QAction *enableFrenchAction;
	QAction *enablePortugueseAction;
	QAction *enablePolishAction;
	QMenu *languageMenu;
	QAction *aboutUltraStarCreatorAction;
	QAction *aboutQtAction;
	QAction *aboutTagLibAction;
	QAction *aboutBASSAction;
	QAction *aboutCLD2Action;
	QAction *checkForUpdateAction;
	QMenu *aboutMenu;
	QAction *generateFreestyleTextFilesActions;
	QMenu *extrasMenu;
	QAction *showMontyAction;
	QMenu *helpMenu;

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
};

#endif // QCMAINWINDOW_H

#ifndef QCMAINWINDOW_H
#define QCMAINWINDOW_H

#include "ui_QCMainWindow.h"
#include "bass.h"
#include "bass_fx.h"

#include "taglib.h"
#include "audioproperties.h"
#include "fileref.h"
#include "tag.h"
#include "tstring.h"

#include <QMainWindow>
#include <QTime>
#include <QProgressBar>
#include <QFileInfo>

namespace Ui {
    class QCMainWindow;
}

class QCMainWindow: public QMainWindow, private Ui::QCMainWindow {
    Q_OBJECT

public:
    QCMainWindow(QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::QCMainWindow *ui;
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
    bool firstNote;
    bool isFirstKeyPress;
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
    void BASS_SetPosition(int seconds);
    void handleMP3();
    float playbackSpeedDecreasePercentage;
    QFileInfo* fileInfo_MP3;
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    enum State {
        uninitialized,
        initialized,
        stopped,
        paused,
        playing
    };
    State state;
    void updateSyllableButtons();
    QStringList lyricsSyllableList;
    void changeLanguage(QString);
    QProgressBar *lyricsProgressBar;
    double MP3LengthTime;

private slots:
    void on_pushButton_startUltraStar_clicked();
    void on_horizontalSlider_MP3_sliderMoved(int position);
    void on_spinBox_Year_valueChanged(QString );
    void on_pushButton_UndoTap_clicked();
    void on_lineEdit_Background_textChanged(QString );
    void on_lineEdit_Cover_textChanged(QString );
    void on_pushButton_Reset_clicked();
    void on_actionAbout_TagLib_triggered();
    void on_actionAbout_BASS_triggered();
    void on_horizontalSlider_PlaybackSpeed_valueChanged(int value);
    void on_actionSpanish_triggered();
    void on_actionPolish_triggered();
    void on_actionItalian_triggered();
    void on_actionGerman_triggered();
    void on_actionFrench_triggered();
    void on_actionEnglish_triggered();
    void on_plainTextEdit_InputLyrics_textChanged();
    void on_pushButton_OutputLyricsDecreaseFontSize_clicked();
    void on_pushButton_OutputLyricsIncreaseFontSize_clicked();
    void on_pushButton_InputLyricsDecreaseFontSize_clicked();
    void on_pushButton_InputLyricsIncreaseFontSize_clicked();
    void on_pushButton_BrowseLyrics_clicked();
    void on_comboBox_Genre_textChanged(QString );
    void on_lineEdit_Creator_textChanged(QString );
    void on_comboBox_Edition_textChanged(QString );
    void on_comboBox_Language_currentIndexChanged(QString );
    void on_lineEdit_Title_textChanged(QString );
    void on_lineEdit_Artist_textChanged(QString );
    void on_actionAbout_Qt_triggered();
    void on_actionQuit_USC_triggered();
    void on_actionAbout_triggered();
    void on_pushButton_BrowseCover_clicked();
    void on_pushButton_BrowseVideo_clicked();
    void on_pushButton_BrowseBackground_clicked();
    void on_pushButton_BrowseMP3_clicked();
    void on_pushButton_Stop_clicked();
    void on_pushButton_CopyToClipboard_clicked();
    void on_pushButton_PasteFromClipboard_clicked();
    void on_pushButton_Tap_released();
    void on_pushButton_Tap_pressed();
    void on_pushButton_PlayPause_clicked();
    bool on_pushButton_SaveToFile_clicked();
    void updateTime();
};

#endif // QCMAINWINDOW_H

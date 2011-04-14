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
    float BPM;
    float sampleRate;
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

private slots:
    void on_pushButton_ShowWebSite_clicked();
    void on_doubleSpinBox_BPM_valueChanged(double BPMValue);
    void on_pushButton_EnableBPMEdit_toggled(bool checked);
    void on_pushButton_SyllabificateTeX_clicked();
    void on_actionWhats_This_triggered();
    void on_comboBox_Year_activated(QString );
    void on_actionHelp_triggered();
    void on_actionCreate_Dummy_Songs_triggered();
    void on_horizontalSlider_PreviewMP3_sliderMoved(int position);
    void on_pushButton_SyllabificateRules_clicked();
    void on_pushButton_startYass_clicked();
    void on_pushButton_PreviewPlayPause_clicked();
    void on_lineEdit_Video_textChanged(QString );
    void on_pushButton_startUltraStar_clicked();
    void on_horizontalSlider_MP3_sliderMoved(int position);
    void on_pushButton_UndoTap_clicked();
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
    void on_lineEdit_Edition_textChanged(QString );
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
    void on_pushButton_PasteFromClipboard_clicked();
    void on_pushButton_Tap_released();
    void on_pushButton_Tap_pressed();
    void on_pushButton_PlayPause_clicked();
    bool on_pushButton_SaveToFile_clicked();
    void updateTime();
    void updatePreviewTime();
};

#endif // QCMAINWINDOW_H

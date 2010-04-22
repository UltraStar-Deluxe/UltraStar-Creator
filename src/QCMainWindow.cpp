#include "main.h"

#include "QCMainWindow.h"
#include "QULogService.h"
#include "QUMessageBox.h"
#include "QUAboutDialog.h"

#include <QFileDialog>
#include <QTextStream>
#include <QClipboard>
#include <QUrl>
#include <QSettings>
#include <QTimer>
#include <QTextCodec>

QCMainWindow::QCMainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::QCMainWindow) {

    ui->setupUi(this);
    setWindowTitle(tr("UltraStar Song Creator %1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION));
    logSrv->add(tr("Ready."), QU::Information);
    numSyllables = 0;
    firstNoteStartBeat = 0;
    currentSyllableIndex = 0;
    currentCharacterIndex = 0;
    firstNote = true;
    accumulatedPauseTime = 0;
    accumulatedPauseBeats = 0;
    clipboard = QApplication::clipboard();
    QMainWindow::statusBar()->showMessage(tr("USC ready."));
    if (BASS_Init(-1, 44100, 0, 0, NULL)) {
        QMainWindow::statusBar()->showMessage(tr("BASS initialized."));
    }
    State state = QCMainWindow::uninitialized;

    this->show();

    QUMessageBox::information(0,
        QObject::tr("Welcome to UltraStar Creator!"),
        QObject::tr("This tool enables you to <b>rapidly</b> create UltraStar text files <b>from scratch</b>.<br><br>The bare <b>minimum</b> of what you need is:<br>* a <b>song file</b> (MP3/OGG format)<br>* the song's <b>lyrics</b><br><br><b>Important information</b> such as BPM and GAP are determined <b>automatically</b>.<br><br><b>Additional meta information</b> is read from the <b>ID3 tags</b>, if available.<br><br>Any <b>further information</b> is <b>desirable</b>, but <b>not necessary</b> for song creation.<br><br><b>Happy creating!</b>"),
        BTN << ":/marks/accept.png" << QObject::tr("Okay. Let's go!"));
}

/*!
 * Overloaded to ensure that all changes are saved before closing this application.
 */
void QCMainWindow::closeEvent(QCloseEvent *event) {
        QSettings settings;

        settings.setValue("windowState", QVariant(this->saveState()));

        // everything sould be fine from now on
        QFile::remove("running.app");

        event->accept();
}

bool QCMainWindow::on_pushButton_SaveToFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Please choose file"), QDir::homePath(), tr("Text files (*.txt)"));

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QUMessageBox::warning(this, tr("Application"),
            tr("Cannot write file %1:\n%2.")
            .arg(fileName)
            .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << ui->plainTextEdit_OutputLyrics->toPlainText();
    QApplication::restoreOverrideCursor();
    return true;
}

void QCMainWindow::on_pushButton_PlayPause_clicked()
{
    if (state == initialized) {
        state = QCMainWindow::playing;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/pause.png"));
        ui->pushButton_PlayPause->setStatusTip(tr("Pause tapping."));
        QWidget::setAcceptDrops(false);
        //QMainWindow::statusBar()->showMessage(tr("USC Tapping."));
        ui->groupBox_SongMetaInformationTags->setDisabled(true);
        ui->groupBox_MP3ArtworkTags->setDisabled(true);
        ui->groupBox_MiscSettings->setDisabled(true);
        ui->groupBox_VideoTags->setDisabled(true);
        ui->groupBox_InputLyrics->setDisabled(true);
        ui->groupBox_OutputLyrics->setEnabled(true);
        ui->groupBox_TapArea->setEnabled(true);
        ui->pushButton_Tap->setEnabled(true);
        ui->pushButton_Stop->setEnabled(true);
        if (ui->lineEdit_Title->text().isEmpty()) {
            ui->lineEdit_Title->setText(tr("Title"));
            ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        }
        if (ui->lineEdit_Artist->text().isEmpty()) {
            ui->lineEdit_Artist->setText(tr("Artist"));
            ui->label_ArtistSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        }
        if (ui->lineEdit_Cover->text().isEmpty()) {
            ui->lineEdit_Cover->setText(tr("%1 - %2 [CO].jpg").arg(ui->lineEdit_Artist->text()).arg(ui->lineEdit_Title->text()));
        }
        if (ui->lineEdit_Background->text().isEmpty()) {
            ui->lineEdit_Background->setText(tr("%1 - %2 [BG].jpg").arg(ui->lineEdit_Artist->text()).arg(ui->lineEdit_Title->text()));
        }

        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#ENCODING:Auto"));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#TITLE:%1").arg(ui->lineEdit_Title->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#ARTIST:%1").arg(ui->lineEdit_Artist->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#LANGUAGE:%1").arg(ui->comboBox_Language->currentText()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#EDITION:%1").arg(ui->comboBox_Edition->currentText()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GENRE:%1").arg(ui->comboBox_Genre->currentText()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#YEAR:%1").arg(ui->comboBox_Year->currentText()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#CREATOR:%1").arg(ui->lineEdit_Creator->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#MP3:%1").arg(ui->lineEdit_MP3->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#COVER:%1").arg(ui->lineEdit_Cover->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#BACKGROUND:%1").arg(ui->lineEdit_Background->text()));

        if (ui->groupBox_VideoTags->isChecked()) {
            if (ui->lineEdit_Video->text().isEmpty()) {
                ui->lineEdit_Video->setText(tr("%1 - %2.avi").arg(ui->lineEdit_Artist->text()).arg(ui->lineEdit_Title->text()));
            }
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#VIDEO:%1").arg(ui->lineEdit_Video->text()));
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#VIDEOGAP:%1").arg(ui->doubleSpinBox_Videogap->text()));
        }
        ui->plainTextEdit_OutputLyrics->appendPlainText("#BPM:" + ui->doubleSpinBox_BPM->text());

        BPM = ui->doubleSpinBox_BPM->value();

        QString rawLyricsString = ui->plainTextEdit_InputLyrics->toPlainText();

        lyricsString = cleanLyrics(rawLyricsString);

        lyricsStringList = lyricsString.split(QRegExp("[ +\\n]"), QString::SkipEmptyParts);

        numSyllables = lyricsStringList.length();
        ui->progressBar_Lyrics->setMaximum(numSyllables);

    if (numSyllables > 5) {
            ui->pushButton_Tap->setText(lyricsStringList[currentSyllableIndex]);
            ui->pushButton_NextSyllable1->setText(lyricsStringList[currentSyllableIndex+1]);
            ui->pushButton_NextSyllable2->setText(lyricsStringList[currentSyllableIndex+2]);
            ui->pushButton_NextSyllable3->setText(lyricsStringList[currentSyllableIndex+3]);
            ui->pushButton_NextSyllable4->setText(lyricsStringList[currentSyllableIndex+4]);
            ui->pushButton_NextSyllable5->setText(lyricsStringList[currentSyllableIndex+5]);
        }

        playbackSpeedDecreasePercentage = 100 - ui->horizontalSlider_PlaybackSpeed->value();
        _mediaStream = BASS_FX_TempoCreate(_mediaStream, BASS_FX_FREESOURCE);
        bool result = BASS_ChannelSetAttribute(_mediaStream, BASS_ATTRIB_TEMPO, -playbackSpeedDecreasePercentage);
        if (result) {
            BASS_Play();
            updateTime();
        }

        ui->pushButton_Tap->setFocus(Qt::OtherFocusReason);

        songTimer.start();
    }
    else if (state == playing) {
        state = QCMainWindow::paused;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/play.png"));
        ui->pushButton_PlayPause->setStatusTip(tr("Continue tapping."));
        ui->pushButton_Tap->setDisabled(true);
        BASS_Pause();
        // pause songTimer
        pauseTimer.start();

    }
    else if (state == paused) {
        state = QCMainWindow::playing;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/pause.png"));
        ui->pushButton_PlayPause->setStatusTip(tr("Pause tapping."));
        accumulatedPauseTime += pauseTimer.elapsed();
        accumulatedPauseBeats = accumulatedPauseTime * (BPM / 15000);
        state = QCMainWindow::playing;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/pause.png"));
        ui->pushButton_Tap->setEnabled(true);
        BASS_Resume();
        // resume songTimer
    }
    else {
        // should not be possible
    }
}

QString QCMainWindow::cleanLyrics(QString rawLyricsString) {
    // delete surplus space characters
    rawLyricsString = rawLyricsString.replace(QRegExp(" {2,}"), " ");

    // delete leading and trailing whitespace
    rawLyricsString = rawLyricsString.trimmed();

    // delete surplus blank lines...
    rawLyricsString = rawLyricsString.replace(QRegExp("\\n{2,}"), "\n");

    // Capitalize each line
    // ...

    // replace misused accents (´,`) by the correct apostrophe (')
    rawLyricsString = rawLyricsString.replace("´", "'");
    rawLyricsString = rawLyricsString.replace("`", "'");

    QString cleanedLyricsString = rawLyricsString;

    ui->plainTextEdit_InputLyrics->setPlainText(cleanedLyricsString);

    return cleanedLyricsString;
}

void QCMainWindow::on_pushButton_Tap_pressed()
{
    noteTimer.start();
    //QMainWindow::statusBar()->showMessage(tr("USC Note Start."));
    currentNoteStartTime = songTimer.elapsed()*(1-playbackSpeedDecreasePercentage/100);
    // conversion from milliseconds [ms] to quarter beats [qb]: time [ms] * BPM [qb/min] * 1/60 [min/s] * 1/1000 [s/ms]
    currentNoteStartBeat = currentNoteStartTime * (BPM / 15000);
    ui->pushButton_Tap->setCursor(Qt::ClosedHandCursor);
}


void QCMainWindow::on_pushButton_Tap_released()
{
    qint32 currentNoteTimeLength = noteTimer.elapsed()*(1-playbackSpeedDecreasePercentage/100);
    ui->pushButton_Tap->setCursor(Qt::OpenHandCursor);
    //QMainWindow::statusBar()->showMessage(tr("USC Note End."));
    ui->progressBar_Lyrics->setValue(ui->progressBar_Lyrics->value()+1);
    qint32 currentNoteBeatLength = qMax(1.0, currentNoteTimeLength * (BPM / 15000));
    if (firstNote){
        firstNoteStartBeat = currentNoteStartBeat;
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GAP:%1").arg(QString::number(currentNoteStartTime)));
        ui->spinBox_Gap->setValue(currentNoteStartTime);
        ui->label_GapSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        firstNote = false;
    }

    int nextSeparatorIndex = lyricsString.mid(1).indexOf(QRegExp("[ +\\n]"));
    QString currentSyllable;
    bool addLinebreak = false;

    if (nextSeparatorIndex != -1) {
        QChar nextSeparator = lyricsString.at(nextSeparatorIndex+1);
        currentSyllable = lyricsString.mid(0,nextSeparatorIndex+1);

        if (currentSyllable.startsWith('+')) {
            currentSyllable = currentSyllable.mid(1);
        }

        if (nextSeparator == ' ') {
            lyricsString = lyricsString.mid(nextSeparatorIndex+1);
            addLinebreak = false;
        }
        else if (nextSeparator == '+') {
            lyricsString = lyricsString.mid(nextSeparatorIndex+2);
            addLinebreak = false;
        }
        else if (nextSeparator == '\n') {
            lyricsString = lyricsString.mid(nextSeparatorIndex+2);
            addLinebreak = true;
        }
    }
    else { // end of lyrics
        currentSyllable = lyricsString;
        addLinebreak = false;
    }

    currentOutputTextLine = tr(": %1 %2 %3 %4").arg(QString::number(currentNoteStartBeat - firstNoteStartBeat - accumulatedPauseBeats)).arg(QString::number(currentNoteBeatLength)).arg(ui->comboBox_DefaultPitch->currentIndex()).arg(currentSyllable);
    ui->plainTextEdit_OutputLyrics->appendPlainText(currentOutputTextLine);
    if (addLinebreak)
    {
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("- %1").arg(QString::number(currentNoteStartBeat - firstNoteStartBeat - accumulatedPauseBeats + currentNoteBeatLength + 1)));
    }


    if ((currentSyllableIndex+1) < numSyllables) {
        currentSyllableIndex++;

        ui->pushButton_Tap->setText(lyricsStringList[currentSyllableIndex]);
        if (currentSyllableIndex+1 < numSyllables) {
            ui->pushButton_NextSyllable1->setText(lyricsStringList[currentSyllableIndex+1]);
        }
        else {
            ui->pushButton_NextSyllable1->setText("");
        }
        if (currentSyllableIndex+2 < numSyllables) {
            ui->pushButton_NextSyllable2->setText(lyricsStringList[currentSyllableIndex+2]);
        }
        else {
            ui->pushButton_NextSyllable2->setText("");
        }
        if (currentSyllableIndex+3 < numSyllables) {
            ui->pushButton_NextSyllable3->setText(lyricsStringList[currentSyllableIndex+3]);
        }
        else {
            ui->pushButton_NextSyllable3->setText("");
        }
        if (currentSyllableIndex+4 < numSyllables) {
            ui->pushButton_NextSyllable4->setText(lyricsStringList[currentSyllableIndex+4]);
        }
        else {
            ui->pushButton_NextSyllable4->setText("");
        }
        if (currentSyllableIndex+5 < numSyllables) {
            ui->pushButton_NextSyllable5->setText(lyricsStringList[currentSyllableIndex+5]);
        }
        else {
            ui->pushButton_NextSyllable5->setText("");
        }
    }
    else {
        ui->pushButton_Tap->setText("");
        on_pushButton_Stop_clicked();
    }
}

void QCMainWindow::on_pushButton_PasteFromClipboard_clicked()
{
    ui->plainTextEdit_InputLyrics->setPlainText(clipboard->text());
}

void QCMainWindow::on_pushButton_CopyToClipboard_clicked()
{
    clipboard->setText(ui->plainTextEdit_OutputLyrics->toPlainText());
}

void QCMainWindow::on_pushButton_Stop_clicked()
{
    if (state == playing || state == paused) {
        state = QCMainWindow::stopped;
        QMainWindow::statusBar()->showMessage(tr("State: stopped."));
        BASS_StopAndFree();

        ui->plainTextEdit_OutputLyrics->appendPlainText("E");
        ui->plainTextEdit_OutputLyrics->appendPlainText("");

        QMainWindow::statusBar()->showMessage(tr("USC ready."));

        ui->pushButton_PlayPause->setIcon(QIcon(":/player/play.png"));
        ui->pushButton_PlayPause->setDisabled(true);
        ui->pushButton_Stop->setDisabled(true);
        ui->pushButton_Reset->setEnabled(true);
        ui->groupBox_TapArea->setDisabled(true);
        ui->pushButton_SaveToFile->setEnabled(true);
        ui->pushButton_CopyToClipboard->setEnabled(true);
        QWidget::setAcceptDrops(true);
    }
    else {
        // should not be possible
    }
}

void QCMainWindow::on_pushButton_BrowseMP3_clicked()
{
    filename_MP3 = QFileDialog::getOpenFileName ( 0, tr("Please choose MP3 file"), QDir::homePath(), tr("Audio files (*.mp3 *.ogg)"));
    handleMP3();
}

void QCMainWindow::on_pushButton_BrowseCover_clicked()
{
    QString filename_Cover = QFileDialog::getOpenFileName ( 0, tr("Please choose cover image file"), QDir::homePath(), tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Cover = new QFileInfo(filename_Cover);
    if (!fileInfo_Cover->fileName().isEmpty()) {
        ui->lineEdit_Cover->setText(fileInfo_Cover->fileName());
    }
}

void QCMainWindow::on_lineEdit_Cover_textChanged(QString cover)
{
    if (!cover.isEmpty()) {
        ui->label_CoverSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
}

void QCMainWindow::on_pushButton_BrowseBackground_clicked()
{
    QString filename_Background = QFileDialog::getOpenFileName ( 0, tr("Please choose background image file"), QDir::homePath(), tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Background = new QFileInfo(filename_Background);
    if (!fileInfo_Background->fileName().isEmpty()) {
        ui->lineEdit_Background->setText(fileInfo_Background->fileName());
    }
}

void QCMainWindow::on_lineEdit_Background_textChanged(QString background)
{
    if (!background.isEmpty()) {
        ui->label_BackgroundSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
}

void QCMainWindow::on_pushButton_BrowseVideo_clicked()
{
    QString filename_Video = QFileDialog::getOpenFileName ( 0, tr("Please choose video file"), QDir::homePath(), tr("Video files (*.avi *.flv *.mpg *.mpeg *.mp4 *.vob *.ts"));
    QFileInfo *fileInfo_Video = new QFileInfo(filename_Video);
    if (!fileInfo_Video->fileName().isEmpty()) {
        ui->lineEdit_Video->setText(fileInfo_Video->fileName());
        ui->label_VideoSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
}

void QCMainWindow::on_actionAbout_triggered()
{
    //QMessageBox::information(this, tr("About UltraStar Song Creator"),
    //                  tr("<b>UltraStar Song Creator 0.1</b><br><br> by:saiya_mg & bohning"));
    QUAboutDialog(this).exec();
}

void QCMainWindow::on_actionQuit_USC_triggered()
{
    close();
}

void QCMainWindow::dragEnterEvent( QDragEnterEvent* event ) {
    const QMimeData* md = event->mimeData();
    if( event && md->hasUrls()) {
        event->acceptProposedAction();
    }
}

void QCMainWindow::dropEvent( QDropEvent* event ) {
    if(event && event->mimeData()) {
        const QMimeData *data = event->mimeData();
        if (data->hasUrls()) {
            QList<QUrl> urls = data->urls();
            while (!urls.isEmpty()) {
                QString fileName = urls.takeFirst().toLocalFile();
                if (!fileName.isEmpty()) {
                    QFileInfo *fileInfo = new QFileInfo(fileName);

                    if (fileInfo->suffix().toLower() == "txt") {
                        QFile file(fileName);
                        if (file.open(QFile::ReadOnly | QFile::Text)) {
                            ui->plainTextEdit_InputLyrics->setPlainText(file.readAll());
                        }
                    }
                    else if (fileInfo->suffix().toLower() == tr("mp3") || fileInfo->suffix().toLower() == tr("ogg")) {
                        filename_MP3 = fileName;
                        handleMP3();
                    }
                    else if (fileInfo->suffix().toLower() == tr("jpg")) {
                        int result = QUMessageBox::question(0,
                                        QObject::tr("Image file drop detected."),
                                        QObject::tr("Use <b>%1</b> as...").arg(fileInfo->fileName()),
                                        BTN << ":/types/cover.png"        << QObject::tr("Cover")
                                            << ":/types/background.png" << QObject::tr("Background")
                                            << ":/marks/cancel.png" << QObject::tr("Ignore this file"));

                        if (result == 0) {
                            if (!fileInfo->fileName().isEmpty()) {
                                ui->lineEdit_Cover->setText(fileInfo->fileName());
                                ui->label_CoverSet->setPixmap(QPixmap(":/marks/path_ok.png"));
                            }
                        }
                        else if (result == 1) {
                            if (!fileInfo->fileName().isEmpty()) {
                                ui->lineEdit_Background->setText(fileInfo->fileName());
                                ui->label_BackgroundSet->setPixmap(QPixmap(":/marks/path_ok.png"));
                            }
                        }
                        else {
                            // user cancelled
                        }
                    }
                    else if ((fileInfo->suffix().toLower() == tr("avi")) || fileInfo->suffix().toLower() == tr("flv") || fileInfo->suffix().toLower() == tr("mpg") || fileInfo->suffix().toLower() == tr("mpeg") || fileInfo->suffix().toLower() == tr("mp4") || fileInfo->suffix().toLower() == tr("vob") || fileInfo->suffix().toLower() == tr("ts")) {
                        if (!ui->groupBox_VideoTags->isChecked()) {
                            ui->groupBox_VideoTags->setChecked(true);
                        }
                        if (!fileInfo->fileName().isEmpty()) {
                            ui->lineEdit_Video->setText(fileInfo->fileName());
                            ui->label_VideoSet->setPixmap(QPixmap(":/marks/path_ok.png"));
                        }
                    }
                }
            }
        }
    }
}

void QCMainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void QCMainWindow::on_lineEdit_Title_textChanged(QString title)
{
    if(!title.isEmpty()) {
        ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_TitleSet->setStatusTip(tr("#TITLE tag is set."));
    }
    else {
        ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_TitleSet->setStatusTip(tr("#TITLE tag is empty."));
    }
}

void QCMainWindow::on_lineEdit_Artist_textChanged(QString artist)
{
    if(!artist.isEmpty()) {
        ui->label_ArtistSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_ArtistSet->setStatusTip(tr("#ARTIST tag is set."));
    }
    else {
        ui->label_ArtistSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_ArtistSet->setStatusTip(tr("#ARTIST tag is empty."));
    }
}

void QCMainWindow::on_comboBox_Language_currentIndexChanged(QString language)
{
    if(!language.isEmpty()) {
        ui->label_LanguageSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_LanguageSet->setStatusTip(tr("#LANGUAGE tag is set."));
    }
    else {
        ui->label_LanguageSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_LanguageSet->setStatusTip(tr("#LANGUAGE tag is empty."));
    }
}

void QCMainWindow::on_comboBox_Edition_textChanged(QString edition)
{
    if(!edition.isEmpty()) {
        ui->label_EditionSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_EditionSet->setStatusTip(tr("#EDITION tag is set."));
    }
    else {
        ui->label_EditionSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_EditionSet->setStatusTip(tr("#EDITION tag is empty."));
    }
}

void QCMainWindow::on_comboBox_Genre_textChanged(QString genre)
{
    if(!genre.isEmpty()) {
        ui->label_GenreSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_GenreSet->setStatusTip(tr("#GENRE tag is set."));
    }
    else {
        ui->label_GenreSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_GenreSet->setStatusTip(tr("#GENRE tag is empty."));
    }
}

void QCMainWindow::on_comboBox_Year_currentIndexChanged(QString year)
{
    if(!year.isEmpty()) {
        ui->label_YearSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_YearSet->setStatusTip(tr("#YEAR tag is set."));
    }
    else {
        ui->label_YearSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_YearSet->setStatusTip(tr("#YEAR tag is empty."));
    }
}

void QCMainWindow::on_lineEdit_Creator_textChanged(QString creator)
{
    if(!creator.isEmpty()) {
        ui->label_CreatorSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_CreatorSet->setStatusTip(tr("#CREATOR tag is set."));
    }
    else {
        ui->label_CreatorSet->setPixmap(QPixmap(":/marks/path_error.png"));
        ui->label_CreatorSet->setStatusTip(tr("#CREATOR tag is empty."));
    }
}

void QCMainWindow::on_pushButton_LoadFromFile_clicked()
{
    QString filename_Text = QFileDialog::getOpenFileName ( 0, tr("Please choose text file"), QDir::homePath(), tr("Text files (*.txt)"));

    if (!filename_Text.isEmpty()) {
        QFile file(filename_Text);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            ui->plainTextEdit_InputLyrics->setPlainText(file.readAll());
        }
    }
}

void QCMainWindow::on_pushButton_InputLyricsIncreaseFontSize_clicked()
{
    QFont font = ui->plainTextEdit_InputLyrics->font();
    if (font.pointSize() < 20) {
        font.setPointSize(font.pointSize()+1);
        ui->plainTextEdit_InputLyrics->setFont(font);
    }
}

void QCMainWindow::on_pushButton_InputLyricsDecreaseFontSize_clicked()
{
    QFont font = ui->plainTextEdit_InputLyrics->font();
    if (font.pointSize() > 5) {
        font.setPointSize(font.pointSize()-1);
        ui->plainTextEdit_InputLyrics->setFont(font);
    }
}

void QCMainWindow::on_pushButton_OutputLyricsIncreaseFontSize_clicked()
{
    QFont font = ui->plainTextEdit_OutputLyrics->font();
    if (font.pointSize() < 20) {
        font.setPointSize(font.pointSize()+1);
        ui->plainTextEdit_OutputLyrics->setFont(font);
    }
}

void QCMainWindow::on_pushButton_OutputLyricsDecreaseFontSize_clicked()
{
    QFont font = ui->plainTextEdit_OutputLyrics->font();
    if (font.pointSize() > 5) {
        font.setPointSize(font.pointSize()-1);
        ui->plainTextEdit_OutputLyrics->setFont(font);
    }
}

void QCMainWindow::on_plainTextEdit_InputLyrics_textChanged()
{
    if (!ui->plainTextEdit_InputLyrics->toPlainText().isEmpty() && !ui->lineEdit_MP3->text().isEmpty()) {
        if (state == QCMainWindow::uninitialized) {
            state = QCMainWindow::initialized;
        }
        ui->pushButton_PlayPause->setEnabled(true);
    }
    else {
        state = QCMainWindow::uninitialized;
        ui->pushButton_PlayPause->setDisabled(true);
    }
}

/*!
 * Changes the application language to english.
 */
void QCMainWindow::on_actionEnglish_triggered()
{
    ui->actionGerman->font().setBold(false);
    ui->actionEnglish->setChecked(true);
    ui->actionEnglish->font().setBold(true);

    QSettings settings;
    settings.setValue("language", QLocale(QLocale::English, QLocale::UnitedStates).name());

    // ---------------

    int result = QUMessageBox::information(this,
                    tr("Change Language"),
                    tr("Application language changed to <b>English</b>. You need to restart UltraStar Creator to take effect."),
                    BTN << ":/control/quit.png" << tr("Quit UltraStar Creator.")
                        << ":/marks/accept.png" << tr("Continue."));
    if(result == 0)
            this->close();
}

/*!
 * Changes the application language to german.
 */
void QCMainWindow::on_actionGerman_triggered()
{
    ui->actionEnglish->font().setBold(false);
    ui->actionGerman->setChecked(true);
    ui->actionGerman->font().setBold(true);

    QSettings settings;
    settings.setValue("language", QLocale(QLocale::German, QLocale::Germany).name());

    // ---------------

    int result = QUMessageBox::information(this,
                    tr("Change Language"),
                    tr("Application language changed to <b>German</b>. You need to restart UltraStar Creator to take effect."),
                    BTN << ":/control/quit.png" << tr("Quit UltraStar Creator.")
                        << ":/marks/accept.png" << tr("Continue."));
    if(result == 0)
            this->close();
}

void QCMainWindow::BASS_Stop() {
        if(!_mediaStream)
                return;

        if(!BASS_ChannelStop(_mediaStream)) {
                //logSrv->add(QString("BASS ERROR: %1").arg(BASS_ErrorGetCode()), QU::Warning);
                return;
        }
}

void QCMainWindow::BASS_Free() {
        if(!_mediaStream)
                return;

        if(!BASS_StreamFree(_mediaStream)) {
                //logSrv->add(QString("BASS ERROR: %1").arg(BASS_ErrorGetCode()), QU::Warning);
                return;
        }
}

void QCMainWindow::BASS_StopAndFree() {
        if(!_mediaStream)
                return;

        if(!BASS_ChannelStop(_mediaStream)) {
                return;
        }

        if(!BASS_StreamFree(_mediaStream)) {
                return;
        }
}

void QCMainWindow::BASS_Play() {
        if(!_mediaStream) {
                return;
        }

        if(!BASS_ChannelPlay(_mediaStream, TRUE)) {
                return;
        }
}

void QCMainWindow::BASS_Pause() {
        if(!_mediaStream) {
                return;
        }

        if(!BASS_ChannelPause(_mediaStream)) {
                return;
        }
}

void QCMainWindow::BASS_Resume() {
        if(!_mediaStream) {
                return;
        }

        if(!BASS_ChannelPlay(_mediaStream, FALSE)) {
                return;
        }
}

/*!
 * Get current position in seconds of the stream.
 */
double QCMainWindow::BASS_Position() {
        if(!_mediaStream)
                return -1;

        return BASS_ChannelBytes2Seconds(_mediaStream, BASS_ChannelGetPosition(_mediaStream, BASS_POS_BYTE));
}

void QCMainWindow::BASS_SetPosition(int seconds) {
        if(!_mediaStream)
                return;

        QWORD pos = BASS_ChannelSeconds2Bytes(_mediaStream, (double)seconds);

        if(!BASS_ChannelSetPosition(_mediaStream, pos, BASS_POS_BYTE)) {
                //logSrv->add(QString("BASS ERROR: %1").arg(BASS_ErrorGetCode()), QU::Warning);
                return;
        }
}

void QCMainWindow::handleMP3() {
    QFileInfo *fileInfo_MP3 = new QFileInfo(filename_MP3);
    if (!fileInfo_MP3->fileName().isEmpty()) {
        ui->lineEdit_MP3->setText(fileInfo_MP3->fileName());
        ui->label_MP3Set->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->label_MP3Set->setStatusTip(tr("#MP3 tag is set."));
        if (!ui->plainTextEdit_InputLyrics->toPlainText().isEmpty()) {
            state = QCMainWindow::initialized;
            QMainWindow::statusBar()->showMessage(tr("State: initialized."));
            ui->pushButton_PlayPause->setEnabled(true);
        }
        else {
            state = QCMainWindow::uninitialized;
            QMainWindow::statusBar()->showMessage(tr("State: uninitialized."));
            ui->pushButton_PlayPause->setDisabled(true);
        }
    }

    _mediaStream = BASS_StreamCreateFile(FALSE, filename_MP3.toLocal8Bit().data() , 0, 0, BASS_STREAM_DECODE);
    QWORD MP3LengthBytes = BASS_ChannelGetLength(_mediaStream, BASS_POS_BYTE); // the length in bytes
    double MP3LengthTime = BASS_ChannelBytes2Seconds(_mediaStream, MP3LengthBytes); // the length in seconds
    ui->progressBar_MP3->setRange(0, (int)MP3LengthTime);
    ui->progressBar_MP3->setValue(0);

    BPMFromMP3 = BASS_FX_BPM_DecodeGet(_mediaStream, 0, 60, 0, BASS_FX_BPM_BKGRND, 0);

    if (BPMFromMP3 < 50) {
        BPMFromMP3 = BPMFromMP3*8;
    }
    else if (BPMFromMP3 < 100) {
        BPMFromMP3 = BPMFromMP3*4;
    }
    else if (BPMFromMP3 < 200) {
        BPMFromMP3 = BPMFromMP3*2;
    }

    ui->doubleSpinBox_BPM->setValue(BPMFromMP3);
    ui->label_BPMSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    ui->label_BPMSet->setStatusTip(tr("#BPM tag set."));

    TagLib::FileRef ref(filename_MP3.toLocal8Bit().data());
    ui->lineEdit_Artist->setText(TStringToQString(ref.tag()->artist()));
    ui->lineEdit_Title->setText(TStringToQString(ref.tag()->title()));
    ui->comboBox_Genre->setEditText(TStringToQString(ref.tag()->genre()));
    ui->comboBox_Year->setCurrentIndex(ui->comboBox_Year->findText(QString::number(ref.tag()->year())));
}

void QCMainWindow::on_horizontalSlider_PlaybackSpeed_valueChanged(int value)
{
    ui->label_PlaybackSpeedPercentage->setText(tr("%1 \%").arg(QString::number(value)));
}

void QCMainWindow::on_actionAbout_BASS_triggered()
{
    QUMessageBox::information(this,
                            tr("About BASS"),
                            QString(tr("<b>BASS Audio Library</b><br><br>"
                                            "BASS is an audio library for use in Windows and MacOSX software. Its purpose is to provide the most powerful and efficient (yet easy to use), sample, stream, MOD music, and recording functions. All in a tiny DLL, under 100KB in size.<br><br>"
                                            "Version: <b>%1</b><br><br>"
                                            "Copyright (c) 1999-2008<br><a href=\"http://www.un4seen.com/bass.html\">Un4seen Developments Ltd.</a> All rights reserved."))
                                            .arg(BASSVERSIONTEXT),
                            QStringList() << ":/marks/accept.png" << "OK",
                            330);
}

void QCMainWindow::on_actionAbout_TagLib_triggered()
{
    QUMessageBox::information(this,
                            tr("About TagLib"),
                            QString(tr("<b>TagLib Audio Meta-Data Library</b><br><br>"
                                            "TagLib is a library for reading and editing the meta-data of several popular audio formats.<br><br>"
                                            "Version: <b>%1.%2.%3</b><br><br>"
                                            "Visit: <a href=\"http://developer.kde.org/~wheeler/taglib.html\">TagLib Homepage</a>"))
                                            .arg(TAGLIB_MAJOR_VERSION)
                                            .arg(TAGLIB_MINOR_VERSION)
                                            .arg(TAGLIB_PATCH_VERSION));
}

void QCMainWindow::updateTime() {
        int posSec = (int)BASS_Position();

        ui->progressBar_MP3->setValue(posSec);

        if(posSec != -1) {
            QTimer::singleShot(1000, this, SLOT(updateTime()));
        }
}

void QCMainWindow::on_pushButton_Reset_clicked()
{
    if (state == stopped) {
        state = QCMainWindow::initialized;
        currentSyllableIndex = 0;
        firstNote = true;
        ui->plainTextEdit_OutputLyrics->clear();
        ui->pushButton_Tap->setText("");
        ui->pushButton_NextSyllable1->setText("");
        ui->pushButton_NextSyllable2->setText("");
        ui->pushButton_NextSyllable3->setText("");
        ui->pushButton_NextSyllable4->setText("");
        ui->pushButton_NextSyllable5->setText("");
        ui->progressBar_MP3->setValue(0);
        ui->progressBar_Lyrics->setValue(0);
        ui->groupBox_SongMetaInformationTags->setEnabled(true);
        ui->groupBox_MP3ArtworkTags->setEnabled(true);
        ui->groupBox_MiscSettings->setEnabled(true);
        ui->groupBox_VideoTags->setEnabled(true);
        ui->groupBox_InputLyrics->setEnabled(true);
        ui->groupBox_OutputLyrics->setDisabled(true);
        ui->groupBox_TapArea->setDisabled(true);
        ui->pushButton_Tap->setDisabled(true);
        ui->pushButton_PlayPause->setEnabled(true);
        ui->pushButton_Stop->setDisabled(true);
        ui->pushButton_Reset->setDisabled(true);
        _mediaStream = BASS_StreamCreateFile(FALSE, filename_MP3.toLocal8Bit().data() , 0, 0, BASS_STREAM_DECODE);
    }
    else {
        // should not be possible
    }
}

#include "main.h"

#include "QCMainWindow.h"
#include "QULogService.h"
#include "QUMessageBox.h"
#include "QUAboutDialog.h"
#include "hunspelldll.h"

#include <QFileDialog>
#include <QTextStream>
#include <QClipboard>
#include <QUrl>
#include <QSettings>
#include <QTimer>
#include <QTextCodec>
#include <QProcess>
#include <QUrl>

QCMainWindow::QCMainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::QCMainWindow) {

    ui->setupUi(this);
    setWindowTitle(tr("UltraStar Song Creator %1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION));

    // adding languages to language combobox as I did not find a way to add itemData within designer
    // this way, foreign language names are displayed to the user while the UltraStar file will contain the
    // english language name
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/cn.png"),tr("Chinese"),"Chinese");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/hr.png"),tr("Croatian"),"Croatian");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/cz.png"),tr("Czech"),"Czech");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/dk.png"),tr("Danish"),"Danish");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/nl.png"),tr("Dutch"),"Dutch");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/us.png"),tr("English"),"English");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/fi.png"),tr("Finnish"),"Finnish");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/fr.png"),tr("French"),"French");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/de.png"),tr("German"),"German");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/in.png"),tr("Hindi"),"Hindi");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/it.png"),tr("Italian"),"Italian");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/jp.png"),tr("Japanese"),"Japanese");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/kr.png"),tr("Korean"),"Korean");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/va.png"),tr("Latin"),"Latin");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/no.png"),tr("Norwegian"),"Norwegian");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/pl.png"),tr("Polish"),"Polish");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/pt.png"),tr("Portuguese"),"Portuguese");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/ru.png"),tr("Russian"),"Russian");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/sk.png"),tr("Slovak"),"Slovak");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/si.png"),tr("Slowenian"),"Slowenian");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/es.png"),tr("Spanish"),"Spanish");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/se.png"),tr("Swedish"),"Swedish");
    ui->comboBox_Language->addItem(QIcon(":/languages/lang/tr.png"),tr("Turkish"),"Turkish");

    logSrv->add(tr("Ready."), QU::Information);
    lyricsProgressBar = new QProgressBar;
    QMainWindow::statusBar()->addPermanentWidget(lyricsProgressBar);
    QMainWindow::statusBar()->showMessage(tr("USC ready."));
    if (BASS_Init(-1, 44100, 0, 0, NULL)) {
        QMainWindow::statusBar()->showMessage(tr("BASS initialized."));
    }

    this->show();

    QUMessageBox::information(0, QObject::tr("Welcome to UltraStar Creator!"),
                              QObject::tr("This tool enables you to <b>rapidly</b> create UltraStar text files <b>from scratch</b>.<br><br>To get started, simply chose a <b>song file</b> in MP3 or OGG format, insert the <b>song lyrics</b> from a file or the clipboard and divide them into syllables with '+'.<br><br><b>Important song meta information</b> such as <b>BPM</b> and <b>GAP</b> are determined <b>automatically</b> while the <b>ID3 tag</b> is used to fill in additional song details, if available.<br><br>To <b>start tapping</b>, hit the play/pause button (Keyboard: CTRL+P). Keep the <b>tap button</b> (keyboard: space bar) pressed for as long as the current syllable is sung to tap a note. <b>Undo</b> the last tap with the undo button (Keyboard: x), <b>stop tapping</b> with the stop button (Keyboard: CTRL+S), <b>restart</b> from the beginning with the reset button (Keyboard: CTRL+R). When finished, <b>save</b> the tapped song using the save button (CTRL+S).<br><br>Having successfully tapped a song, use the UltraStar internal editor for <b>finetuning the timings</b>, setting <b>note pitches</b> and <b>golden</b> or <b>freestyle notes</b>.<br><br><b>Happy creating!</b>"),
                              BTN << ":/marks/accept.png" << QObject::tr("Okay. Let's go!"),550,0);

    QSettings settings;
    ui->lineEdit_Creator->setText(settings.value("creator", "").toString());
    QFileInfo fi(settings.value("songfile").toString());
    if (fi.exists()) {
        settings.remove("songfile");
        fileInfo_MP3 = &fi;
        handleMP3();
    }

    // init
    numSyllables = 0;
    firstNoteStartBeat = 0;
    currentNoteBeatLength = 0;
    currentSyllableIndex = 0;
    currentCharacterIndex = 0;
    bool isFirstKeyPress = true;
    firstNote = true;
    clipboard = QApplication::clipboard();
    state = QCMainWindow::uninitialized;
    previewState = QCMainWindow::uninitialized;
    defaultDir = QDir::homePath();
}

/*!
 * Overloaded to ensure that all changes are saved before closing this application.
 */
void QCMainWindow::closeEvent(QCloseEvent *event) {
    QSettings settings;

    settings.setValue("windowState", QVariant(this->saveState()));
    settings.setValue("windowSize", size());

    // everything should be fine from now on
    QFile::remove("running.app");

    event->accept();
}

bool QCMainWindow::on_pushButton_SaveToFile_clicked()
{
    QString suggestedAbsoluteFilePath = fileInfo_MP3->absolutePath() + "\\" + ui->lineEdit_Artist->text() + " - " + ui->lineEdit_Title->text() + ".txt";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Please choose file"), suggestedAbsoluteFilePath, tr("Text files (*.txt)"));

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QUMessageBox::warning(this, tr("Application"),
            tr("Cannot write file %1:\n%2.")
            .arg(fileName)
            .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QTextCodec *codec = QTextCodec::codecForName("Windows-1252");
    if (codec->canEncode(ui->plainTextEdit_OutputLyrics->toPlainText())) {
        out.setCodec(codec);
        out << "#ENCODING:CP1252\n";
    }
    else {
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        out.setGenerateByteOrderMark(true); // BOM needed by UltraStar 1.1
        out << "#ENCODING:UTF8\n";
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << ui->plainTextEdit_OutputLyrics->toPlainText();
    QApplication::restoreOverrideCursor();
    ui->pushButton_startUltraStar->setEnabled(true);
    ui->pushButton_startYass->setEnabled(true);
    return true;
}

void QCMainWindow::on_pushButton_PlayPause_clicked()
{
    QSettings settings;

    if (state == initialized) {
        setCursor(Qt::WaitCursor);
        state = QCMainWindow::playing;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/pause.png"));
        ui->pushButton_PlayPause->setStatusTip(tr("Pause tapping."));
        QWidget::setAcceptDrops(false);
        ui->groupBox_MP3Tag->setDisabled(true);
        ui->groupBox_SongMetaInformationTags->setDisabled(true);
        ui->groupBox_ArtworkTags->setDisabled(true);
        ui->groupBox_MiscSettings->setDisabled(true);
        ui->groupBox_VideoTags->setDisabled(true);
        ui->groupBox_InputLyrics->setDisabled(true);
        ui->groupBox_OutputLyrics->setEnabled(true);
        ui->pushButton_SaveToFile->setDisabled(true);
        ui->pushButton_startUltraStar->setDisabled(true);
        ui->pushButton_startYass->setDisabled(true);
        ui->groupBox_TapArea->setEnabled(true);
        ui->pushButton_UndoTap->setDisabled(true);
        ui->pushButton_Tap->setEnabled(true);
        ui->pushButton_UndoTap->setDisabled(true);
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

        //ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#ENCODING:UTF8"));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#TITLE:%1").arg(ui->lineEdit_Title->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#ARTIST:%1").arg(ui->lineEdit_Artist->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#LANGUAGE:%1").arg(ui->comboBox_Language->itemData(ui->comboBox_Language->currentIndex()).toString()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#EDITION:%1").arg(ui->comboBox_Edition->currentText()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GENRE:%1").arg(ui->comboBox_Genre->currentText()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#YEAR:%1").arg(ui->spinBox_Year->text()));
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

        settings.setValue("creator", ui->lineEdit_Creator->text());

        QString rawLyricsString = ui->plainTextEdit_InputLyrics->toPlainText();

        lyricsString = cleanLyrics(rawLyricsString);

        splitLyricsIntoSyllables();
        numSyllables = lyricsSyllableList.length();
        lyricsProgressBar->setRange(0,numSyllables);
        lyricsProgressBar->setValue(0);
        lyricsProgressBar->show();

        updateSyllableButtons();

        // reset from preview play
        BASS_StopAndFree();
        _mediaStream = BASS_StreamCreateFile(FALSE, fileInfo_MP3->absoluteFilePath().toLocal8Bit().data() , 0, 0, BASS_STREAM_DECODE);

        playbackSpeedDecreasePercentage = 100 - ui->horizontalSlider_PlaybackSpeed->value();
        _mediaStream = BASS_FX_TempoCreate(_mediaStream, BASS_FX_FREESOURCE);
        bool result = BASS_ChannelSetAttribute(_mediaStream, BASS_ATTRIB_TEMPO, -playbackSpeedDecreasePercentage);
        if (result) {
            BASS_Play();
            updateTime();
        }

        ui->pushButton_Tap->setFocus(Qt::OtherFocusReason);
        setCursor(Qt::ArrowCursor);
    }
    else if (state == playing) {
        state = QCMainWindow::paused;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/play.png"));
        ui->pushButton_PlayPause->setStatusTip(tr("Continue tapping."));
        ui->pushButton_Tap->setDisabled(true);
        ui->pushButton_NextSyllable1->setDisabled(true);
        ui->pushButton_NextSyllable2->setDisabled(true);
        ui->pushButton_NextSyllable3->setDisabled(true);
        ui->pushButton_NextSyllable4->setDisabled(true);
        ui->pushButton_NextSyllable5->setDisabled(true);
        BASS_Pause();
    }
    else if (state == paused) {
        state = QCMainWindow::playing;
        ui->pushButton_PlayPause->setIcon(QIcon(":/player/pause.png"));
        ui->pushButton_PlayPause->setStatusTip(tr("Pause tapping."));
        ui->pushButton_Tap->setEnabled(true);
        ui->pushButton_NextSyllable1->setEnabled(true);
        ui->pushButton_NextSyllable2->setEnabled(true);
        ui->pushButton_NextSyllable3->setEnabled(true);
        ui->pushButton_NextSyllable4->setEnabled(true);
        ui->pushButton_NextSyllable5->setEnabled(true);
        ui->pushButton_Tap->setFocus(Qt::OtherFocusReason);
        BASS_Resume();
    }
    else {
        // should not be possible
    }
}

QString QCMainWindow::cleanLyrics(QString rawLyricsString) {
    // delete surplus space characters
    rawLyricsString = rawLyricsString.replace(QRegExp(" {2,}"), " ");

    // delete surplus blank lines...
    rawLyricsString = rawLyricsString.replace(QRegExp("\\n{2,}"), "\n");

    // replace misused accents (´,`) by the correct apostrophe (')
    rawLyricsString = rawLyricsString.replace("´", "'");
    rawLyricsString = rawLyricsString.replace("`", "'");

    // delete leading and trailing whitespace from each line
    QStringList rawLyricsStringList = rawLyricsString.split(QRegExp("\\n"));
    QStringList lyricsStringList;
    rawLyricsString.clear();
    QStringListIterator lyricsLineIterator(rawLyricsStringList);
    while (lyricsLineIterator.hasNext()) {
        QString currentLine = lyricsLineIterator.next();
        currentLine = currentLine.trimmed();
        currentLine.replace(0,1,currentLine.at(0).toUpper());
        lyricsStringList.append(currentLine);
    }
    rawLyricsString = lyricsStringList.join("\n");

    QString cleanedLyricsString = rawLyricsString;

    ui->plainTextEdit_InputLyrics->setPlainText(cleanedLyricsString);

    return cleanedLyricsString;
}

void QCMainWindow::on_pushButton_Tap_pressed()
{
    currentNoteStartTime = BASS_Position()*1000; // milliseconds

    // conversion from milliseconds [ms] to quarter beats [qb]: time [ms] * BPMFromMP3 [qb/min] * 1/60 [min/s] * 1/1000 [s/ms]
    previousNoteEndBeat = currentNoteStartBeat + currentNoteBeatLength - firstNoteStartBeat;
    currentNoteStartBeat = currentNoteStartTime * (BPMFromMP3 / 15000);
    ui->pushButton_Tap->setCursor(Qt::ClosedHandCursor);
}


void QCMainWindow::on_pushButton_Tap_released()
{
    double currentNoteTimeLength = BASS_Position()*1000 - currentNoteStartTime;
    ui->pushButton_Tap->setCursor(Qt::OpenHandCursor);
    lyricsProgressBar->setValue(lyricsProgressBar->value()+1);
    if (!ui->pushButton_UndoTap->isEnabled()) {
        ui->pushButton_UndoTap->setEnabled(true);
    }
    currentNoteBeatLength = qMax(1.0, currentNoteTimeLength * (BPMFromMP3 / 15000));
    if (firstNote){
        firstNoteStartBeat = currentNoteStartBeat;
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GAP:%1").arg(QString::number(currentNoteStartTime)));
        ui->doubleSpinBox_Gap->setValue(currentNoteStartTime);
        ui->label_GapSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        firstNote = false;
    }
    bool addLinebreak = false;
    QString linebreakString = "";

    QString currentSyllable = lyricsSyllableList[currentSyllableIndex];
    if (currentSyllable.startsWith("\n")) {
        addLinebreak = true;
        currentSyllable = currentSyllable.mid(1);
    }

    if (addLinebreak)
    {
        qint32 linebreakBeat = previousNoteEndBeat + (currentNoteStartBeat - firstNoteStartBeat - previousNoteEndBeat)/2;
        linebreakString = tr("- %1\n").arg(QString::number(linebreakBeat));
    }

    currentOutputTextLine = tr("%1: %2 %3 %4 %5").arg(linebreakString).arg(QString::number(currentNoteStartBeat - firstNoteStartBeat)).arg(QString::number(currentNoteBeatLength)).arg(ui->comboBox_DefaultPitch->currentIndex()).arg(currentSyllable);
    ui->plainTextEdit_OutputLyrics->appendPlainText(currentOutputTextLine);


    if ((currentSyllableIndex+1) < numSyllables) {
        currentSyllableIndex++;
        updateSyllableButtons();
    }
    else {
        ui->pushButton_Tap->setText("");
        on_pushButton_Stop_clicked();
    }
}

void QCMainWindow::on_pushButton_PasteFromClipboard_clicked()
{
    if (clipboard->mimeData()->hasText()) {
        ui->plainTextEdit_InputLyrics->setPlainText(clipboard->text());
    }
}

void QCMainWindow::on_pushButton_Stop_clicked()
{
    if (state == playing || state == paused) {
        state = QCMainWindow::stopped;
        QMainWindow::statusBar()->showMessage(tr("State: stopped."));
        BASS_StopAndFree();
        ui->label_TimeElapsed->setText("0:00");
        ui->label_TimeToRun->setText("0:00");

        ui->plainTextEdit_OutputLyrics->appendPlainText("E");
        ui->plainTextEdit_OutputLyrics->appendPlainText("");

        QMainWindow::statusBar()->showMessage(tr("USC ready."));

        ui->pushButton_PlayPause->setIcon(QIcon(":/player/play.png"));
        ui->pushButton_PlayPause->setDisabled(true);
        ui->pushButton_Stop->setDisabled(true);
        ui->pushButton_Reset->setEnabled(true);
        ui->groupBox_TapArea->setDisabled(true);
        ui->pushButton_SaveToFile->setEnabled(true);
        QWidget::setAcceptDrops(true);
    }
    else {
        // should not be possible
    }
}

void QCMainWindow::on_pushButton_BrowseMP3_clicked()
{
    QString filename_MP3 = QFileDialog::getOpenFileName ( 0, tr("Please choose MP3 file"), defaultDir, tr("Audio files (*.mp3 *.ogg)"));
    fileInfo_MP3 = new QFileInfo(filename_MP3);
    if (fileInfo_MP3->exists()) {
        defaultDir = fileInfo_MP3->absolutePath();        
        handleMP3();
    }
}

void QCMainWindow::on_pushButton_BrowseCover_clicked()
{
    QString filename_Cover = QFileDialog::getOpenFileName ( 0, tr("Please choose cover image file"), defaultDir, tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Cover = new QFileInfo(filename_Cover);
    if (fileInfo_Cover->exists()) {
        if (defaultDir == QDir::homePath()) {
            defaultDir = fileInfo_Cover->absolutePath();
        }
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
    QString filename_Background = QFileDialog::getOpenFileName ( 0, tr("Please choose background image file"), defaultDir, tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Background = new QFileInfo(filename_Background);
    if (fileInfo_Background->exists()) {
        if (defaultDir == QDir::homePath()) {
            defaultDir = fileInfo_Background->absolutePath();
        }
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
    QString filename_Video = QFileDialog::getOpenFileName ( 0,
                                                            tr("Please choose video file"),
                                                            defaultDir,
                                                            tr("Video files (*.avi *.divx *.flv *.mpg *.mpeg *.mp4 *.m4v *.vob *.ts);;All files (*.*)"));
    QFileInfo *fileInfo_Video = new QFileInfo(filename_Video);
    if (fileInfo_Video->exists()) {
        if (defaultDir == QDir::homePath()) {
            defaultDir = fileInfo_Video->absolutePath();
        }
        ui->lineEdit_Video->setText(fileInfo_Video->fileName());

    }
}

void QCMainWindow::on_lineEdit_Video_textChanged(QString video)
{
    if (!video.isEmpty()) {
        ui->label_VideoSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
}

void QCMainWindow::on_actionAbout_triggered()
{
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
                        if (fileInfo->exists()) {
                            defaultDir = fileInfo->absolutePath();
                            fileInfo_MP3 = fileInfo;
                            handleMP3();
                        }
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
                    else if ((fileInfo->suffix().toLower() == tr("avi")) || fileInfo->suffix().toLower() == tr("divx") || fileInfo->suffix().toLower() == tr("flv") || fileInfo->suffix().toLower() == tr("mpg") || fileInfo->suffix().toLower() == tr("mpeg") || fileInfo->suffix().toLower() == tr("m4v") || fileInfo->suffix().toLower() == tr("mp4") || fileInfo->suffix().toLower() == tr("vob") || fileInfo->suffix().toLower() == tr("ts")) {
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

void QCMainWindow::on_pushButton_BrowseLyrics_clicked()
{
    QString filename_Text = QFileDialog::getOpenFileName ( 0, tr("Please choose text file"), defaultDir, tr("Text files (*.txt)"));

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
    changeLanguage("English");
}

/*!
 * Changes the application language to french.
 */
void QCMainWindow::on_actionFrench_triggered()
{
    changeLanguage("French");
}

/*!
 * Changes the application language to german.
 */
void QCMainWindow::on_actionGerman_triggered()
{
    changeLanguage("German");
}

/*!
 * Changes the application language to italian.
 */
void QCMainWindow::on_actionItalian_triggered()
{
    changeLanguage("Italian");
}

/*!
 * Changes the application language to polish.
 */
void QCMainWindow::on_actionPolish_triggered()
{
    changeLanguage("Polish");
}

/*!
 * Changes the application language to spanish.
 */
void QCMainWindow::on_actionSpanish_triggered()
{
    changeLanguage("Spanish");
}

void QCMainWindow::changeLanguage(QString language) {
    QSettings settings;
    QString translatedLanguage;

    if (language == "English") {
        settings.setValue("language", QLocale(QLocale::English, QLocale::UnitedStates).name());
        translatedLanguage = tr("English");
    }
    else if (language == "French") {
        settings.setValue("language", QLocale(QLocale::French, QLocale::France).name());
        translatedLanguage = tr("French");
    }
    else if (language == "German") {
        settings.setValue("language", QLocale(QLocale::German, QLocale::Germany).name());
        translatedLanguage = tr("German");
    }
    else if (language == "Italian") {
        settings.setValue("language", QLocale(QLocale::Italian, QLocale::Italy).name());
        translatedLanguage = tr("Italian");
    }
    else if (language == "Polish") {
        settings.setValue("language", QLocale(QLocale::Polish, QLocale::Poland).name());
        translatedLanguage = tr("Polish");
    }
    else if (language == "Spanish") {
        settings.setValue("language", QLocale(QLocale::Spanish, QLocale::Spain).name());
        translatedLanguage = tr("Spanish");
    }

    int result = QUMessageBox::information(this,
                    tr("Change Language"),
                    tr("Application language changed to <b>%1</b>. You need to restart UltraStar Creator to take effect.").arg(translatedLanguage),
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
    setCursor(Qt::WaitCursor);

    ui->lineEdit_MP3->setText(fileInfo_MP3->fileName());

    _mediaStream = BASS_StreamCreateFile(FALSE, fileInfo_MP3->absoluteFilePath().toLocal8Bit().data() , 0, 0, BASS_STREAM_DECODE);
    QWORD MP3LengthBytes = BASS_ChannelGetLength(_mediaStream, BASS_POS_BYTE); // the length in bytes
    MP3LengthTime = BASS_ChannelBytes2Seconds(_mediaStream, MP3LengthBytes); // the length in seconds
    ui->horizontalSlider_MP3->setRange(0, (int)MP3LengthTime);
    ui->horizontalSlider_PreviewMP3->setRange(0, (int)MP3LengthTime);
    ui->horizontalSlider_MP3->setValue(0);
    ui->horizontalSlider_PreviewMP3->setValue(0);
    ui->label_TimeElapsed->setText(tr("0:00"));
    ui->label_PreviewTimeElapsed->setText(tr("0:00"));
    int minutesToRun = (MP3LengthTime / 60);
    int secondsToRun = ((int)MP3LengthTime % 60);
    ui->label_TimeToRun->setText(tr("-%1:%2").arg(minutesToRun).arg(secondsToRun, 2, 10, QChar('0')));
    ui->label_PreviewTimeToRun->setText(tr("-%1:%2").arg(minutesToRun).arg(secondsToRun, 2, 10, QChar('0')));

    BPMFromMP3 = BASS_FX_BPM_DecodeGet(_mediaStream, 0, MP3LengthTime, 0, BASS_FX_BPM_BKGRND, 0);

    if (BPMFromMP3 == 0) {
        BPMFromMP3 = 50;
    }

    if (BPMFromMP3 <= 50) {
        BPMFromMP3 = BPMFromMP3*8;
    }
    else if (BPMFromMP3 <= 100) {
        BPMFromMP3 = BPMFromMP3*4;
    }
    else if (BPMFromMP3 <= 200) {
        BPMFromMP3 = BPMFromMP3*2;
    }

    ui->doubleSpinBox_BPM->setValue(BPMFromMP3);
    ui->label_BPMSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    ui->label_BPMSet->setStatusTip(tr("#BPM tag set."));

    TagLib::FileRef ref(fileInfo_MP3->absoluteFilePath().toLocal8Bit().data());
    ui->lineEdit_Artist->setText(TStringToQString(ref.tag()->artist()));
    ui->lineEdit_Title->setText(TStringToQString(ref.tag()->title()));
    ui->comboBox_Genre->setEditText(TStringToQString(ref.tag()->genre()));
    ui->spinBox_Year->setValue(ref.tag()->year());
    // lyrics from mp3 lyrics-tag

    ui->groupBox_SongMetaInformationTags->setEnabled(true);
    ui->groupBox_ArtworkTags->setEnabled(true);
    ui->groupBox_InputLyrics->setEnabled(true);
    ui->label_MP3Set->setPixmap(QPixmap(":/marks/path_ok.png"));
    ui->label_BPMSet->setStatusTip(tr("MP3 set."));
    previewState = QCMainWindow::initialized;
    ui->pushButton_PreviewPlayPause->setEnabled(true);

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
    setCursor(Qt::ArrowCursor);
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
                                            "Version: <b>%1</b><br>"
                                            "<br><br><b>BASS FX Effects Extension</b><br><br>"
                                            "BASS FX is an extension providing several effects, including tempo & pitch control.<br><br>"
                                            "Version: <b>2.4.5</b><br><br><br>"
                                            "Copyright (c) 2003-2010<br><a href=\"http://www.un4seen.com/bass.html\">Un4seen Developments Ltd.</a> All rights reserved.")).arg(BASSVERSIONTEXT),
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
        int minutesElapsed = posSec / 60;
        int secondsElapsed = posSec % 60;
        ui->label_TimeElapsed->setText(tr("%1:%2").arg(minutesElapsed).arg(secondsElapsed, 2, 10, QChar('0')));
        int timeToRun = MP3LengthTime - posSec;
        int minutesToRun = (timeToRun / 60);
        int secondsToRun = (timeToRun % 60);
        ui->label_TimeToRun->setText(tr("-%1:%2").arg(minutesToRun).arg(secondsToRun, 2, 10, QChar('0')));
        ui->horizontalSlider_MP3->setValue(posSec);

        if(posSec != -1) {
            QTimer::singleShot(1000, this, SLOT(updateTime()));
        }
}

void QCMainWindow::updatePreviewTime() {
        int posSec = (int)BASS_Position();
        int minutesElapsed = posSec / 60;
        int secondsElapsed = posSec % 60;
        ui->label_PreviewTimeElapsed->setText(tr("%1:%2").arg(minutesElapsed).arg(secondsElapsed, 2, 10, QChar('0')));
        int timeToRun = MP3LengthTime - posSec;
        int minutesToRun = (timeToRun / 60);
        int secondsToRun = (timeToRun % 60);
        ui->label_PreviewTimeToRun->setText(tr("-%1:%2").arg(minutesToRun).arg(secondsToRun, 2, 10, QChar('0')));
        ui->horizontalSlider_PreviewMP3->setValue(posSec);

        if(posSec != -1) {
            QTimer::singleShot(1000, this, SLOT(updatePreviewTime()));
        }
}

void QCMainWindow::on_pushButton_Reset_clicked()
{
    if (state == stopped) {
        lyricsProgressBar->hide();
        state = QCMainWindow::initialized;
        previewState = QCMainWindow::initialized;
        numSyllables = 0;
        firstNoteStartBeat = 0;
        currentNoteBeatLength = 0;
        currentSyllableIndex = 0;
        currentCharacterIndex = 0;
        firstNote = true;
        lyricsSyllableList.clear();
        ui->plainTextEdit_OutputLyrics->clear();
        ui->pushButton_Tap->setText("");
        ui->pushButton_NextSyllable1->setText("");
        ui->pushButton_NextSyllable2->setText("");
        ui->pushButton_NextSyllable3->setText("");
        ui->pushButton_NextSyllable4->setText("");
        ui->pushButton_NextSyllable5->setText("");
        ui->horizontalSlider_MP3->setValue(0);
        lyricsProgressBar->setValue(0);
        ui->groupBox_MP3Tag->setEnabled(true);
        ui->groupBox_SongMetaInformationTags->setEnabled(true);
        ui->groupBox_ArtworkTags->setEnabled(true);
        ui->groupBox_MiscSettings->setEnabled(true);
        ui->groupBox_VideoTags->setEnabled(true);
        ui->groupBox_InputLyrics->setEnabled(true);
        ui->groupBox_OutputLyrics->setDisabled(true);
        ui->groupBox_TapArea->setDisabled(true);
        ui->pushButton_Tap->setDisabled(true);
        ui->pushButton_UndoTap->setDisabled(true);
        ui->pushButton_PlayPause->setEnabled(true);
        ui->pushButton_Stop->setDisabled(true);
        ui->pushButton_Reset->setDisabled(true);
        _mediaStream = BASS_StreamCreateFile(FALSE, fileInfo_MP3->absoluteFilePath().toLocal8Bit().data() , 0, 0, BASS_STREAM_DECODE);
    }
    else {
        // should not be possible
    }
}

void QCMainWindow::on_pushButton_UndoTap_clicked()
{
    if (currentSyllableIndex > 0) {
        currentSyllableIndex = currentSyllableIndex-1;
        if (currentSyllableIndex == 0) {
            firstNote = true;
            ui->plainTextEdit_OutputLyrics->undo(); // delete GAP
            ui->pushButton_UndoTap->setDisabled(true);
        }
        updateSyllableButtons();
        lyricsProgressBar->setValue(lyricsProgressBar->value()-1);
        ui->plainTextEdit_OutputLyrics->undo();
    }
}

void QCMainWindow::updateSyllableButtons() {
    QString syllable = lyricsSyllableList[currentSyllableIndex];
    if (syllable.startsWith("\n")) {
        syllable.replace("\n","");
        ui->pushButton_Tap->setIcon(QIcon(":/marks/pilcrow.png"));
    }
    else {
        ui->pushButton_Tap->setIcon(QIcon());
    }
    ui->pushButton_Tap->setText(syllable);

    if (currentSyllableIndex+1 < numSyllables) {
        syllable = lyricsSyllableList[currentSyllableIndex+1];
        if (syllable.startsWith("\n")) {
            syllable.replace("\n","");
           ui->pushButton_NextSyllable1->setIcon(QIcon(":/marks/pilcrow.png"));
        }
        else {
            ui->pushButton_NextSyllable1->setIcon(QIcon());
        }
        ui->pushButton_NextSyllable1->setText(syllable);
    }
    else {
        ui->pushButton_NextSyllable1->setText("");
    }
    if (currentSyllableIndex+2 < numSyllables) {
        syllable = lyricsSyllableList[currentSyllableIndex+2];
        if (syllable.startsWith("\n")) {
            syllable.replace("\n","");
            ui->pushButton_NextSyllable2->setIcon(QIcon(":/marks/pilcrow.png"));
        }
        else {
            ui->pushButton_NextSyllable2->setIcon(QIcon());
        }
        ui->pushButton_NextSyllable2->setText(syllable);
    }
    else {
        ui->pushButton_NextSyllable2->setText("");
    }
    if (currentSyllableIndex+3 < numSyllables) {
        syllable = lyricsSyllableList[currentSyllableIndex+3];
        if (syllable.startsWith("\n")) {
            syllable.replace("\n","");
            ui->pushButton_NextSyllable3->setIcon(QIcon(":/marks/pilcrow.png"));
        }
        else {
            ui->pushButton_NextSyllable3->setIcon(QIcon());
        }
        ui->pushButton_NextSyllable3->setText(syllable);
    }
    else {
        ui->pushButton_NextSyllable3->setText("");
    }
    if (currentSyllableIndex+4 < numSyllables) {
        syllable = lyricsSyllableList[currentSyllableIndex+4];
        if (syllable.startsWith("\n")) {
            syllable.replace("\n","");
            ui->pushButton_NextSyllable4->setIcon(QIcon(":/marks/pilcrow.png"));
        }
        else {
            ui->pushButton_NextSyllable4->setIcon(QIcon());
        }
        ui->pushButton_NextSyllable4->setText(syllable);
    }
    else {
        ui->pushButton_NextSyllable4->setText("");
    }
    if (currentSyllableIndex+5 < numSyllables) {
        syllable = lyricsSyllableList[currentSyllableIndex+5];
        if (syllable.startsWith("\n")) {
            syllable.replace("\n","");
            ui->pushButton_NextSyllable5->setIcon(QIcon(":/marks/pilcrow.png"));
        }
        else {
            ui->pushButton_NextSyllable5->setIcon(QIcon());
        }
        ui->pushButton_NextSyllable5->setText(syllable);
    }
    else {
        ui->pushButton_NextSyllable5->setText("");
    }
}

void QCMainWindow::splitLyricsIntoSyllables()
{
    int nextSeparatorIndex = lyricsString.mid(1).indexOf(QRegExp("[ +\\n]"));
    QString currentSyllable = "";

    while (nextSeparatorIndex != -1) {
        currentSyllable = lyricsString.mid(0,nextSeparatorIndex+1);
        if (currentSyllable.startsWith("+")) {
            currentSyllable = currentSyllable.mid(1);
        }
        lyricsSyllableList.append(currentSyllable);
        lyricsString = lyricsString.mid(nextSeparatorIndex+1);
        nextSeparatorIndex = lyricsString.mid(1).indexOf(QRegExp("[ +\\n]"));
        if (nextSeparatorIndex == -1) {
            currentSyllable = lyricsString;
            if (currentSyllable.startsWith("+")) {
                currentSyllable = currentSyllable.mid(1);
            }
            lyricsSyllableList.append(currentSyllable);
        }
    }
}

void QCMainWindow::keyPressEvent(QKeyEvent *event) {
    if (state == QCMainWindow::playing && isFirstKeyPress) {
        isFirstKeyPress = false;
        switch(event->key()) {
        case Qt::Key_V:
            event->ignore();
            QCMainWindow::on_pushButton_Tap_pressed();
            break;
        case Qt::Key_X:
            event->ignore();
            QCMainWindow::on_pushButton_UndoTap_clicked();
            break;
        default: QWidget::keyPressEvent(event);
        }
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

void QCMainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (state == QCMainWindow::playing && !isFirstKeyPress) {
        isFirstKeyPress = true;
        switch(event->key()) {
        case Qt::Key_V:
            event->ignore();
            QCMainWindow::on_pushButton_Tap_released();
        default: QWidget::keyPressEvent(event);
        }
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

void QCMainWindow::on_spinBox_Year_valueChanged(QString year)
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
void QCMainWindow::on_horizontalSlider_MP3_sliderMoved(int position)
{
    BASS_SetPosition(position);
}

void QCMainWindow::on_horizontalSlider_PreviewMP3_sliderMoved(int position)
{
    BASS_SetPosition(position);
}

void QCMainWindow::on_pushButton_startUltraStar_clicked()
{
    QSettings settings;
    QString USdxFilePath;
    QFileInfo *USdxFileInfo;

    if(settings.contains("USdxFilePath")) {
        USdxFilePath = settings.value("USdxFilePath").toString();
        USdxFileInfo = new QFileInfo(USdxFilePath);
        if(USdxFileInfo->exists()) {
            settings.setValue("USdxFilePath", USdxFilePath);
            QStringList USdxArguments;
            USdxArguments << "-SongPath" << fileInfo_MP3->absolutePath();
            QProcess::startDetached(USdxFilePath, USdxArguments);
        }
        else {
            settings.remove("USdxFilePath");
        }
    }
    else {
        USdxFilePath = QFileDialog::getOpenFileName(0, tr("Choose UltraStar executable"), QDir::homePath(),tr("UltraStar executable (*.exe)"));
        USdxFileInfo = new QFileInfo(USdxFilePath);
        if(USdxFileInfo->exists()) {
            settings.setValue("USdxFilePath", USdxFilePath);
            QStringList USdxArguments;
            USdxArguments << "-SongPath " << fileInfo_MP3->absolutePath();
            QProcess::startDetached(USdxFilePath, USdxArguments);
        }
    }
}

void QCMainWindow::on_pushButton_PreviewPlayPause_clicked()
{
    if (previewState == QCMainWindow::initialized) {
        previewState = QCMainWindow::playing;
        _mediaStream = BASS_FX_TempoCreate(_mediaStream, BASS_FX_FREESOURCE);
        bool result = BASS_ChannelSetAttribute(_mediaStream, BASS_ATTRIB_TEMPO, 0);
        if (result) {
            BASS_Play();
            updatePreviewTime();
        }
        ui->pushButton_PreviewPlayPause->setIcon(QIcon(":/control_pause_blue.png"));
    }
    else if (previewState == QCMainWindow::playing) {
        previewState = QCMainWindow::paused;
        BASS_Pause();
        ui->pushButton_PreviewPlayPause->setIcon(QIcon(":/control_play_blue.png"));
    }
    else if (previewState == QCMainWindow::paused) {
        previewState = QCMainWindow::playing;
        BASS_Resume();
        ui->pushButton_PreviewPlayPause->setIcon(QIcon(":/control_pause_blue.png"));
    }
    else {
        // should not occur
    }
}

void QCMainWindow::on_pushButton_startYass_clicked()
{
    QSettings settings;
    QString YassFilePath;
    QFileInfo *YassFileInfo;

    if(settings.contains("YassFilePath")) {
        YassFilePath = settings.value("YassFilePath").toString();
        YassFileInfo = new QFileInfo(YassFilePath);
        if(YassFileInfo->exists()) {
            settings.setValue("YassFilePath", YassFilePath);
            QStringList YassArguments;
            YassArguments << fileInfo_MP3->absolutePath();
            QProcess::startDetached(YassFilePath, YassArguments);
        }
        else {
            settings.remove("YassFilePath");
        }
    }
    else {
        YassFilePath = QFileDialog::getOpenFileName(0, tr("Choose YASS executable"), QDir::homePath(),tr("YASS executable (*.exe)"));
        YassFileInfo = new QFileInfo(YassFilePath);
        if(YassFileInfo->exists()) {
            settings.setValue("YassFilePath", YassFilePath);
            QStringList YassArguments;
            YassArguments << fileInfo_MP3->absolutePath();
            QProcess::startDetached(YassFilePath, YassArguments);
        }
    }
}

void QCMainWindow::on_pushButton_Syllabificate_clicked()
{
    nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(finishedSlot(QNetworkReply*)));

    QNetworkRequest request(QUrl("http://www.probandodominio.co.cc/silabear.php"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QByteArray data;
    QUrl params;
    bool langAvailable = false;

    QString language = ui->comboBox_Language->itemData(ui->comboBox_Language->currentIndex()).toString();
    if (language == "English") {
        language = "Eng";
        langAvailable = true;
    }
    else if (language == "German") {
        language = "Deu";
        langAvailable = true;
    }
    else if (language == "Spanish") {
        language = "Esp";
        langAvailable = true;
    }
    else {
        QString infoString;
        if (!ui->comboBox_Language->currentText().isEmpty()) {
            infoString = QString("The automatic lyrics syllabification is not (yet) available for <b>%1</b>.").arg(ui->comboBox_Language->currentText());
        }
        else {
            infoString = QString("The song language has not yet been set.");
        }
        int result = QUMessageBox::question(0,
                        QObject::tr("Syllabification."),
                        QObject::tr("%1 Apply...").arg(infoString),
                        BTN << ":/languages/lang/us.png"    << QObject::tr("English syllabification rules.")
                            << ":/languages/lang/de.png"    << QObject::tr("German syllabification rules.")
                            << ":/languages/lang/es.png"    << QObject::tr("Spanish syllabification rules.")
                            << ":/marks/cancel.png"         << QObject::tr("Cancel."));

        if (result == 0) {
            language = "Eng";
            langAvailable = true;
        }
        else if (result == 1) {
            language = "Deu";
            langAvailable = true;
        }
        else if (result == 2) {
            language = "Esp";
            langAvailable = true;
        }
        else {
            // user cancelled
        }
    }

    if (langAvailable) {
        params.addQueryItem("txt", cleanLyrics(ui->plainTextEdit_InputLyrics->toPlainText()));
        params.addQueryItem("idioma", language);
        params.addQueryItem("silabear", "silabear");
        params.addQueryItem("separador", "%2B");
        data.append(params.toString());
        data.remove(0,1);

        QNetworkReply* reply = nam->post(request,data);
    }
}

void QCMainWindow::finishedSlot(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        // read data from QNetworkReply here
        QByteArray bytes = reply->readAll();
        QString syllabifiedLyrics(bytes); // string
        syllabifiedLyrics.replace("&nbsp;"," ");
        syllabifiedLyrics.replace("<br>","##break##");
        syllabifiedLyrics.replace("</br>","##break##");
        syllabifiedLyrics.replace("</p>","##break##");
        syllabifiedLyrics.remove(QRegExp("<head>(.*)</head>",Qt::CaseInsensitive));
        syllabifiedLyrics.remove(QRegExp("<form(.)[^>]*</form>",Qt::CaseInsensitive));
        syllabifiedLyrics.remove(QRegExp("<script(.)[^>]*</script>",Qt::CaseInsensitive));
        syllabifiedLyrics.remove(QRegExp("<style(.)[^>]*</style>",Qt::CaseInsensitive));
        syllabifiedLyrics.remove(QRegExp("<(.)[^>]*>"));
        syllabifiedLyrics.replace("##break##","\n");
        ui->plainTextEdit_InputLyrics->setPlainText(syllabifiedLyrics.trimmed());
    }
    else
    {
        QUMessageBox::information(this, tr("Network error"), QString(tr("Error: %1").arg(reply->errorString())));
    }
}

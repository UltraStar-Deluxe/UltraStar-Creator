#include "main.h"

#include "QCMainWindow.h"
#include "QULogService.h"
#include "QUMessageBox.h"
#include "QUAboutDialog.h"

#include <QFileDialog>
#include <QTextStream>
#include <QClipboard>
#include <QSettings>
#include <QTimer>
#include <QTextCodec>
#include <QProcess>
#include <QDirIterator>
#include <QWhatsThis>

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

    // deactivate TeX syllabification for now
    ui->pushButton_SyllabificateTeX->setVisible(false);

    logSrv->add(tr("Ready."), QU::Information);
    lyricsProgressBar = new QProgressBar;
    QMainWindow::statusBar()->addPermanentWidget(lyricsProgressBar);
    QMainWindow::statusBar()->showMessage(tr("USC ready."));
    if (BASS_Init(-1, 44100, 0, 0, NULL)) {
        QMainWindow::statusBar()->showMessage(tr("BASS initialized."));
    }

    this->show();

    QSettings settings;
    bool firstRun = settings.value("firstRun", "true").toBool();

    if(firstRun) {
        QUMessageBox::information(0, QObject::tr("Welcome to UltraStar Creator!"),
                                  QObject::tr("This tool enables you to <b>rapidly</b> create UltraStar text files <b>from scratch</b>.<br><br>To get started, simply chose a <b>song file</b> in MP3 or OGG format, insert the <b>song lyrics</b> from a file or the clipboard and divide them into syllables with '+'.<br><br><b>Important song meta information</b> such as <b>BPM</b> and <b>GAP</b> are determined <b>automatically</b> while the <b>ID3 tag</b> is used to fill in additional song details, if available.<br><br>To <b>start tapping</b>, hit the play/pause button (Keyboard: CTRL+P). Keep the <b>tap button</b> (keyboard: space bar) pressed for as long as the current syllable is sung to tap a note. <b>Undo</b> the last tap with the undo button (Keyboard: x), <b>stop tapping</b> with the stop button (Keyboard: CTRL+S), <b>restart</b> from the beginning with the reset button (Keyboard: CTRL+R). When finished, <b>save</b> the tapped song using the save button (CTRL+S).<br><br>Having successfully tapped a song, use the UltraStar internal editor for <b>finetuning the timings</b>, setting <b>note pitches</b> and <b>golden</b> or <b>freestyle notes</b>.<br><br><b>Happy creating!</b>"),
                                  BTN << ":/marks/accept.png" << QObject::tr("Okay. Let's go!"),550,0);
        firstRun = false;
        settings.setValue("firstRun", firstRun);
    }

    ui->lineEdit_Creator->setText(settings.value("creator", "").toString());

    // restore font sizes
    QFont font = ui->plainTextEdit_InputLyrics->font();
    font.setPointSize(settings.value("inputlyricsfontsize", 10).toInt());
    ui->plainTextEdit_InputLyrics->setFont(font);
    font.setPointSize(settings.value("outputlyricsfontsize", 10).toInt());
    ui->plainTextEdit_OutputLyrics->setFont(font);

    //
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
    isFirstKeyPress = true;
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
    settings.setValue("inputlyricsfontsize", ui->plainTextEdit_InputLyrics->fontInfo().pointSize());
    settings.setValue("outputlyricsfontsize", ui->plainTextEdit_OutputLyrics->fontInfo().pointSize());

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
        ui->groupBox_MiscSettings->setDisabled(true);
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
        if (!ui->comboBox_Language->currentText().isEmpty()) {
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#LANGUAGE:%1").arg(ui->comboBox_Language->itemData(ui->comboBox_Language->currentIndex()).toString()));
        }
        if (!ui->lineEdit_Edition->text().isEmpty()) {
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#EDITION:%1").arg(ui->lineEdit_Edition->text()));
        }
        if (!ui->comboBox_Genre->currentText().isEmpty()) {
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GENRE:%1").arg(ui->comboBox_Genre->currentText()));
        }
        if (!ui->comboBox_Year->currentText().isEmpty()) {
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#YEAR:%1").arg(ui->comboBox_Year->currentText()));
        }
        if (!ui->lineEdit_Creator->text().isEmpty()) {
            ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#CREATOR:%1").arg(ui->lineEdit_Creator->text()));
        }
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#MP3:%1").arg(ui->lineEdit_MP3->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#COVER:%1").arg(ui->lineEdit_Cover->text()));
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#BACKGROUND:%1").arg(ui->lineEdit_Background->text()));

        if (!ui->lineEdit_Video->text().isEmpty()) {
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

    // conversion from milliseconds [ms] to quarter beats [qb]: time [ms] * BPM [qb/min] * 1/60 [min/s] * 1/1000 [s/ms]
    previousNoteEndBeat = currentNoteStartBeat + currentNoteBeatLength - firstNoteStartBeat;
    currentNoteStartBeat = currentNoteStartTime * (BPM / 15000);
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
    currentNoteBeatLength = qMax(1.0, currentNoteTimeLength * (BPM / 15000));
    if (firstNote){
        firstNoteStartBeat = currentNoteStartBeat;
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GAP:%1").arg(QString::number(currentNoteStartTime, 'f', 2)));
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
        ui->lineEdit_Cover->setText(fileInfo_Cover->fileName());
        ui->label_CoverSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
}

void QCMainWindow::on_pushButton_BrowseBackground_clicked()
{
    QString filename_Background = QFileDialog::getOpenFileName ( 0, tr("Please choose background image file"), defaultDir, tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Background = new QFileInfo(filename_Background);
    if (fileInfo_Background->exists()) {
        ui->lineEdit_Background->setText(fileInfo_Background->fileName());
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
        ui->doubleSpinBox_Videogap->setEnabled(true);
    }
    else {
        ui->doubleSpinBox_Videogap->setDisabled(true);
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
                            QTextStream lyrics(&file);
                            ui->plainTextEdit_InputLyrics->setPlainText(lyrics.readAll());
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

void QCMainWindow::on_lineEdit_Edition_textChanged(QString edition)
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
            QTextStream lyrics(&file);
            ui->plainTextEdit_InputLyrics->setPlainText(lyrics.readAll());
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
    BPM = BPMFromMP3;

    if (BPM == 0) {
        BPM = 50;
    }

    while (BPM <= 200) {
        BPM = BPM*2;
    }

    ui->doubleSpinBox_BPM->setValue(BPM);
    ui->label_BPMSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    ui->label_BPMSet->setStatusTip(tr("#BPM tag set."));

    TagLib::FileRef ref(fileInfo_MP3->absoluteFilePath().toLocal8Bit().data());
    ui->lineEdit_Artist->setText(TStringToQString(ref.tag()->artist()));
    ui->lineEdit_Title->setText(TStringToQString(ref.tag()->title()));
    ui->comboBox_Genre->setEditText(TStringToQString(ref.tag()->genre()));
    ui->comboBox_Year->setCurrentIndex(ui->comboBox_Year->findText(QString(ref.tag()->year())));
    // TODO: lyrics from mp3 lyrics-tag

    ui->groupBox_SongMetaInformationTags->setEnabled(true);
    ui->groupBox_MiscSettings->setEnabled(true);
    ui->groupBox_InputLyrics->setEnabled(true);
    ui->label_MP3Set->setPixmap(QPixmap(":/marks/path_ok.png"));
    ui->label_BPMSet->setStatusTip(tr("MP3 set."));
    previewState = QCMainWindow::initialized;
    ui->groupBox_Control->setEnabled(true);
    ui->pushButton_PreviewPlayPause->setEnabled(true);
    ui->label_PreviewTimeElapsed->setEnabled(true);
    ui->horizontalSlider_PreviewMP3->setEnabled(true);
    ui->label_PreviewTimeToRun->setEnabled(true);

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

        if (posSec != -1) {
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
        ui->groupBox_MiscSettings->setEnabled(true);
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

void QCMainWindow::on_comboBox_Year_activated(QString year)
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
            QProcess::startDetached(USdxFilePath, USdxArguments, USdxFileInfo->absolutePath());
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
            QProcess::startDetached(USdxFilePath, USdxArguments, USdxFileInfo->absolutePath());
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
            QProcess::startDetached(YassFilePath, YassArguments, YassFileInfo->absolutePath());
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
            QProcess::startDetached(YassFilePath, YassArguments, YassFileInfo->absolutePath());
        }
    }
}

void QCMainWindow::on_actionCreate_Dummy_Songs_triggered()
{
    int result = QUMessageBox::question(0,
                    QObject::tr("Freestyle text file generation."),
                    QObject::tr("This function will generate UltraStar compatible freestyle text files without any lyrics for each audio file in a subsequently selectable folder.<br><br>Each MP3 will be moved into a separate subdirectory and a text file containing the bare minimum of information will be automatically created along with a standard cover and background.<br><br>If your audio files follow an 'Artist - Title.mp3' naming scheme, they will be correctly mapped in the resulting song file."),
                    BTN << ":/marks/accept.png" << QObject::tr("Go ahead, I know what I am doing!")
                        << ":/marks/cancel.png" << QObject::tr("I'm not sure. I want cancel."),400,0);

    if (result == 0) {
        QString SongCollectionPath;
        SongCollectionPath = QFileDialog::getExistingDirectory(0, tr("Choose root song folder"), QDir::homePath());

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QDirIterator it(SongCollectionPath, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            if (it.fileInfo().suffix().toLower() == tr("mp3") || it.fileInfo().suffix().toLower() == tr("ogg")) {

                QMainWindow::statusBar()->showMessage(tr("Creating %1").arg(it.fileInfo().completeBaseName()));

                // create directory, move MP3/OGG into it
                QFile songFile(it.fileInfo().absoluteFilePath());
                QFileInfo songInfo(songFile);
                int separatorPos = songInfo.fileName().indexOf(" - ");

                QString artist;
                QString title;
                QString dirName;

                if (separatorPos != -1) {
                    artist = songInfo.completeBaseName().mid(0, separatorPos);
                    artist = artist.trimmed();
                    QStringList tokens = artist.split(QRegExp("(\\s+)"));
                        QList<QString>::iterator tokItr = tokens.begin();

                        for (tokItr = tokens.begin(); tokItr != tokens.end(); ++tokItr) {
                        (*tokItr) = (*tokItr).at(0).toUpper() + (*tokItr).mid(1);
                        }
                    artist = tokens.join(" ");
                    artist.replace("Feat.", "feat.", Qt::CaseSensitive);
                    artist.replace("With.", "with.", Qt::CaseSensitive);
                    artist.replace("Vs.", "vs.", Qt::CaseSensitive);

                    title = songInfo.completeBaseName().mid(separatorPos + 3);
                    title = title.trimmed();
                    tokens = title.split(QRegExp("(\\s+)"));
                        tokItr = tokens.begin();

                        for (tokItr = tokens.begin(); tokItr != tokens.end(); ++tokItr) {
                        (*tokItr) = (*tokItr).at(0).toUpper() + (*tokItr).mid(1);
                        }
                    title = tokens.join(" ");

                    dirName = tr("%1 - %2").arg(artist).arg(title);
                }
                else { // audio file does not follow "Artist - Title.*" naming scheme
                    artist = songInfo.completeBaseName();
                    title = "";

                    dirName = artist;
                }

                songInfo.dir().mkdir(dirName);
                QString newFileName(tr("%1/%2/%3.%4").arg(songInfo.absolutePath()).arg(dirName).arg(dirName).arg(songInfo.suffix().toLower()));
                songFile.rename(newFileName);

                // text file
                QString textFilename(tr("%1/%2/%3.txt").arg(songInfo.absolutePath()).arg(dirName).arg(dirName));
                QFile file(textFilename);
                if (!file.open(QFile::WriteOnly | QFile::Text)) {
                    QUMessageBox::warning(this, tr("Application"),
                        tr("Cannot write file %1:\n%2.")
                        .arg(textFilename)
                        .arg(file.errorString()));
                }

                QTextStream out(&file);
                QTextCodec *codec = QTextCodec::codecForName("Windows-1252");
                //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
                //out.setGenerateByteOrderMark(true); // BOM needed by UltraStar 1.1

                out.setCodec(codec);
                QString textString;
                //textString += "#ENCODING:CP1252\n";
                //textString += "#ENCODING:UTF8\n";
                if (separatorPos != -1) {
                    textString += tr("#TITLE:%1\n").arg(title);
                }
                else {
                    textString += tr("#TITLE:?\n");
                }
                textString += tr("#ARTIST:%1\n").arg(artist);
                textString += "#LANGUAGE:\n";
                textString += "#EDITION:\n";
                textString += "#GENRE:\n";
                textString += "#YEAR:\n";
                if (separatorPos != -1) {
                    textString += tr("#MP3:%1 - %2.%3\n").arg(artist).arg(title).arg(songInfo.suffix().toLower());
                    textString += tr("#COVER:%1 - %2 [CO].jpg\n").arg(artist).arg(title);
                    textString += tr("#BACKGROUND:%1 - %2 [BG].jpg\n").arg(artist).arg(title);
                }
                else {
                    textString += tr("#MP3:%1.%2\n").arg(artist).arg(songInfo.suffix().toLower());
                    textString += tr("#COVER:%1 [CO].jpg\n").arg(artist);
                    textString += tr("#BACKGROUND:%1 [BG].jpg\n").arg(artist);
                }
                textString += "#BPM:300\n";
                textString += "#GAP:0\n";
                textString += "F 0 0 0 \n";
                textString += "- 1\n";
                textString += "F 1 0 0 \n";
                textString += "E\n";
                out << textString;

                // Cover
                QString coverFilename(tr("%1/%2/%3 [CO].jpg").arg(songInfo.absolutePath()).arg(dirName).arg(dirName));
                QFile cover(":/NoCover.jpg");
                cover.copy(coverFilename);

                // Background
                QString backgroundFilename(tr("%1/%2/%3 [BG].jpg").arg(songInfo.absolutePath()).arg(dirName).arg(dirName));
                QFile background(":/NoBackground.jpg");
                background.copy(backgroundFilename);
            }
        }
        QApplication::restoreOverrideCursor();
    }
    else {
        // user cancelled
    }
}

void QCMainWindow::on_actionHelp_triggered()
{
    QUMessageBox::information(0, QObject::tr("Welcome to UltraStar Creator!"),
                              QObject::tr("This tool enables you to <b>rapidly</b> create UltraStar text files <b>from scratch</b>.<br><br>To get started, simply chose a <b>song file</b> in MP3 or OGG format, insert the <b>song lyrics</b> from a file or the clipboard and divide them into syllables with '+'.<br><br><b>Important song meta information</b> such as <b>BPM</b> and <b>GAP</b> are determined <b>automatically</b> while the <b>ID3 tag</b> is used to fill in additional song details, if available.<br><br>To <b>start tapping</b>, hit the play/pause button (Keyboard: CTRL+P). Keep the <b>tap button</b> (keyboard: space bar) pressed for as long as the current syllable is sung to tap a note. <b>Undo</b> the last tap with the undo button (Keyboard: x), <b>stop tapping</b> with the stop button (Keyboard: CTRL+S), <b>restart</b> from the beginning with the reset button (Keyboard: CTRL+R). When finished, <b>save</b> the tapped song using the save button (CTRL+S).<br><br>Having successfully tapped a song, use the UltraStar internal editor for <b>finetuning the timings</b>, setting <b>note pitches</b> and <b>golden</b> or <b>freestyle notes</b>.<br><br><b>Happy creating!</b>"),
                              BTN << ":/marks/accept.png" << QObject::tr("Okay. Let's go!"),550,0);
}

void QCMainWindow::on_actionWhats_This_triggered()
{
    if (!QWhatsThis::inWhatsThisMode()) {
        QWhatsThis::enterWhatsThisMode();
    }
    else {
        QWhatsThis::leaveWhatsThisMode();
    }
}

// begin syllabification --> thanks to Klafhor who provided the PHP code as a basis

void QCMainWindow::on_pushButton_SyllabificateRules_clicked()
{
    QString lyrics = ui->plainTextEdit_InputLyrics->toPlainText();
    QString language = ui->comboBox_Language->itemData(ui->comboBox_Language->currentIndex()).toString();
    QChar sep = '+';
    QString syllabifiedLyrics = "";
    QString lang;

    if ((language != "English") && (language != "German") && (language != "Spanish")) {
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
            language = "English";
        }
        else if (result == 1) {
            language = "German";
        }
        else if (result == 2) {
            language = "Spanish";
        }
        else {
            syllabifiedLyrics = lyrics;
        }
    }

    if (language == "English") {
        lang = "en";

        for (int i = 0; i < lyrics.length(); i++)
        {
            QChar ch1 = lyrics.at(i);
            QChar ch2 = lyrics.at(i+1);
            QChar ch3 = lyrics.at(i+2);
            QChar ch4 = lyrics.at(i+3);
            QChar ch5 = lyrics.at(i+4);

            // "e" at the end of a word isn't considered a vowel, for example "case"
            if (isVowel(ch1, lang) && isConsonant(ch2, lang) && ch3.toLower() == 'e' && ((!isConsonant(ch4, lang) && !isVowel(ch4, lang)) || (ch4.toLower() == 's' && (!isConsonant(ch5, lang) && !isVowel(ch5, lang))))) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3;
                i = i + 2;
            }
            // "e" at the end of a word preceded by two consonants isn't considered a vowel, for example "table"
            else if (isVowel(ch1, lang) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && ch4.toLower() == 'e' && ((!isConsonant(ch5, lang) && !isVowel(ch5, lang)) || ch5.toLower() == 's')) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + ch4;
                i = i + 3;
            }
            // initial consonants
            else if ((!isVowel(ch1, lang) && !isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || isConsonant(ch4, lang))) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3;
                i = i + 2;
            }
            // a consonant between 2 vowels
            else if (isVowel(ch1, lang) && isConsonant(ch2, lang) && (isVowel(ch3, lang) || ch3.toLower() == 'y')) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // inseparable consonants between vowels
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && isInseparable(ch2, ch3, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // separable consonants between vowels
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && !isInseparable(ch2, ch3, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // "qu" is inseparable
            else if (ch1.toLower() == 'q' && ch2.toLower() == 'u') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2;
                i = i + 1;
            }
            /*
            // hiatus
            else if (isVowel(ch1) && isVowel(ch2) && isHiatus(ch1, ch2)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }*/
            // other options
            else {
                syllabifiedLyrics = syllabifiedLyrics + ch1;
            }
        }
    }
    else if (language == "German") {
        lang = "de";

        for (int i = 0; i < lyrics.length(); i++)
        {
            QChar ch1 = lyrics.at(i);
            QChar ch2 = lyrics.at(i+1);
            QChar ch3 = lyrics.at(i+2);
            QChar ch4 = lyrics.at(i+3);
            QChar ch5 = lyrics.at(i+4);
            QChar ch6 = lyrics.at(i+5);
            QChar ch7 = lyrics.at(i+6);
            QChar ch8 = lyrics.at(i+7);

            // consonants at the beginning
            if ((!isVowel(ch1, lang) && !isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || isConsonant(ch4, lang))) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3;
                i = i + 2;
            }
            // consonant between two vowels
            else if (isVowel(ch1, lang) && isConsonant(ch2, lang) && (isVowel(ch3, lang) || ch3.toLower() == 'y')) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // double consonant isn't separated in german, by example "ad-X-ded"
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && ch2.toLower() == ch3.toLower()) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // biss-chen is an exception where "sch" is separated
            else if (ch1.toLower() == 'b' && ch2.toLower() == 'i' && ch3.toLower() == 's' && ch4.toLower() == 's' && ch5.toLower() == 'c') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + ch4 + sep;
                i = i + 3;
            }
            // Diminutiv 1: "äs+chen" or "ös+chen", e. g. "Hös+chen", "Mi+mös+chen", "Rös+chen", "Häs+chen", "Näs+chen"
            else if ((ch1.toLower() == 'ä' || ch1.toLower() == 'ö') && ch2.toLower() == 's' && ch3.toLower() == 'c' && ch4.toLower() == 'h' && ch5.toLower() == 'e' && ch6.toLower() == 'n') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep + ch3 + ch4 + ch5 + ch6;
                i = i + 5;
            }
            // Diminutiv 2: "äus+chen", e. g. "Mäus+chen", "Päus+chen", "Häus+chen"
            else if (ch1.toLower() == 'ä' && ch2.toLower() == 'u' && ch3.toLower() == 's' && ch4.toLower() == 'c' && ch5.toLower() == 'h' && ch6.toLower() == 'e' && ch7.toLower() == 'n') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + sep + ch4 + ch5 + ch6 + ch7;
                i = i + 6;
            }
            // Diminutiv 3: Umlaut + "ss" + suffix "chen", e. g. "Fäss+chen", "Dös+chen", "Küss+chen", "Flüss+chen", "Nüss+chen"
            else if (isUmlaut(ch1, lang) && ch2.toLower() == 's' && ch3.toLower() == 's' && ch4.toLower() == 'c' && ch5.toLower() == 'h' && ch6.toLower() == 'e' && ch7.toLower() == 'n') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + sep + ch4 + ch5 + ch6 + ch7;
                i = i + 6;
            }
            // Diminutiv 4: Umlaut + "sch" + suffix "chen", e. g. "Fläsch+chen"
            else if (isUmlaut(ch1, lang) && ch2.toLower() == 's' && ch3.toLower() == 'c' && ch4.toLower() == 'h' && ch5.toLower() == 'c' && ch6.toLower() == 'h' && ch7.toLower() == 'e' && ch8.toLower() == 'n') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + ch4 + sep + ch5 + ch6 + ch7 + ch8;
                i = i + 7;
            }
            // "sch" is inseparable in all other cases
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && ch2.toLower() == 's' && ch3.toLower() == 'c' && ch4.toLower() == 'h') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep + ch2;
                i = i + 1;
            }
            // prefix "ab"
            else if (ch1.toLower() == 'a' && ch2.toLower() == 'b') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // prefix "auf"
            else if (ch1.toLower() == 'a' && ch2.toLower() == 'u' && ch3.toLower() == 'f' && !((ch4.toLower() == 'e' && ch5.toLower() == 'n') || ch4.toLower() == 't')) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + sep;
                i = i + 2;
            }
            // prefix "aus"
            else if (ch1.toLower() == 'a' && ch2.toLower() == 'u' && ch3.toLower() == 's') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + sep;
                i = i + 2;
            }
            /* problem: abnabeln
            // prefix "be"
            else if (ch1.toLower() == 'b' && ch2.toLower() == 'e') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }*/
            // prefix "ver"
            else if (ch1.toLower() == 'v' && ch2.toLower() == 'e' && ch3.toLower() == 'r') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3 + sep;
                i = i + 2;
            }
            // "eie" is separated into "ei-e", e. g. "Klei+e", "Fei+er"
            else if (ch1.toLower() == 'e' && ch2.toLower() == 'i' && ch3.toLower() == 'e') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // "eue" is separated into "eu-e", e. g. "Reu+e"
            else if (ch1.toLower() == 'e' && ch2.toLower() == 'u' && ch3.toLower() == 'e') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // inseparable consonants between vowels
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && isInseparable(ch2, ch3, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // separable consonants between vowels
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && !isInseparable(ch2, ch3, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // "qu" is inseparable
            else if (ch1.toLower() == 'q' && ch2.toLower() == 'u') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2;
                i = i + 1;
            }
            // all other cases
            else {
                syllabifiedLyrics = syllabifiedLyrics + ch1;
            }
        }
    }
    else if (language == "Spanish") {
        lang = "es";

        for (int i = 0; i < lyrics.length(); i++)
        {
            QChar ch1 = lyrics.at(i);
            QChar ch2 = lyrics.at(i+1);
            QChar ch3 = lyrics.at(i+2);
            QChar ch4 = lyrics.at(i+3);

            // initial consonants don't separate in a word, the syllable need a vowel, by example "t-X-ranslation
            if ((!isVowel(ch1, lang) && !isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || isConsonant(ch4, lang)))
            {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + ch3;
                i = i + 2;
            }
            // consonant between 2 vowels, we separate this, by example "e-xample"
            else if (isVowel(ch1, lang) && isConsonant(ch2, lang) && (isVowel(ch3, lang) || ch3.toLower() == 'y')) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // inseparable consonants between vowels, for example "hel-X-lo"
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && isInseparable(ch2, ch3, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // separable consonants between vowels, for example "bet-ween"
            else if ((isVowel(ch1, lang) || isConsonant(ch1, lang)) && isConsonant(ch2, lang) && isConsonant(ch3, lang) && (isVowel(ch4, lang) || ch4.toLower() == 'y') && !isInseparable(ch2, ch3, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2 + sep;
                i = i + 1;
            }
            // the characters "qu" are inseparable "q-X-u"
            else if (ch1.toLower() == 'q' && ch2.toLower() == 'u') {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + ch2;
                i = i + 1;
            }
            // hiatus
            else if (isVowel(ch1, lang) && isVowel(ch2, lang) && isHiatus(ch1, ch2, lang)) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + sep;
            }
            // synalepha first word ending in a vowel
            else if (isVowel(ch1, lang) && ch2 == ' ' && ((isVowel(ch3, lang) || (ch3.toLower() == 'y' && (!isVowel(ch4, lang) && !isConsonant(ch4, lang)))) || (ch3.toLower() == 'h' && isVowel(ch4, lang)))) {
                syllabifiedLyrics = syllabifiedLyrics + ch1 + "·";
                i = i + 1;
            }
            // synalepha first word being 'y'
            else if ((ch1 == ' ') && ch2.toLower() == 'y' && ch3 == ' ' && (isVowel(ch4, lang) || ch4.toLower() == 'h')) {
                syllabifiedLyrics = syllabifiedLyrics + " y·";
                i = i + 2;
            }
            // synalepha and the beginning of the text
            else if (i == 1 && ch1.toLower() == 'y' && ch2 == ' ' && (isVowel(ch3, lang) || ch3.toLower() == 'h')) {
                syllabifiedLyrics = syllabifiedLyrics + "y·";
                i = i + 1;
            }
            // all other cases
            else {
                syllabifiedLyrics = syllabifiedLyrics + ch1;
            }
        }
    }
    ui->plainTextEdit_InputLyrics->setPlainText(syllabifiedLyrics);
}

bool QCMainWindow::isVowel(QChar character, QString lang)
{
    QChar ch = character.toLower();

    if (lang == "en")
    {
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "de")
    {
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'ä' || ch == 'ö' || ch == 'ü')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "es")
    {
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'á' || ch == 'é' || ch == 'í' || ch == 'ó' || ch == 'ú' || ch == 'à' || ch == 'ì' || ch == 'ò' || ch == 'ù' || ch == 'â' || ch == 'ê' || ch == 'î' || ch == 'ô' || ch == 'û')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool QCMainWindow::isUmlaut(QChar character, QString lang)
{
    QChar ch = character.toLower();

    if (lang == "en")
    {
        return false;
    }
    else if (lang == "de")
    {
        if (ch == 'ä' || ch == 'ö' || ch == 'ü')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "es")
    {
        return false;
    }
    else
    {
        return false;
    }
}

bool QCMainWindow::isDiphthong(QChar character1, QChar character2, QString lang)
{
    QChar ch1 = character1.toLower();
    QChar ch2 = character2.toLower();
    QString str = QString("%1%2").arg(ch1).arg(ch2);

    if (lang == "en")
    {
        if (str == "ou" || str == "ie" || str == "oi" || str == "oo" || str == "ea" || str == "ee" || str == "ai")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "de")
    {
        if (str == "au" || str == "ei" || str == "eu")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "es")
    {
        if (str == "ey" || str == "ai" || str == "oy" || str == "eu" || str == "au" || str == "ou" || str == "ie" || str == "ia" || str == "io" || str == "iu" || str == "ui" || str == "ue" || str == "ua" || str == "uo")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool QCMainWindow::isConsonant(QChar character, QString lang)
{
    QChar ch = character.toLower();

    if (lang == "en")
    {
        if (ch == 'b' || ch == 'c' || ch == 'd' || ch == 'f' || ch == 'g' || ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm' || ch == 'n' || ch == 'p' || ch == 'q' || ch == 'r' || ch == 's' || ch == 't' || ch == 'v' || ch == 'w' || ch == 'x' || ch == 'y' || ch == 'z')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "de")
    {
        if (ch == 'b' || ch == 'c' || ch == 'd' || ch == 'f' || ch == 'g' || ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm' || ch == 'n' || ch == 'p' || ch == 'q' || ch == 'r' || ch == 's' || ch == 't' || ch == 'v' || ch == 'w' || ch == 'x' || ch == 'y' || ch == 'z' || ch == 'ß')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (lang == "es")
    {
        if (ch == 'b' || ch == 'c' || ch == 'd' || ch == 'f' || ch == 'g' || ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm' || ch == 'n' || ch == 'ñ' || ch == 'p' || ch == 'q' || ch == 'r' || ch == 's' || ch == 't' || ch == 'v' || ch == 'w' || ch == 'x' || ch == 'y' || ch == 'z')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool QCMainWindow::isInseparable(QChar character1, QChar character2, QString lang)
{
    QChar ch1 = character1.toLower();
    QChar ch2 = character2.toLower();
    QString str = QString("%1%2").arg(ch1).arg(ch2);

    if (lang == "en" || lang == "de" || lang == "es")
    {
        if (((ch1 == 'b' || ch1 == 'c' || ch1 == 'd' || ch1 == 'f' || ch1 == 'g' || ch1 == 'p' || ch1 == 't' || ch1 == 'w') && (ch2 == 'l' || ch2 == 'r')) || (str == "rr") || (str == "ll") || ((ch1 == 'c' || ch1 == 't' ||  ch1 == 'r' || ch1 == 's' || ch1 == 'p' || ch1 == 'w') && ch2 == 'h') || (str == "ck"))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool QCMainWindow::isStrongVowel(QChar character, QString lang)
{
    QChar ch = character.toLower();

    if (lang == "es")
    {
        if (ch == 'a' || ch == 'e' || ch == 'o' || ch == 'á' || ch == 'é' || ch == 'ó')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool QCMainWindow::isHiatus(QChar character1, QChar character2, QString lang)
{
    QChar ch1 = character1.toLower();
    QChar ch2 = character2.toLower();
    QString str = QString("%1%2").arg(ch1).arg(ch2);

    if (lang == "es")
    {
        if ((isStrongVowel(ch1, lang) && isStrongVowel(ch2, lang)) || str == "íú" || str == "úí" || ch1 == ch2 || str == "aá" || str == "áa" || str == "eé" || str == "ée" || str == "ií" || str == "íi" || str == "oó" || str == "óo" || str == "uú" || str == "úu")
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

// end syllabification

void QCMainWindow::on_pushButton_SyllabificateTeX_clicked()
{
    //
    QString language = ui->comboBox_Language->itemData(ui->comboBox_Language->currentIndex()).toString();
    QChar sep = '+';
    QString syllabifiedLyrics = "";
    QFile patternFile;
    QString patternEncoding;

    if (language.isEmpty()) {
        QUMessageBox::warning(this, tr("Application"),
            tr("Please choose the song's language first."));
    }
    else if (language == "Croatian") {
        patternFile.setFileName(":/hyph/hyph_hr.dic");
        patternEncoding = "ISO-8859-2";
    }
    else if (language == "Czech") {
        patternFile.setFileName(":/hyph/hyph_cs_CZ.dic");
        patternEncoding = "ISO-8859-2";
    }
    else if (language == "Danish") {
        patternFile.setFileName(":/hyph/hyph_da_DK.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Dutch") {
        patternFile.setFileName(":/hyph/hyph_nl_NL.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "English") {
        patternFile.setFileName(":/hyph/hyph_en_US.dic");
        //patternFile.setFileName(":/hyph/hyph_en_GB.dic"); // British
        //patternFile.setFileName(":/hyph/hyph_en_CA.dic"); // Canadian
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Finnish") {
        patternFile.setFileName(":/hyph/hyph_fi_FI.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "French") {
        patternFile.setFileName(":/hyph/hyph_fr_FR.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "German") {
        patternFile.setFileName(":/hyph/hyph_de_DE.dic");
        //patternFile.setFileName(":/hyph/hyph_de_CH.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Hindi") {
        // no pattern file available
    }
    else if (language == "Italian") {
        patternFile.setFileName(":/hyph/hyph_it_IT.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Latin") {
        // no pattern file available
    }
    else if (language == "Norwegian") {
        patternFile.setFileName(":/hyph/hyph_nb_NO.dic");
        //patternFile.setFileName(":/hyph/hyph_nn_NO.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Polish") {
        patternFile.setFileName(":/hyph/hyph_pl_PL.dic");
        patternEncoding = "ISO-8859-2";
    }
    else if (language == "Portuguese") {
        patternFile.setFileName(":/hyph/hyph_pt_PT.dic");
        //patternFile.setFileName(":/hyph/hyph_pt_BR.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Russian") {
        patternFile.setFileName(":/hyph/hyph_ru_RU.dic");
        patternEncoding = "KOI8-R";
    }
    else if (language == "Slovak") {
        patternFile.setFileName(":/hyph/hyph_sk_SK.dic");
        patternEncoding = "ISO-8859-2";
    }
    else if (language == "Slowenian") {
        patternFile.setFileName(":/hyph/hyph_sl_SI.dic");
        patternEncoding = "ISO-8859-2";
    }
    else if (language == "Spanish") {
        patternFile.setFileName(":/hyph/hyph_es_ES.dic");
        patternEncoding = "ISO-8859-1";
    }
    else if (language == "Swedish") {
        patternFile.setFileName(":/hyph/hyph_sv_SE.dic");
        patternEncoding = "ISO-8859-2";
    }
    else if (language == "Turkish") {
        // no pattern file available
    }

    else {
        //
    }

    //

    if (patternFile.exists()) {
        if (patternFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&patternFile);
            //in.setCodec(QTextCodec::codecForName("UTF-8"));
            in.setCodec(QTextCodec::codecForName(patternEncoding.toLatin1()));
            QString patterns = in.readAll();
            QString lyrics = ui->plainTextEdit_InputLyrics->toPlainText();
            QStringList words = lyrics.split(QRegExp("\\s"));
            foreach (QString word, words) {
                QString currentWord = word.toLower();
                int wordLength = currentWord.length();
                int patternCount = 0;
                QStringList patternList;
                int hyphenIndicators[wordLength-1];
                for (int h = 0; h < (wordLength-1); h++) {
                    hyphenIndicators[h] = 0;
                }
                for (int i = 0; i < (wordLength-1); i++) {
                    QString subString = currentWord.mid(i, wordLength-i);
                    //ui->plainTextEdit_OutputLyrics->appendPlainText("subString = " + subString + "\n");
                    int subStringLength = subString.length();
                    // pattern must start with whitespace (beginning of line)
                    QString regExpString = "\\s";
                    // for the beginning of the word, a '.' must occur, possibly followed by an integer
                    if (i == 0) {
                        regExpString += "\\.";
                    }
                    // otherwise, no '.', but possibly an integer
                    else {
                        regExpString += "\\d?";
                    }
                    // first two letters of the substring are mandatory
                    regExpString += subString.mid(0,1) + "\\d?";
                    regExpString += subString.mid(1,1) + "\\d?";

                    // the following letters are optional (Spanish syllabification patterns don't have '.' at the end of words
                    QString endString = "";
                    if (language == "Spanish") {
                        endString += "\\.?";
                    }
                    else {
                        endString += "\\.";
                    }
                    for (int j = 2; j <= (subStringLength-1); j++) {
                        regExpString += "(" + subString.mid(j,1);
                        if (j != (subStringLength-1)) {
                            regExpString += "\\d?";
                        }
                        endString += ")?";
                    }
                    regExpString += endString;
                    // pattern must end with whitespace (end of line)
                    regExpString += "\\s";
                    //ui->plainTextEdit_OutputLyrics->appendPlainText("regExpString = " + regExpString + "\n");
                    int patternSearchPos = 0;
                    int patternStartPos = patterns.indexOf(QRegExp(regExpString), patternSearchPos);
                    while (patternStartPos != -1) {
                        patternCount++;
                        int patternEndPos = patterns.indexOf(QRegExp("\\s"), patternStartPos+1);
                        int patternLength = patternEndPos - patternStartPos - 1;
                        QString pattern = patterns.mid(patternStartPos+1,patternLength);
                        pattern = pattern.remove('.');
                        patternLength = pattern.length();
                        patternList << pattern;

                        QString output = word.mid(0,1);
                        for (int z = 0; z < (wordLength-1); z++) {
                            if (hyphenIndicators[z]%2 != 0) {
                                output += "+";
                                output += word.mid(z+1,1);
                            }
                            else {
                                output += word.mid(z+1,1);
                            }
                        }
                        ui->plainTextEdit_OutputLyrics->appendPlainText(output);

                        ui->plainTextEdit_OutputLyrics->appendPlainText(pattern);


                        // copy hyphen indicators into hyphenIndicators for current pattern
                        int numChars = 0;
                        for (int k = 0; k < patternLength; k++) {
                            if (pattern.at(k).isDigit()) {
                                if (hyphenIndicators[i+numChars-1] < pattern.at(k).digitValue()) {
                                    hyphenIndicators[i+numChars-1] = pattern.at(k).digitValue();
                                }
                            }
                            else {
                                numChars++;
                            }
                        }

                        // step ahead for next search
                        patternSearchPos = patternStartPos+patternLength;
                        // search again
                        patternStartPos = patterns.indexOf(QRegExp(regExpString), patternSearchPos);

                    }
                }
                // don't allow a single character at the end
                hyphenIndicators[wordLength-2] = 0;

                QString output = word.mid(0,1);
                for (int z = 0; z < (wordLength-1); z++) {
                    if (hyphenIndicators[z]%2 != 0) {
                        output += "+";
                        output += word.mid(z+1,1);
                    }
                    else {
                        output += word.mid(z+1,1);
                    }
                }
                ui->plainTextEdit_OutputLyrics->appendPlainText(output);
            }
        }
    }
    else {
        QUMessageBox::warning(this, tr("Application"),
            tr("Pattern file not available."));
    }
}



void QCMainWindow::on_pushButton_EnableBPMEdit_toggled(bool checked)
{
    ui->doubleSpinBox_BPM->setReadOnly(!checked);
    if(checked){
        ui->pushButton_EnableBPMEdit->setIcon(QIcon(":/types/lock-unlock.png"));
    }
    else {
        ui->pushButton_EnableBPMEdit->setIcon(QIcon(":/types/lock.png"));
    }
}

void QCMainWindow::on_doubleSpinBox_BPM_valueChanged(double BPMValue)
{
    BPM = BPMValue;

    if (BPM <= 50) {
        BPM = BPM*8;
    }
    else if (BPM <= 100) {
        BPM = BPM*4;
    }
    else if (BPM <= 200) {
        BPM = BPM*2;
    }
}

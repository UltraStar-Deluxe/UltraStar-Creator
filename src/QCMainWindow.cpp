#include "main.h"

#include "QCMainWindow.h"
#include "QULogService.h"
#include "QUMessageBox.h"
#include "QUAboutDialog.h"

#include <QFileDialog>
#include <QTextStream>
#include <QClipboard>
#include <QUrl>

// global variables
static bool firstNote = true;
QString currentSyllable;
qint32 currentNoteStartTime;
qint32 currentNoteStartBeat;
qint32 firstNoteStartBeat = 0;
static double bpm;
QString lyricsString;
QStringList lyricsStringList;
qint32 numSyllables = 0;
QClipboard *clipboard = QApplication::clipboard();
qint32 currentSyllableGlobalIndex = 0;
qint32 currentCharacterIndex = 0;
QString filename_MP3;

QCMainWindow::QCMainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::QCMainWindow) {

    ui->setupUi(this);
    setWindowTitle(tr("UltraStar Song Creator 0.1"));
    //(void*)statusBar();
    //statusBar()->showMessage(tr("USC ready."));
    logSrv->add(tr("Ready."), QU::Information);
}

void QCMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
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

void QCMainWindow::on_pushButton_Start_clicked()
{
    QWidget::setAcceptDrops(false);
    //statusBar()->showMessage(tr("USC Tapping."));
    ui->plainTextEdit_OutputLyrics->setPlainText("");
    ui->groupBox_SongMetaInformationTags->setDisabled(true);
    ui->groupBox_MP3ArtworkTags->setDisabled(true);
    ui->groupBox_MiscSettings->setDisabled(true);
    ui->groupBox_VideoTags->setDisabled(true);
    ui->groupBox_InputLyrics->setDisabled(true);
    ui->groupBox_OutputLyrics->setEnabled(true);
    ui->pushButton_Start->setDisabled(true);
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
        ui->label_CoverSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
    if (ui->lineEdit_Background->text().isEmpty()) {
        ui->lineEdit_Background->setText(tr("%1 - %2 [BG].jpg").arg(ui->lineEdit_Artist->text()).arg(ui->lineEdit_Title->text()));
        ui->label_BackgroundSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }

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

    bpm = ui->doubleSpinBox_BPM->value();
    lyricsString = ui->plainTextEdit_InputLyrics->toPlainText();
    // split string by /n into lines, trim lines, capitalize first word
    // split lines into syllables
    // ???
    // starting with first line, iterate over syllables
    // if last syllable --> add linebreak in output lyrics, go to next line
    lyricsStringList = lyricsString.split(QRegExp("[ +\\n]"), QString::SkipEmptyParts);

    numSyllables = lyricsStringList.length();

    if (numSyllables > 5) {
        ui->pushButton_Tap->setText(lyricsStringList[currentSyllableGlobalIndex]);
        ui->pushButton_NextSyllable1->setText(lyricsStringList[currentSyllableGlobalIndex+1]);
        ui->pushButton_NextSyllable2->setText(lyricsStringList[currentSyllableGlobalIndex+2]);
        ui->pushButton_NextSyllable3->setText(lyricsStringList[currentSyllableGlobalIndex+3]);
        ui->pushButton_NextSyllable4->setText(lyricsStringList[currentSyllableGlobalIndex+4]);
        ui->pushButton_NextSyllable5->setText(lyricsStringList[currentSyllableGlobalIndex+5]);
    }

    // start mp3..

    currentSongTimer.start();
}

void QCMainWindow::on_pushButton_Tap_pressed()
{
    currentNoteTimer.start();
    //statusBar()->showMessage(tr("USC Note Start."));
    currentNoteStartTime = currentSongTimer.elapsed();
    currentNoteStartBeat = currentNoteStartTime * (bpm / 15000);
}

void QCMainWindow::on_pushButton_Tap_released()
{
    qint32 currentNoteTimeLength = currentNoteTimer.elapsed();
    //statusBar()->showMessage(tr("USC Note End."));
    qint32 currentNoteBeatLength = qMax(1.0, currentNoteTimeLength * (bpm / 15000));
    if (firstNote){
        firstNoteStartBeat = currentNoteStartBeat;
        ui->plainTextEdit_OutputLyrics->appendPlainText(tr("#GAP:%1").arg(QString::number(currentNoteStartTime)));
        ui->spinBox_Gap->setValue(currentNoteStartTime);
        firstNote = false;
    }
    currentSyllable = tr(": %1 %2 %3 %4").arg(QString::number(currentNoteStartBeat - firstNoteStartBeat)).arg(QString::number(currentNoteBeatLength)).arg(QString::number(ui->spinBox_DefaultPitch->value())).arg(lyricsStringList[currentSyllableGlobalIndex]);
    ui->plainTextEdit_OutputLyrics->appendPlainText(currentSyllable);

    if ((currentSyllableGlobalIndex+1) < numSyllables) {
        currentSyllableGlobalIndex++;

        ui->pushButton_Tap->setText(lyricsStringList[currentSyllableGlobalIndex]);
        if (currentSyllableGlobalIndex+1 < numSyllables) {
            ui->pushButton_NextSyllable1->setText(lyricsStringList[currentSyllableGlobalIndex+1]);
        }
        else {
            ui->pushButton_NextSyllable1->setText("");
        }
        if (currentSyllableGlobalIndex+2 < numSyllables) {
            ui->pushButton_NextSyllable2->setText(lyricsStringList[currentSyllableGlobalIndex+2]);
        }
        else {
            ui->pushButton_NextSyllable2->setText("");
        }
        if (currentSyllableGlobalIndex+3 < numSyllables) {
            ui->pushButton_NextSyllable3->setText(lyricsStringList[currentSyllableGlobalIndex+3]);
        }
        else {
            ui->pushButton_NextSyllable3->setText("");
        }
        if (currentSyllableGlobalIndex+4 < numSyllables) {
            ui->pushButton_NextSyllable4->setText(lyricsStringList[currentSyllableGlobalIndex+4]);
        }
        else {
            ui->pushButton_NextSyllable4->setText("");
        }
        if (currentSyllableGlobalIndex+5 < numSyllables) {
            ui->pushButton_NextSyllable5->setText(lyricsStringList[currentSyllableGlobalIndex+5]);
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
    QString inputLyrics = clipboard->text();
    // delete leading and trailing whitespace
    inputLyrics = inputLyrics.trimmed();
    // delete surplus blank lines...

    // replace misused accents by the correct apostrophe
    inputLyrics = inputLyrics.replace("´", "'");
    inputLyrics = inputLyrics.replace("`", "'");

    ui->plainTextEdit_InputLyrics->setPlainText(inputLyrics);
}

void QCMainWindow::on_pushButton_CopyToClipboard_clicked()
{
    clipboard->setText(ui->plainTextEdit_OutputLyrics->toPlainText());
}

void QCMainWindow::on_pushButton_Stop_clicked()
{
    ui->plainTextEdit_OutputLyrics->appendPlainText("E");
    ui->plainTextEdit_OutputLyrics->appendPlainText("");

    //statusBar()->showMessage(tr("USC ready."));

    ui->pushButton_Stop->setDisabled(true);
    ui->groupBox_TapArea->setDisabled(true);
    ui->pushButton_SaveToFile->setEnabled(true);
    ui->pushButton_CopyToClipboard->setEnabled(true);
    QWidget::setAcceptDrops(true);
}

void QCMainWindow::on_pushButton_BrowseMP3_clicked()
{
    filename_MP3 = QFileDialog::getOpenFileName ( 0, tr("Please choose MP3 file"), QDir::homePath(), tr("Audio files (*.mp3 *.ogg)"));
    QFileInfo *fileInfo_MP3 = new QFileInfo(filename_MP3);
    if (!fileInfo_MP3->fileName().isEmpty()) {
        ui->lineEdit_MP3->setText(fileInfo_MP3->fileName());
        ui->label_MP3Set->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->pushButton_Start->setEnabled(true);
    }
}

void QCMainWindow::on_pushButton_BrowseCover_clicked()
{
    QString filename_Cover = QFileDialog::getOpenFileName ( 0, tr("Please choose cover image file"), QDir::homePath(), tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Cover = new QFileInfo(filename_Cover);
    if (!fileInfo_Cover->fileName().isEmpty()) {
        ui->lineEdit_Cover->setText(fileInfo_Cover->fileName());
        ui->label_CoverSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
}

void QCMainWindow::on_pushButton_BrowseBackground_clicked()
{
    QString filename_Background = QFileDialog::getOpenFileName ( 0, tr("Please choose background image file"), QDir::homePath(), tr("Image files (*.jpg)"));
    QFileInfo *fileInfo_Background = new QFileInfo(filename_Background);
    if (!fileInfo_Background->fileName().isEmpty()) {
        ui->lineEdit_Background->setText(fileInfo_Background->fileName());
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
                if (!fileName.isEmpty() && fileName.endsWith(tr(".txt"))) {
                    QFile file(fileName);
                    if (file.open(QFile::ReadOnly | QFile::Text)) {
                        ui->plainTextEdit_InputLyrics->setPlainText(file.readAll());
                    }
                }
                else if (!fileName.isEmpty() && (fileName.endsWith(tr(".mp3")) || fileName.endsWith(tr(".ogg")))) {
                    filename_MP3 = fileName;
                    QFileInfo *fileInfo_MP3 = new QFileInfo(fileName);
                    if (!fileInfo_MP3->fileName().isEmpty()) {
                        ui->lineEdit_MP3->setText(fileInfo_MP3->fileName());
                        ui->label_MP3Set->setPixmap(QPixmap(":/marks/path_ok.png"));
                        ui->pushButton_Start->setEnabled(true);
                    }
                }
                else if (!fileName.isEmpty() && fileName.endsWith(tr(".jpg"))) {
                    QFileInfo *fileInfo = new QFileInfo(fileName);
                    QMessageBox msg(QMessageBox::Question, tr("Image file drop"), tr("Use '%1' as...").arg(fileInfo->fileName()));
                    QPushButton *coverButton = msg.addButton(tr("Cover"), QMessageBox::YesRole);
                    QPushButton *backgroundButton = msg.addButton(tr("Background"), QMessageBox::NoRole);
                    msg.exec();
                    if (msg.clickedButton() == coverButton) {
                        QFileInfo *fileInfo_Cover = new QFileInfo(fileName);
                        if (!fileInfo_Cover->fileName().isEmpty()) {
                            ui->lineEdit_Cover->setText(fileInfo_Cover->fileName());
                            ui->label_CoverSet->setPixmap(QPixmap(":/marks/path_ok.png"));
                        }
                    }
                    else if (msg.clickedButton() == backgroundButton) {
                        QFileInfo *fileInfo_Background = new QFileInfo(fileName);
                        if (!fileInfo_Background->fileName().isEmpty()) {
                            ui->lineEdit_Background->setText(fileInfo_Background->fileName());
                            ui->label_BackgroundSet->setPixmap(QPixmap(":/marks/path_ok.png"));
                        }
                    }
                }
                else if (!fileName.isEmpty() && (fileName.endsWith(tr(".avi")) || fileName.endsWith(tr(".flv")) || fileName.endsWith(tr(".mpg")) || fileName.endsWith(tr(".mpeg")) || fileName.endsWith(tr(".mp4")) || fileName.endsWith(tr(".vob")) || fileName.endsWith(tr(".ts")))) {
                    if (!ui->groupBox_VideoTags->isChecked()) {
                        ui->groupBox_VideoTags->setChecked(true);
                    }
                    QFileInfo *fileInfo_Video = new QFileInfo(fileName);
                    if (!fileInfo_Video->fileName().isEmpty()) {
                        ui->lineEdit_Video->setText(fileInfo_Video->fileName());
                        ui->label_VideoSet->setPixmap(QPixmap(":/marks/path_ok.png"));
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

void QCMainWindow::on_lineEdit_Title_editingFinished()
{
    if(!ui->lineEdit_Title->text().isEmpty()) {
        ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->lineEdit_Artist->setFocus();
    }
    else {
        ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_lineEdit_Artist_editingFinished()
{
    if(!ui->lineEdit_Artist->text().isEmpty()) {
        ui->label_ArtistSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->comboBox_Language->setFocus();
    }
    else {
        ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_comboBox_Language_currentIndexChanged(QString language)
{
    if(!language.isEmpty()) {
        ui->label_LanguageSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
    else {
        ui->label_LanguageSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_comboBox_Edition_currentIndexChanged(QString edition)
{
    if(!edition.isEmpty()) {
        ui->label_EditionSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
    else {
        ui->label_EditionSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_comboBox_Genre_currentIndexChanged(QString genre)
{
    if(!genre.isEmpty()) {
        ui->label_GenreSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
    else {
        ui->label_GenreSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_comboBox_Year_currentIndexChanged(QString year)
{
    if(!year.isEmpty()) {
        ui->label_YearSet->setPixmap(QPixmap(":/marks/path_ok.png"));
    }
    else {
        ui->label_YearSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_lineEdit_Creator_editingFinished()
{
    if(!ui->lineEdit_Creator->text().isEmpty()) {
        ui->label_CreatorSet->setPixmap(QPixmap(":/marks/path_ok.png"));
        ui->doubleSpinBox_BPM->setFocus();
    }
    else {
        ui->label_TitleSet->setPixmap(QPixmap(":/marks/path_error.png"));
    }
}

void QCMainWindow::on_doubleSpinBox_BPM_editingFinished()
{
     ui->pushButton_BrowseMP3->setFocus();
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

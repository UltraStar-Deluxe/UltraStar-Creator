#include "QUMonty.h"
#include "QULogService.h"
//#include "QUSongDatabase.h"

#include <QFile>
#include <QSettings>
#include <QPixmap>
#include <QFileInfo>
#include <QRegExp>

QUMonty::QUMonty() {
	initMessages();

	_watcher = new QFileSystemWatcher();
}

QUMonty::~QUMonty() {
	delete _watcher;
}

void QUMonty::initMessages() {
	messages << QObject::tr("Do you want me to say something else? Tell everybody about your requests on our <a href=\"https://github.com/UltraStar-Deluxe/UltraStar-Creator/issues\">project page</a>.");
	messages << QObject::tr("The BPM tag is not editable? Yeah, that's right. The BPM is automatically determined! If you use an external program for BPM detection (e.g. <a href=\"http://www.mixmeister.com/bpmanalyzer/bpmanalyzer.asp\">Mixmeister BPM Analyzer</a>), you can click on the little padlock to be able to edit a custom value.");
	messages << QObject::tr("Did I hear you correctly? A missing feature?! Just click <a href=\"https://github.com/UltraStar-Deluxe/UltraStar-Creator/issues\">here</a> and let everybody know - especially the developers!");
	messages << QObject::tr("Did you notice that huge amount of default values for song genre? This list of genres originates from the good old ID3v1 specification as well as some custom Winamp extensions.");
	messages << QObject::tr("I shall be quiet? Then click on <i>Hide</i> next to my speech bubble and I will disappear. To let me talk again, simply click the little button with the question mark in the upper right corner or simply hit <i>F1</i>.");
	messages << QObject::tr("The default values for Artist, Title and Year in the <i>Song Header</i> are read from the ID3 tag, if available.");
	messages << QObject::tr("If you drag & drop an audio file from your explorer onto the UltraStar Creator window, it will be used as your new work!");
	messages << QObject::tr("If you drag & drop a text file from your explorer onto the UltraStar Creator window, it will be used as the song lyrics!");
	messages << QObject::tr("You are not sure in which year your favorite song was released? Well, why don't you look it up on <a href=\"swisscharts.com\">swisscharts.com</a> using the little globe icon on the left? Just make sure <i>Artist</i> and <i>Title</i> fields contain valid entries.");
	messages << QObject::tr("The song is too fast for you and you can't keep up with the tapping?? Don't worry, you can change the playback speed under <i>Options &rarr; Playback speed</i>, even after the tapping has already started.");
	messages << QObject::tr("Are you using the latest version of UltraStar Creator? Well, why don't you check by clicking <i>About &rarr; Check for Update</i>? You can enable an automatic update check at startup in the resulting dialog.");
	messages << QObject::tr("You finished tapping your song and saved it successfully? Now you need to finetune your song by setting correct pitches, adjusting note lengths and timings in the Ultrastar Editor or Yass Editor using the little icons below the <i>Generated Ultrastar file</i> box.");
	messages << QObject::tr("UltraStar Creator is not available in your language? Well, if you are interested in translating, just get in touch with one of the developers at the <a href=\"https://github.com/UltraStar-Deluxe/UltraStar-Creator\">project site</a>.");
	messages << QObject::tr("Did you remark that the state of the application window is saved on exit?");
    messages << QObject::tr("Resulting UltraStar files are saved with UTF-8 encoding. If you use older version of UltraStar, you might need to convert it to a different encoding such as CP1252.");
	messages << QObject::tr("You can change the size of the three main columns of <i>Song Header</i>, <i>Song Lyrics</i>, and <i>UltraStar file</i> by clicking between adjacent columns and dragging to left or right.");
	messages << QObject::tr("Once the song header information is filled out and the song lyrics are properly split into syllables, you are ready to tap the song. Simply hit the green Play button or press <i>P</i>.");
	messages << QObject::tr("UltraStar Creator offers automatic syllabification for English, German, and Spanish songs. Just make sure a proper language is set in the song header before clicking on the little syllabification icon below the input lyrics. Although this feature is not perfect, it can save you a lot of time and you only need to change the result here and there.");
	messages << QObject::tr("Did you tap a few syllables too much? No problem, just hit the little undo arrow to the left of the tap button or simply press <i>X</i> to undo the last tap while simultaneously rewinding the audio by one second.");
	messages << QObject::tr("The tapping process can be paused and resumed by clicking the Pause/Play button or by simply pressing <i>P</i>.");
	messages << QObject::tr("Messed up and want to start over? Just stop tapping by clicking on the Stop button and then on the Rewind button. Or simply press <i>S</i> and then <i>R</i><br><br>Practice does indeed make perfect!");
	messages << QObject::tr("To tap a syllable, simply click on the tap button as long as the syllable is sung. Alternatively, you can use the <i>Space</i> bar.");
	messages << QObject::tr("Superfluous empty lines and whitespace will be removed from the input lyrics automatically once you start tapping.");
	messages << QObject::tr("Automatic capitalization of beginnings of lyric lines can be toggled through the little toolbutton below the input lyrics. All line beginnings will be capitalized when you start tapping.");
	messages << QObject::tr("If you put your audio file as well as cover and background image or video file all in one folder before selecting it as your work from within UltraStar Creator, I will try to assign the files automatically to the corresponding tags. If I get confused, you can always select the correct image or video from the corresponding dropdown menu.");
	messages << QObject::tr("Unfortunately, I can't help you set the note pitches for the tapped syllables, but at least the default pitch for every note can be configured under <i>Options &rarr; Default pitch</i>.");
	messages << QObject::tr("There is a little <i>Preview Play</i> button directly below the selected audio file. You can use this preview to check the song's lyrics for mistakes and adjust the syllabification.");
	messages << QObject::tr("Freestyle text files??? Well, point me to a folder full of your favorite songs files and I will generate UltraStar compatible text files (without any lyrics) for them. Each MP3 will be moved into a separate subfolder. If the audio files follow an 'Artist - Title.mp3' naming scheme, they will be correctly mapped in the resulting song file.");
}

QUMonty* QUMonty::_instance = 0;
QUMonty* QUMonty::instance() {
	if(_instance == 0)
		_instance = new QUMonty();

	return _instance;
}

QPixmap QUMonty::pic(QUMonty::Status status) {
	switch(status) {
	case QUMonty::normal:
		return QPixmap(QString(":/monty/normal.png")).scaledToHeight(96, Qt::SmoothTransformation);
	case QUMonty::back:
		return QPixmap(QString(":/monty/back.png")).scaledToHeight(96, Qt::SmoothTransformation);
	case QUMonty::happy:
		return QPixmap(QString(":/monty/happy.png")).scaledToHeight(96, Qt::SmoothTransformation);
	case QUMonty::seated:
		return QPixmap(QString(":/monty/seated.png")).scaledToHeight(96, Qt::SmoothTransformation);
	default:
		return QPixmap();
	}
}

QString QUMonty::welcomeMsg() {
	QString welcomeStr(QObject::tr("Hello! I am Monty the Mammoth. I will tell you some hints from time to time.<br><br>To get started, open a song file and fill out missing information in the song header. Then paste the song lyrics into the corresponding field, syllabificate as needed and then start tapping by pressing <i>Play</i>."));

	return welcomeStr;
}

void QUMonty::talk(QLabel *montyLbl, QLabel *msgLbl) {
	montyLbl->setPixmap(pic((QUMonty::Status)(qrand() % 4)));
	QString message = messages[qrand() % messages.size()];
	msgLbl->setText(message);
}

/*!
 * Searches for hints to the given question.
 */
void QUMonty::answer(QLabel *montyLbl, QLabel *msgLbl, const QString &question, bool prev) {
	static QStringList answers;

	if(!question.isEmpty()) {
		answers.clear(); // process new question

		QRegExp rx(question, Qt::CaseInsensitive);
		foreach(QString msg, messages) {
			if(msg.contains(rx))
				answers.append(msg);
		}
	}

	if(answers.isEmpty()) {
		montyLbl->setPixmap(pic((QUMonty::Status)(qrand() % 4)));
		msgLbl->setText(QObject::tr("Sorry. I don't understand you."));
		return;
	}

	// show next answer
	QString out = prev ? answers.last() : answers.first();
	if(!prev) {
		answers.removeFirst();
		answers.append(out);
	} else {
		answers.removeLast();
		answers.prepend(out);
	}

	montyLbl->setPixmap(pic((QUMonty::Status)(qrand() % 4)));
	msgLbl->setText(answers.first());
}

bool QUMonty::autoSaveEnabled() const {
	QSettings settings;
	return settings.value("autoSave", QVariant(true)).toBool();
}

/*!
 * Extract a resource (e.g. an image/icon) to a subdirectory of the report output
 * path and return a relative path to that resource.
 * \param item The resource to extract (e.g. ":/accept.png")
 * \param dest Directory of the report, NOT the subdirectory for the image
 * \returns Relative local path to the resource.
 */
QString QUMonty::useImageFromResource(const QString &item, QDir dest) {
	if(!dest.cd("images")) {
		dest.mkdir("images");
		if(!dest.cd("images")) {
//			logSrv->add(tr("Subdirectory for images could not be created!"), QU::Warning);
			return QString();
		}
	}

	QPixmap pixmap(item);
	QFileInfo fi(dest, QFileInfo(item).fileName());

	if(fi.exists()) {
		dest.cdUp();
		return dest.relativeFilePath(fi.filePath());
	}

	if(!pixmap.save(fi.filePath())) {
//		logSrv->add(QString(tr("The resource file \"%1\" could NOT be saved.")).arg(fi.filePath()), QU::Warning);
		return QString();
	}

//	logSrv->add(QString(tr("The resource file \"%1\" was extracted successfully.")).arg(fi.filePath()), QU::Information);

	dest.cdUp();
	return dest.relativeFilePath(fi.filePath());
}

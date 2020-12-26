#include "main.h"
#include "QUMainWindow.h"
#include "QUMessageBox.h"

#include <QApplication>
#include <QDateTime>
#include <QTranslator>
#include <QSettings>
#include <QSplashScreen>
#include <QPixmap>
#include <QString>
#include <QFile>
#include <QLocale>
#include <QFileInfo>
//#include <QRandomGenerator>

void initApplication();
void initLanguage(QApplication&, QTranslator&, QTranslator&, QSplashScreen&);
void handlePreviousAppCrash();
void handleWipWarning();
void handleReleaseCandidateInformation();
void handleArguments();

/*!
 * This is the main entry point of the application
 *
 */
int main(int argc, char *argv[]) {
	initApplication();

	QApplication app(argc, argv);
	QTranslator trContent;
	QTranslator trQt;

	QSplashScreen splash(QPixmap(":/splash/splash.png"));
	splash.show();

	initLanguage(app, trContent, trQt, splash);

	#ifdef WIP_VERSION
	handleWipWarning();
	#endif // WIP_VERSION

	#ifdef RC_VERSION
	handleReleaseCandidateInformation();
	#endif // RC_VERSION

	handlePreviousAppCrash();
	handleArguments();

	QUMainWindow mainWindow;
	app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

	mainWindow.show();
	splash.finish(&mainWindow);

	return app.exec();
}

/*!
 * Initializes general aspects of this application like resources,
 * random timers and registry setups.
 */
void initApplication() {
	Q_INIT_RESOURCE(UltraStar_Creator);

	//QRandomGenerator(QDateTime::currentDateTime().toTime_t());

	QCoreApplication::setOrganizationName("HPI");
	QCoreApplication::setApplicationName("UltraStar Creator");
}

/*!
 * Installs a proper translator according to the registry settings. That's why you
 * have to restart this application if you want to change its language. Uses the system
 * language if no registry setting is found.
 *
 * Note that there is no special translation file for English present. That's why all
 * strings in the application source code is in english.
 */
void initLanguage(QApplication &app, QTranslator &trContent, QTranslator &trQt, QSplashScreen &s) {
	QSettings settings;
	bool settingFound;
	QLocale lang;

	if(!settings.contains("language")) {
			settingFound = false;
			lang = QLocale::system();
			settings.setValue("language", lang.name()); // remember this language (system local)
	} else {
			settingFound = true;
			lang = QLocale(settings.value("language").toString());
	}

	if (lang.language() == QLocale::French) {
		if(trContent.load(":/UltraStar-Creator.fr.qm")) {
			app.installTranslator(&trContent);
		}
		if(trQt.load(":/qt_fr.qm")) {
			app.installTranslator(&trQt);
		}
	} else if (lang.language() == QLocale::German) {
		if(trContent.load(":/UltraStar-Creator.de.qm")) {
			app.installTranslator(&trContent);
		}
		if(trQt.load(":/qt_de.qm")) {
			app.installTranslator(&trQt);
		}
	} else if (lang.language() == QLocale::Polish) {
		if(trContent.load(":/UltraStar-Creator.pl.qm")) {
			app.installTranslator(&trContent);
		}
		if(trQt.load(":/qt_pl.qm")) {
			app.installTranslator(&trQt);
		}
	} else if (lang.language() == QLocale::Spanish) {
		if(trContent.load(":/UltraStar-Creator.es.qm")) {
			app.installTranslator(&trContent);
		}
		if(trQt.load(":/qt_es.qm")) {
			app.installTranslator(&trQt);
		}
	} else if (lang.language() == QLocale::Portuguese) {
		if(trContent.load(":/UltraStar-Creator.pt.qm")) {
			app.installTranslator(&trContent);
		}
		if(trQt.load(":/qt_pt.qm")) {
			app.installTranslator(&trQt);
		}
	}

	s.showMessage(QString(QObject::tr("%1.%2.%3 is loading...")).arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION), Qt::AlignBottom | Qt::AlignRight, Qt::white);

	// message needs to be here because it can be translated only after installing the translator
	if(!settingFound)
			QUMessageBox::information(0,
							QObject::tr("Init Language"),
							QObject::tr("There was no configuration setting about your language. Your <b>system language</b> will be used if possible.<br>"
											"<br>"
											"Choose <b>Language</b> from the menu to change this."),
							BTN << ":/icons/accept.png" << QObject::tr("Continue."),
							270);
}

/*!
 * This function displays a warning about the ongoing work on this software
 * and that it should not be distributed.
 */
void handleWipWarning() {
	QUMessageBox::warning(0,
							QObject::tr("WIP version detected!"),
							QObject::tr("This version is still under development <b>and may not be very stable</b>. <br><br><b>Please</b> do not distribute this version."),
							BTN << ":/icons/accept.png" << QObject::tr("I will be careful."));
}

/*!
 * This function displays a message that this software is a release candidate version.
 *
 */
void handleReleaseCandidateInformation() {
	QUMessageBox::information(0,
							QObject::tr("Release Candidate Information"),
							QObject::tr("<b>Dear testing person,</b><br><br>this version is meant to be <b>almost bug-free</b>. Nevertheless, it might be possible that some errors are still remaining.<br> While testing all these cool new features of version %1.%2.%3, consider the following:<br><br>- xxx<br>- xxx<br>- xxx<br>- xxx<br>- xxx<br>- xxx<br><br>Provide feedback and bug reports at: <a href=\"https://github.com/UltraStar-Deluxe/UltraStar-Creator/issues\">GitHub</a>").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION),
							BTN << ":/icons/accept.png" << QObject::tr("Okay!"),
							400);
}

/*!
 * This function handles a previous crash of the application by detecting
 * if the file 'running.app' exists in the working directory
 */
void handlePreviousAppCrash() {
	if(!QFile::exists("running.app")) {
			QFile f("running.app");
			f.open(QIODevice::WriteOnly);
			f.close();

			return; // everything is ok
	}

	QUMessageBox::critical(0,
		QObject::tr("Application Crash Detected"),
		QObject::tr("The UltraStar Creator did not exit successfully last time. <br>"
						"<br>"
						"Please report this problem <a href=\"https://github.com/UltraStar-Deluxe/UltraStar-Creator/issues\">here</a>."),
		BTN	<< ":/icons/accept.png"		<< QObject::tr("Try again."));
}

/*!
 * This function handles the input arguments of the application.
 * So far, an MP3 file can be passed to it as the song file to be tapped.
 */
void handleArguments() {
	QSettings settings;
	foreach(QString arg, qApp->arguments()) {
			QFileInfo fi(arg);

			if(fi.isFile() && (fi.fileName().endsWith(".mp3") || fi.fileName().endsWith(".ogg"))) {
				// use argument as audio file
				settings.setValue("songfile", fi.absoluteFilePath());
			}

			if(fi.isFile() && fi.fileName().endsWith(".txt")) {
			// use argument as input lyrics file
			}

			// add cover/background and video files here

			if(fi.isDir() && fi.exists()) {
					settings.setValue("songPath", arg);
					break;
			}
	}
}

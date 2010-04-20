#include "main.h"
#include "QCMainWindow.h"
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

void initApplication();
void initLanguage(QApplication&, QTranslator&, QSplashScreen&);
void handlePreviousAppCrash();
void handleWipWarning();
void handleReleaseCandidateInformation();
void handleArguments();

int main(int argc, char *argv[]) {
    initApplication();

    QApplication app(argc, argv);
    QTranslator tr;

    QSplashScreen splash(QPixmap(":/icons/splash.png"));
    splash.show();

    initLanguage(app, tr, splash);

    #ifdef WIP_VERSION
    handleWipWarning();
    #endif // WIP_VERSION

    #ifdef RC_VERSION
    handleReleaseCandidateInformation();
    #endif // RC_VERSION

    handlePreviousAppCrash();
    handleArguments();

    QCMainWindow mainWindow;
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
	Q_INIT_RESOURCE(usc);

	qsrand(QDateTime::currentDateTime().toTime_t());

	QCoreApplication::setOrganizationName("HPI");
	QCoreApplication::setApplicationName("UltraStar Creator");
}

/*!
 * Installs a proper translator according to the registry settings. That's why you
 * have to restart this application if you want to change its language. Uses the system
 * language if no registry setting is found.
 *
 * Note that there is no special translation file for english present. That's why all
 * strings in the application source code is in english.
 */
void initLanguage(QApplication &app, QTranslator &t, QSplashScreen &splash) {
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

        if(lang.language() == QLocale::German) {
                if(t.load(":/lang/usc.de.qm")) {
                        app.installTranslator(&t);
                        //monty->initMessages(":/txt/hints_de");
                }
        } else if (lang.language() == QLocale::Polish) {
                if(t.load(":/lang/usc.pl.qm")) {
                        app.installTranslator(&t);
                        //monty->initMessages(":/txt/hints_pl");
                }
        }

        splash.showMessage(QString(QObject::tr("%1.%2.%3 is loading...")).arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(PATCH_VERSION), Qt::AlignBottom | Qt::AlignRight, Qt::white);

        // message needs to be here because it can be translated only after installing
        // the translator
        if(!settingFound)
                QUMessageBox::information(0,
                                QObject::tr("Init Language"),
                                QObject::tr("There was no configuration setting about your language. Your <b>system language</b> will be used if possible.<br>"
                                                "<br>"
                                                "See <b>Language</b> to change this."),
                                BTN << ":/marks/accept.png" << QObject::tr("Continue."),
                                270);
}

void handleWipWarning() {
        QUMessageBox::warning(0,
                                QObject::tr("WIP version detected!"),
                                QObject::tr("This version is still under development <b>and may not be very stable</b>. <br><br><b>Please</b> do not distribute this version."),
                                BTN << ":/marks/accept.png" << QObject::tr("I will be careful."));
}

void handleReleaseCandidateInformation() {
        QUMessageBox::information(0,
                                QObject::tr("Release Candidate Information"),
                                QObject::tr("<b>Dear testing person,</b><br><br>this version is meant to be <b>feature-complete and almost bug-free</b>. Nevertheless, it might be possible that some errors are still remaining.<br> While testing all these cool new features of version 0.1.0, consider the following:<br><br>- xxx<br>- xxx<br>- xxx<br>- xxx<br>- xxx<br>- xxx<br><br>Send feedback and bug reports to: <a href=\"mailto:xxx@users.sourceforge.net\">xxx@users.sourceforge.net</a>"),
                                BTN << ":/marks/accept.png" << QObject::tr("Okay!"),
                                400);
}

void handlePreviousAppCrash() {
        if(!QFile::exists("running.app")) {
                QFile f("running.app");
                f.open(QIODevice::WriteOnly);
                f.close();

                return; // everything is ok
        }

        int result = QUMessageBox::critical(0,
                        QObject::tr("Application Crash Detected"),
                        QObject::tr("The UltraStar Creator did not exit successfully last time. <br>"
                                        "<br>"
                                        "Please report this problem <a href=\"http://usc.sf.net\">here</a>."),
                        BTN	<< ":/marks/accept.png"        << QObject::tr("Try again."));
}

/*!
 * Read arguments song files to be loaded.
 */
void handleArguments() {
        QSettings s;
        foreach(QString arg, qApp->arguments()) {
                QFileInfo fi(arg);

                if(fi.isFile() && (fi.fileName().endsWith(".mp3") || fi.fileName().endsWith(".ogg"))) {
                    // use argument as audio file
                }

                if(fi.isFile() && fi.fileName().endsWith(".txt")) {
                    // use argument as input lyrics file
                }

                // add cover/background and video files here

                if(fi.isDir() && fi.exists()) {
//			QStringList paths(s.value("songPaths").toStringList());
//			paths.append(arg);
//			paths.removeDuplicates();
//			s.setValue("songPaths", paths);
                        s.setValue("songPath", arg);
                        break;
                }
        }
}

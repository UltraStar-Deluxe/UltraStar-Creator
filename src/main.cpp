#include "main.h"
#include "QCMainWindow.h"

#include <QApplication>
#include <QDateTime>

void initApplication();

int main(int argc, char *argv[]) {
	initApplication();

	QApplication app(argc, argv);
	QCMainWindow mainWindow;

    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    mainWindow.show();

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

#ifndef QU_H_
#define QU_H_

#include <QObject>
#include <QFileInfo>

#define N_A "-"
#define NONE "(none)"

#define CHAR_UTF8_APPROX "\xe2\x89\x88"
#define CHAR_UTF8_NEQUAL "\xe2\x89\xa0"
#define CHAR_UTF8_DOT    "\xc2\xb7"
#define CHAR_UTF8_FLAT   "\xe2\x99\xad"
#define CHAR_UTF8_SHARP  "\xe2\x99\xaf"

//! This class provides global flags and enumerations.
class QU: QObject {
	Q_OBJECT
public:
	QU(QObject *parent = 0);

	static QDir BaseDir;

	enum MessageType {
		Information,
		Warning,
		Help,
		Saving,
		Error
	};
	Q_DECLARE_FLAGS(MessageTypes, MessageType)
};

#endif /*QU_H_*/

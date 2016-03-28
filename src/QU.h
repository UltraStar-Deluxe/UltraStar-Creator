#ifndef QU_H_
#define QU_H_

#include <QObject>
#include <QFileInfo>

#define N_A "-"
#define NONE "(none)"

#define CHAR_UTF8_APPROX "\xe2\x89\x88"
#define CHAR_UTF8_NEQUAL "\xe2\x89\xa0"
#define CHAR_UTF8_DOT    "\xc2\xb7"
#define CHAR_UTF8_SHARP  "\xe2\x99\xaf"
#define CHAR_UTF8_FLAT   "\xe2\x99\xad"
#define CHAR_UTF8_AUML   "\xc3\xa4"
#define CHAR_UTF8_AACUTE "\xc3\xa1"
#define CHAR_UTF8_EACUTE "\xc3\xa9"
#define CHAR_UTF8_IACUTE "\xc3\xad"
#define CHAR_UTF8_OUML   "\xc3\xb6"
#define CHAR_UTF8_OACUTE "\xc3\xb3"
#define CHAR_UTF8_UUML   "\xc3\xbc"
#define CHAR_UTF8_UACUTE "\xc3\xba"
#define CHAR_UTF8_SHARPS "\xc3\x9f"

// used for encodings
#define ENCODING_CP1250	"CP1250"
#define ENCODING_CP1252	"CP1252"
#define ENCODING_UTF8	"UTF8"

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

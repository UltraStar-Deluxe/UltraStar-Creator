#include "QUStringSupport.h"

#include <QRegularExpression>

QUStringSupport::QUStringSupport(QObject *parent): QObject(parent) {}

/*!
 * Removes all characters of the given text that cannot be used in a file or
 * directory name.
 *
 * Path separator '/' is not removed.
 *
 * \param text A single line of text without line breaks.
 */
QString QUStringSupport::withoutUnsupportedCharacters(const QString &text) {
	QString cleanText = text;
#ifdef Q_OS_WIN32
	// replace colons by dashes
	cleanText.replace(':', '-');
	// remove everything else
	cleanText.remove(QRegularExpression("[\\\\:\\*\\?\"\\|<>]"));

	/* MB: commented out as trailing and leading dots do not seem to pose problems in Windows
	// remove trailing and leading dots
	bool dotsRemoved = false;

	while(cleanText.endsWith(".")) {
		dotsRemoved = true;
		cleanText.chop(1);
	}

	while (cleanText.startsWith(".")) {
		dotsRemoved = true;
		cleanText.remove(0, 1);
	}

	if(dotsRemoved)
		cleanText = cleanText.trimmed();
	*/
#endif
	return cleanText;
}

QString QUStringSupport::withoutPathDelimiters(const QString &text) {
	return QString(text).replace("/", "-");
}

/*!
 * Remove all "folder tags" like [SC], [VIDEO], a.s.o. from the given text.
 */
QString QUStringSupport::withoutFolderTags(const QString &text) {
	QRegularExpression rx("\\[.*\\]", QRegularExpression::InvertedGreedinessOption);
	return QString(text).remove(rx).trimmed();
}

/*!
 * Remove all leading spaces and tabs.
 */
QString QUStringSupport::withoutLeadingBlanks(const QString &text) {
	QString result = text;

	while(result.startsWith(" "))
		result.remove(0, 1);

	while(result.startsWith("\t"))
		result.remove(0, 1);

	return result;
}

QString QUStringSupport::withoutAnyUmlaut(const QString &text) {
	QString result = text;

	result.replace("ä", "ae", Qt::CaseInsensitive);
	result.replace("ö", "oe", Qt::CaseInsensitive);
	result.replace("ü", "ue", Qt::CaseInsensitive);
	result.replace("ß", "ss", Qt::CaseInsensitive);

	return result;
}

QString QUStringSupport::withoutAnyUmlautEx(const QString &text) {
	QString result = text;

	result.replace("ä", "ae", Qt::CaseInsensitive);
	result.replace("ö", "oe", Qt::CaseInsensitive);
	result.replace("ü", "ue", Qt::CaseInsensitive);
	result.replace("ß", "ss", Qt::CaseInsensitive);

	result.replace("ô", "o", Qt::CaseInsensitive);
	result.replace("é", "e", Qt::CaseInsensitive);
	result.replace("è", "e", Qt::CaseInsensitive);

	result.replace("_", " ");
	result.replace("-", " ");
	result.replace("~", " ");
	result.replace("#", " ");

	return result;
}

QStringList QUStringSupport::extractTags(const QString &text) {
	QRegularExpression rx = QRegularExpression("\\[([^\\]]+)\\]");
	QRegularExpressionMatchIterator i = rx.globalMatch(text);

	QStringList tags;
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		tags << match.captured(1).trimmed();
	}

	if(text.contains("(kar)", Qt::CaseInsensitive))
		tags << "kar";

	return tags;
}

#ifndef QUSTRINGSUPPORT_H_
#define QUSTRINGSUPPORT_H_

#include <QObject>
#include <QString>
#include <QStringList>

class QUStringSupport: QObject {
    Q_OBJECT
    
public:
    QUStringSupport(QObject *parent = 0);

    static QString withoutUnsupportedCharacters (const QString &text);
	static QString withoutFolderTags(const QString &text);
	static QString withoutLeadingBlanks(const QString &text);
	static QString withoutAnyUmlaut(const QString &text);
	static QString withoutAnyUmlautEx(const QString &text);
	static QString withoutPathDelimiters(const QString &text);

	static QStringList extractTags(const QString &text); // extract all []-tags from a given string
};

#endif // QUSTRINGSUPPORT_H_

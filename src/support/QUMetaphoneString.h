#ifndef QUMETAPHONESTRING_H_
#define QUMETAPHONESTRING_H_

#include <QString>
#include <QStringList>

/*!
 * Double Metaphone (c) 1998, 1999 by Lawrence Philips
 *
 * Ported to Qt 4.3.5 by Marcel Taeumel
 *
 * Visit:
 *    http://en.wikipedia.org/wiki/Double_Metaphone
 *    http://www.ddj.com/cpp/184401251
 */
class QUMetaphoneString: public QString {
public:
	QUMetaphoneString(): QString() {}
	QUMetaphoneString(const char *str): QString(str) {}
	QUMetaphoneString(const QString &other): QString(other) {}

    bool isSlavoGermanic();
    bool hasVowel(int at);

    inline void add(const char *main);
    inline void add(const char *main, const char *alt);

    bool anyStringAt(int start, int length, ... );
    void doDoubleMetaphone(QString &metaph, QString &metaph2);
        
    static bool equal(QString token1, QString token2, bool ignoreEmpty = false);

private:
	int  _length;
	int  _last;
	bool _hasAlternate;
	QString _primary;
	QString _secondary;
};

#endif /* QUMETAPHONESTRING_H_ */

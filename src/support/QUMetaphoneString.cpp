#include "QUMetaphoneString.h"
#include "QUStringSupport.h"

#include <QList>
#include <QPair>
#include <QRegExp>

bool QUMetaphoneString::isSlavoGermanic() {
	return contains(QRegExp("W|K|CZ|WITZ"));
}

bool QUMetaphoneString::hasVowel(int at) {
	if( (at < 0) or (at >= _length) )
		return false;

	return QRegExp("[AEIOUY]").exactMatch(mid(at, 1));
}


inline void QUMetaphoneString::add(const char *main) {
	if(*main)
		_primary += main;
}

inline void QUMetaphoneString::add(const char *main, const char *alt) {
	if(*main)
		_primary += main;

	if(*alt) {
		_hasAlternate = true;
		if(alt[0] != ' ')
			_secondary += alt;
	} else
		if( *main and (main[0] != ' ') )
			_secondary += main;
}

/*!
 * Checks if any of the given test strings is at the given position.
 */
bool QUMetaphoneString::anyStringAt(int start, int length, ... ) {
    char    buffer[64];
    char*   test;
    QString target;

    test = buffer;
    target = mid(start, length);

    va_list sstrings;
    va_start(sstrings, length);

    do
    {
            test = va_arg(sstrings, char*);
            if(*test and (target == test))
                    return true;

    }while(strcmp(test, ""));

    va_end(sstrings);

    return false;
}

/*!
 * Creates to metaphone strings from this string object.
 */
void QUMetaphoneString::doDoubleMetaphone(QString &metaph, QString &metaph2) {
	int current = 0;

	_length = this->size();
	if(_length < 1)
		return;
	_last = _length - 1;//zero based index

	_hasAlternate = false;

	//pad the original string so that we can index beyond the edge of the world
	insert(this->size(), "     ");

	//skip these when at start of word
	if(anyStringAt(0, 2, "GN", "KN", "PN", "WR", "PS", ""))
		current += 1;

	//Initial 'X' is pronounced 'Z' e.g. 'Xavier'
	if(at(0) == 'X')
	{
		add("S"); //'Z' maps to 'S'
		current += 1;
	}

	///////////main loop//////////////////////////
	while((_primary.size() < 4) or (_secondary.size() < 4))
	{
		if(current >= _length)
			break;

		switch(at(current).toAscii())
		{
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		case 'Y':
			if(current == 0)
				//all init vowels now map to 'A'
				add("A");
			current +=1;
			break;

		case 'B':

			//"-mb", e.g", "dumb", already skipped over...
			add("P");

			if(at(current + 1) == 'B')
				current +=2;
			else
				current +=1;
			break;

		case 'Ç':
			add("S");
			current += 1;
			break;

		case 'C':
			//various germanic
			if((current > 1)
					and !hasVowel(current - 2)
					and anyStringAt((current - 1), 3, "ACH", "")
			and ((at(current + 2) != 'I') and ((at(current + 2) != 'E')
					or anyStringAt((current - 2), 6, "BACHER", "MACHER", "")) ))
			{
				add("K");
				current +=2;
				break;
			}

			//special case 'caesar'
			if((current == 0) and anyStringAt(current, 6, "CAESAR", ""))
			{
				add("S");
				current +=2;
				break;
			}

			//italian 'chianti'
			if(anyStringAt(current, 4, "CHIA", ""))
			{
				add("K");
				current +=2;
				break;
			}

			if(anyStringAt(current, 2, "CH", ""))
			{
				//find 'michael'
				if((current > 0) and anyStringAt(current, 4, "CHAE", ""))
				{
					add("K", "X");
					current +=2;
					break;
				}

				//greek roots e.g. 'chemistry', 'chorus'
				if((current == 0)
						and (anyStringAt((current + 1), 5, "HARAC", "HARIS", "")
								or anyStringAt((current + 1), 3, "HOR", "HYM", "HIA", "HEM", ""))
								and !anyStringAt(0, 5, "CHORE", ""))
				{
					add("K");
					current +=2;
					break;
				}

				//germanic, greek, or otherwise 'ch' for 'kh' sound
				if((anyStringAt(0, 4, "VAN ", "VON ", "") or anyStringAt(0, 3, "SCH", ""))
						// 'architect but not 'arch', 'orchestra', 'orchid'
						or anyStringAt((current - 2), 6, "ORCHES", "ARCHIT", "ORCHID")
				or anyStringAt((current + 2), 1, "T", "S", "")
				or ((anyStringAt((current - 1), 1, "A", "O", "U", "E", "") or (current == 0))
						//e.g., 'wachtler', 'wechsler', but not 'tichner'
						and anyStringAt((current + 2), 1, "L", "R", "N", "M", "B", "H", "F", "V", "W", " ", "")))
				{
					add("K");
				}else{
					if(current > 0)
					{
						if(anyStringAt(0, 2, "MC", ""))
							//e.g., "McHugh"
							add("K");
						else
							add("X", "K");
					}else
						add("X");
				}
				current +=2;
				break;
			}
			//e.g, 'czerny'
			if(anyStringAt(current, 2, "CZ", "") and !anyStringAt((current - 2), 4, "WICZ", ""))
			{
				add("S", "X");
				current += 2;
				break;
			}

			//e.g., 'focaccia'
			if(anyStringAt((current + 1), 3, "CIA", ""))
			{
				add("X");
				current += 3;
				break;
			}

			//double 'C', but not if e.g. 'McClellan'
			if(anyStringAt(current, 2, "CC", "") and !((current == 1) and (at(0) == 'M')))
				//'bellocchio' but not 'bacchus'
				if(anyStringAt((current + 2), 1, "I", "E", "H") and !anyStringAt((current + 2), 2, "HU", ""))
				{
					//'accident', 'accede' 'succeed'
					if(((current == 1) and (at(current - 1) == 'A'))
							or anyStringAt((current - 1), 5, "UCCEE", "UCCES", ""))
						add("KS");
					//'bacci', 'bertucci', other italian
					else
						add("X");
					current += 3;
					break;
				}else{//Pierce's rule
					add("K");
					current += 2;
					break;
				}

			if(anyStringAt(current, 2, "CK", "CG", "CQ", ""))
			{
				add("K");
				current += 2;
				break;
			}

			if(anyStringAt(current, 2, "CI", "CE", "CY", ""))
			{
				//italian vs. english
				if(anyStringAt(current, 3, "CIO", "CIE", "CIA", ""))
					add("S", "X");
				else
					add("S");
				current += 2;
				break;
			}

			//else
			add("K");

			//name sent in 'mac caffrey', 'mac gregor
			if(anyStringAt((current + 1), 2, " C", " Q", " G", ""))
				current += 3;
			else
				if(anyStringAt((current + 1), 1, "C", "K", "Q", "")
						and !anyStringAt((current + 1), 2, "CE", "CI", ""))
					current += 2;
				else
					current += 1;
			break;

		case 'D':
			if(anyStringAt(current, 2, "DG", ""))
				if(anyStringAt((current + 2), 1, "I", "E", "Y", ""))
				{
					//e.g. 'edge'
					add("J");
					current += 3;
					break;
				}else{
					//e.g. 'edgar'
					add("TK");
					current += 2;
					break;
				}

			if(anyStringAt(current, 2, "DT", "DD", ""))
			{
				add("T");
				current += 2;
				break;
			}

			//else
			add("T");
			current += 1;
			break;

		case 'F':
			if(at(current + 1) == 'F')
				current += 2;
			else
				current += 1;
			add("F");
			break;

		case 'G':
			if(at(current + 1) == 'H')
			{
				if((current > 0) and !hasVowel(current - 1))
				{
					add("K");
					current += 2;
					break;
				}

				if(current < 3)
				{
					//'ghislane', ghiradelli
					if(current == 0)
					{
						if(at(current + 2) == 'I')
							add("J");
						else
							add("K");
						current += 2;
						break;
					}
				}
				//Parker's rule (with some further refinements) - e.g., 'hugh'
				if(((current > 1) and anyStringAt((current - 2), 1, "B", "H", "D", "") )
						//e.g., 'bough'
						or ((current > 2) and anyStringAt((current - 3), 1, "B", "H", "D", "") )
						//e.g., 'broughton'
						or ((current > 3) and anyStringAt((current - 4), 1, "B", "H", "") ) )
				{
					current += 2;
					break;
				}else{
					//e.g., 'laugh', 'McLaughlin', 'cough', 'gough', 'rough', 'tough'
					if((current > 2)
							and (at(current - 1) == 'U')
							and anyStringAt((current - 3), 1, "C", "G", "L", "R", "T", "") )
					{
						add("F");
					}else
						if((current > 0) and at(current - 1) != 'I')
							add("K");

					current += 2;
					break;
				}
			}

			if(at(current + 1) == 'N')
			{
				if((current == 1) and hasVowel(0) and !isSlavoGermanic())
				{
					add("KN", "N");
				}else
					//not e.g. 'cagney'
					if(!anyStringAt((current + 2), 2, "EY", "")
							and (at(current + 1) != 'Y') and !isSlavoGermanic())
					{
						add("N", "KN");
					}else
						add("KN");
				current += 2;
				break;
			}

			//'tagliaro'
			if(anyStringAt((current + 1), 2, "LI", "") and !isSlavoGermanic())
			{
				add("KL", "L");
				current += 2;
				break;
			}

			//-ges-,-gep-,-gel-, -gie- at beginning
			if((current == 0)
					and ((at(current + 1) == 'Y')
							or anyStringAt((current + 1), 2, "ES", "EP", "EB", "EL", "EY", "IB", "IL", "IN", "IE", "EI", "ER", "")) )
			{
				add("K", "J");
				current += 2;
				break;
			}

			// -ger-,  -gy-
			if((anyStringAt((current + 1), 2, "ER", "") or (at(current + 1) == 'Y'))
					and !anyStringAt(0, 6, "DANGER", "RANGER", "MANGER", "")
					and !anyStringAt((current - 1), 1, "E", "I", "")
					and !anyStringAt((current - 1), 3, "RGY", "OGY", "") )
			{
				add("K", "J");
				current += 2;
				break;
			}

			// italian e.g, 'biaggi'
			if(anyStringAt((current + 1), 1, "E", "I", "Y", "") or anyStringAt((current - 1), 4, "AGGI", "OGGI", ""))
			{
				//obvious germanic
				if((anyStringAt(0, 4, "VAN ", "VON ", "") or anyStringAt(0, 3, "SCH", ""))
						or anyStringAt((current + 1), 2, "ET", ""))
					add("K");
				else
					//always soft if french ending
					if(anyStringAt((current + 1), 4, "IER ", ""))
						add("J");
					else
						add("J", "K");
				current += 2;
				break;
			}

			if(at(current + 1) == 'G')
				current += 2;
			else
				current += 1;
			add("K");
			break;

		case 'H':
			//only keep if first & before vowel or btw. 2 vowels
			if(((current == 0) or hasVowel(current - 1))
					and hasVowel(current + 1))
			{
				add("H");
				current += 2;
			}else//also takes care of 'HH'
				current += 1;
			break;

		case 'J':
			//obvious spanish, 'jose', 'san jacinto'
			if(anyStringAt(current, 4, "JOSE") or anyStringAt(0, 4, "SAN ", "") )
			{
				if(((current == 0) and (at(current + 4) == ' ')) or anyStringAt(0, 4, "SAN ", "") )
					add("H");
				else
				{
					add("J", "H");
				}
				current +=1;
				break;
			}

			if((current == 0) and !anyStringAt(current, 4, "JOSE"))
				add("J", "A");//Yankelovich/Jankelowicz
			else
				//spanish pron. of e.g. 'bajador'
				if(hasVowel(current - 1)
						and !isSlavoGermanic()
						and ((at(current + 1) == 'A') or (at(current + 1) == 'O')))
					add("J", "H");
				else
					if(current == _last)
						add("J", " ");
					else
						if(!anyStringAt((current + 1), 1, "L", "T", "K", "S", "N", "M", "B", "Z", "")
								and !anyStringAt((current - 1), 1, "S", "K", "L", ""))
							add("J");

			if(at(current + 1) == 'J')//it could happen!
				current += 2;
			else
				current += 1;
			break;

		case 'K':
			if(at(current + 1) == 'K')
				current += 2;
			else
				current += 1;
			add("K");
			break;

		case 'L':
			if(at(current + 1) == 'L')
			{
				//spanish e.g. 'cabrillo', 'gallegos'
				if(((current == (_length - 3))
						and anyStringAt((current - 1), 4, "ILLO", "ILLA", "ALLE", ""))
						or ((anyStringAt((_last - 1), 2, "AS", "OS", "") or anyStringAt(_last, 1, "A", "O", ""))
								and anyStringAt((current - 1), 4, "ALLE", "")) )
				{
					add("L", " ");
					current += 2;
					break;
				}
				current += 2;
			}else
				current += 1;
			add("L");
			break;

		case 'M':
			if((anyStringAt((current - 1), 3, "UMB", "")
					and (((current + 1) == _last) or anyStringAt((current + 2), 2, "ER", "")))
					//'dumb','thumb'
					or  (at(current + 1) == 'M') )
				current += 2;
			else
				current += 1;
			add("M");
			break;

		case 'N':
			if(at(current + 1) == 'N')
				current += 2;
			else
				current += 1;
			add("N");
			break;

		case 'Ñ':
			current += 1;
			add("N");
			break;

		case 'P':
			if(at(current + 1) == 'H')
			{
				add("F");
				current += 2;
				break;
			}

			//also account for "campbell", "raspberry"
			if(anyStringAt((current + 1), 1, "P", "B", ""))
				current += 2;
			else
				current += 1;
			add("P");
			break;

		case 'Q':
			if(at(current + 1) == 'Q')
				current += 2;
			else
				current += 1;
			add("K");
			break;

		case 'R':
			//french e.g. 'rogier', but exclude 'hochmeier'
			if((current == _last)
					and !isSlavoGermanic()
					and anyStringAt((current - 2), 2, "IE", "")
			and !anyStringAt((current - 4), 2, "ME", "MA", ""))
				add("", "R");
			else
				add("R");

			if(at(current + 1) == 'R')
				current += 2;
			else
				current += 1;
			break;

		case 'S':
			//special cases 'island', 'isle', 'carlisle', 'carlysle'
			if(anyStringAt((current - 1), 3, "ISL", "YSL", ""))
			{
				current += 1;
				break;
			}

			//special case 'sugar-'
			if((current == 0) and anyStringAt(current, 5, "SUGAR", ""))
			{
				add("X", "S");
				current += 1;
				break;
			}

			if(anyStringAt(current, 2, "SH", ""))
			{
				//germanic
				if(anyStringAt((current + 1), 4, "HEIM", "HOEK", "HOLM", "HOLZ", ""))
					add("S");
				else
					add("X");
				current += 2;
				break;
			}

			//italian & armenian
			if(anyStringAt(current, 3, "SIO", "SIA", "") or anyStringAt(current, 4, "SIAN", ""))
			{
				if(!isSlavoGermanic())
					add("S", "X");
				else
					add("S");
				current += 3;
				break;
			}

			//german & anglicisations, e.g. 'smith' match 'schmidt', 'snider' match 'schneider'
			//also, -sz- in slavic language altho in hungarian it is pronounced 's'
			if(((current == 0)
					and anyStringAt((current + 1), 1, "M", "N", "L", "W", ""))
					or anyStringAt((current + 1), 1, "Z", ""))
			{
				add("S", "X");
				if(anyStringAt((current + 1), 1, "Z", ""))
					current += 2;
				else
					current += 1;
				break;
			}

			if(anyStringAt(current, 2, "SC", ""))
			{
				//Schlesinger's rule
				if(at(current + 2) == 'H')
					//dutch origin, e.g. 'school', 'schooner'
					if(anyStringAt((current + 3), 2, "OO", "ER", "EN", "UY", "ED", "EM", ""))
					{
						//'schermerhorn', 'schenker'
						if(anyStringAt((current + 3), 2, "ER", "EN", ""))
						{
							add("X", "SK");
						}else
							add("SK");
						current += 3;
						break;
					}else{
						if((current == 0) and !hasVowel(3) and (at(3) != 'W'))
							add("X", "S");
						else
							add("X");
						current += 3;
						break;
					}

				if(anyStringAt((current + 2), 1, "I", "E", "Y", ""))
				{
					add("S");
					current += 3;
					break;
				}
				//else
				add("SK");
				current += 3;
				break;
			}

			//french e.g. 'resnais', 'artois'
			if((current == _last) and anyStringAt((current - 2), 2, "AI", "OI", ""))
				add("", "S");
			else
				add("S");

			if(anyStringAt((current + 1), 1, "S", "Z", ""))
				current += 2;
			else
				current += 1;
			break;

		case 'T':
			if(anyStringAt(current, 4, "TION", ""))
			{
				add("X");
				current += 3;
				break;
			}

			if(anyStringAt(current, 3, "TIA", "TCH", ""))
			{
				add("X");
				current += 3;
				break;
			}

			if(anyStringAt(current, 2, "TH", "")
					or anyStringAt(current, 3, "TTH", ""))
			{
				//special case 'thomas', 'thames' or germanic
				if(anyStringAt((current + 2), 2, "OM", "AM", "")
						or anyStringAt(0, 4, "VAN ", "VON ", "")
				or anyStringAt(0, 3, "SCH", ""))
				{
					add("T");
				}else{
					add("0", "T");
				}
				current += 2;
				break;
			}

			if(anyStringAt((current + 1), 1, "T", "D", ""))
				current += 2;
			else
				current += 1;
			add("T");
			break;

		case 'V':
			if(at(current + 1) == 'V')
				current += 2;
			else
				current += 1;
			add("F");
			break;

		case 'W':
			//can also be in middle of word
			if(anyStringAt(current, 2, "WR", ""))
			{
				add("R");
				current += 2;
				break;
			}

			if((current == 0)
					and (hasVowel(current + 1) or anyStringAt(current, 2, "WH", "")))
			{
				//Wasserman should match Vasserman
				if(hasVowel(current + 1))
					add("A", "F");
				else
					//need Uomo to match Womo
					add("A");
			}

			//Arnow should match Arnoff
			if(((current == _last) and hasVowel(current - 1))
					or anyStringAt((current - 1), 5, "EWSKI", "EWSKY", "OWSKI", "OWSKY", "")
			or anyStringAt(0, 3, "SCH", ""))
			{
				add("", "F");
				current +=1;
				break;
			}

			//polish e.g. 'filipowicz'
			if(anyStringAt(current, 4, "WICZ", "WITZ"))
			{
				add("TS", "FX");
				current +=4;
				break;
			}

			//else skip it
			current +=1;
			break;

		case 'X':
			//french e.g. breaux
			if(!((current == _last)
					and (anyStringAt((current - 3), 3, "IAU", "EAU", "")
							or anyStringAt((current - 2), 2, "AU", "OU", ""))) )
				add("KS");

			if(anyStringAt((current + 1), 1, "C", "X", ""))
				current += 2;
			else
				current += 1;
			break;

		case 'Z':
			//chinese pinyin e.g. 'zhao'
			if(at(current + 1) == 'H')
			{
				add("J");
				current += 2;
				break;
			}else
				if(anyStringAt((current + 1), 2, "ZO", "ZI", "ZA", "")
						or (isSlavoGermanic() and ((current > 0) and at(current - 1) != 'T')))
				{
					add("S", "TS");
				}
				else
					add("S");

			if(at(current + 1) == 'Z')
				current += 2;
			else
				current += 1;
			break;

		default:
			current += 1;
		}
	}

	metaph = _primary;
	//only give back 4 char metaph
	if(metaph.size() > 4)
		metaph = metaph.left(4);
	if(_hasAlternate)
	{
		metaph2 = _secondary;
		if(metaph2.size() > 4)
			metaph2 = metaph2.left(4);
	}
}

/*!
 * Checks whether the given tokens could be meant as equal.
 */
bool QUMetaphoneString::equal(QString token1, QString token2, bool ignoreEmpty) {
	token1 = QUStringSupport::withoutAnyUmlautEx(token1).toUpper();
	token2 = QUStringSupport::withoutAnyUmlautEx(token2).toUpper();

	// remove special characters
	for(int i = 0; i < token1.length(); i++) if(token1[i] != QChar(' ') and token1[i].category() != QChar::Letter_Uppercase) token1.remove(i, 1);
	for(int i = 0; i < token2.length(); i++) if(token2[i] != QChar(' ') and token2[i].category() != QChar::Letter_Uppercase) token2.remove(i, 1);

	// replace "lonely, unimportant" words
	token1.append(' '); token1.prepend(' ');
	token2.append(' '); token2.prepend(' ');
	QRegExp rx(" THE | DER | DIE | DAS | A | AN | EIN ");
	token1.replace(rx, " ");
	token2.replace(rx, " ");

	token1 = token1.simplified().trimmed();
	token2 = token2.simplified().trimmed();

	if(token1.isEmpty() or token2.isEmpty())
		return !ignoreEmpty;

	/* Use MetaPhone */

	QList<QPair<QString, QString> > mp1;
	QList<QPair<QString, QString> > mp2;

	foreach(QString text, token1.split(" ")) {
		QUMetaphoneString mps(text);

		QString m1, m2;
		mps.doDoubleMetaphone(m1, m2);

		mp1.append(QPair<QString, QString>(m1, m2));
	}

	foreach(QString text, token2.split(" ")) {
		QUMetaphoneString mps(text);

		QString m1, m2;
		mps.doDoubleMetaphone(m1, m2);

		mp2.append(QPair<QString, QString>(m1, m2));
	}

	int neededHits = qMin(mp1.size(), mp2.size());
	int hits = 0;

	for(int i = 0; i < mp1.size(); i++) {
		if(mp1.at(i).first.isEmpty())
			continue; // should not happen...

		for(int j = 0; j < mp2.size(); j++) {
//			if(mp2.at(j).first.isEmpty())
//				continue; // should not happen...

			if(QString::compare(mp1.at(i).first, mp2.at(j).first) == 0)
				hits++;
			else if(mp2.at(j).second.length() > 1 and mp1.at(i).first == mp2.at(j).second)
				hits++;
			else if(mp1.at(i).second.length() > 1 and mp1.at(i).second == mp2.at(j).first)
				hits++;
			else if(mp1.at(i).second.length() > 1 and mp2.at(j).second.length() > 1 and mp1.at(i).second == mp2.at(j).second)
				hits++;

			if(hits >= neededHits)
				return true;
		}
	}

	return false;
}

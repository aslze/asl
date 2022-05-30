#ifndef ASL_QT_INTEROP_H
#define ASL_QT_INTEROP_H

#include <QString>
#include <asl/String.h>

namespace asl {

/**
Converts a String to a Qt QString
*/
inline QString qt(const String& s)
{
	return QString::ASL_QTFROM(*s);
}

/**
Converts a Qt QString to a String
*/
inline String qt(const QString& s)
{
	QByteArray a = s.ASL_QTTO();
	return String(a.data(), a.size());
}

inline ASL_DEPRECATED(QString _qt_(const String& s), "Use qt(s)") { return qt(s); }

inline ASL_DEPRECATED(String _qt_(const QString& s), "Use qt(s)") { return qt(s); }

}

#endif

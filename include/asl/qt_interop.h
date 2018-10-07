#ifndef ASL_QT_INTEROP_H
#define ASL_QT_INTEROP_H

namespace asl {

/**
Converts a String to a Qt QString
*/
inline QString qt(const String& s)
{
	return QString::QTFROM(s);
}

/**
Converts a Qt QString to a String
*/
inline String qt(const QString& s)
{
	QByteArray a = s.QTTO();
	return String(a.data(), a.size());
}

inline QString _qt_(const String& s) { return qt(s); }

inline String _qt_(const QString& s) { return qt(s); }

}

#endif

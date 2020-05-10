#ifndef ASL_QT_INTEROP_H
#define ASL_QT_INTEROP_H

#if defined QSTRING_H && !defined QTFROM
#if defined(ASL_ANSI)
#define QTFROM fromLocal8Bit
#define QTTO toLocal8Bit
#else
#define QTFROM fromUtf8
#define QTTO toUtf8
#endif
#endif

namespace asl {

template<>
struct Castable<QString, String> { typedef int is; };

template<>
struct Castable<String, QString> { typedef int is; };

template<>
inline QString to(const asl::String& x, DummyType<QString>)
{
	return QString::QTFROM(*x);
}

template<>
inline asl::String to(const QString& x, DummyType<asl::String>)
{
	QByteArray a = x.QTTO();
	return String(a.data(), a.size());
}

/**
Converts a String to a Qt QString
*/
inline QString qt(const String& s)
{
	return QString::QTFROM(*s);
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

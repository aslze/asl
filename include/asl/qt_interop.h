#ifndef ASL_QT_INTEROP_H
#define ASL_QT_INTEROP_H

namespace asl {

inline QString _qt_(const asl::String& s)
{
	return QString::fromUtf8(s);
}

inline asl::String _qt_(const QString& s)
{
	return s.toUtf8().data();
}

}

#endif

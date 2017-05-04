#ifndef VERSIONHELPER_H
#define VERSIONHELPER_H

#include <QString>
#include <QStringList>

namespace Proof {
constexpr quint64 packVersion(quint16 major, quint16 year, quint16 month, quint16 day)
{
    return ((quint64)major << 48) | ((quint64)year << 32) | ((quint64)month << 16) | ((quint64)day);
}

constexpr quint16 majorVersionPart(quint64 version)
{
    return version >> 48;
}

constexpr quint64 dateVersionPart(quint64 version)
{
    return version & ~(0xFFFFull << 48);
}

inline quint64 packVersion(const QStringList &version)
{
    if (version.count() < 4)
        return 0x0;
    return ((quint64)version[0].toShort() << 48)
            | ((quint64)version[1].toShort() << 32)
            | ((quint64)version[2].toShort() << 16)
            | ((quint64)version[3].toShort());
}

inline quint64 packVersion(const QString &version)
{
    return packVersion(version.split(QStringLiteral(".")));
}

inline QString unpackVersionToString(quint64 version)
{
    return QStringLiteral("%1.%2.%3.%4")
            .arg(version >> 48)
            .arg((version >> 32) & 0xFFFF)
            .arg((version >> 16) & 0xFFFF)
            .arg(version & 0xFFFF);
}
}

#endif // VERSIONHELPER_H

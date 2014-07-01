#include "humanizer.h"

#include <QStringList>

using namespace Proof;

Humanizer::Humanizer(QObject *parent)
    : QObject(parent)
{
}

Humanizer::~Humanizer()
{
}

QString Proof::Humanizer::humanizeTime(qlonglong seconds, Proof::Humanizer::TimeCategory stopAt)
{
    return Proof::humanizeTime(seconds, stopAt);
}

QString Humanizer::humanizeBytesSize(qlonglong bytes)
{
    return Proof::humanizeBytesSize(bytes);
}

QString Proof::humanizeTime(qlonglong seconds, Humanizer::TimeCategory stopAt)
{
    static const qlonglong secondsInMinute = 60;
    static const qlonglong secondsInHour = secondsInMinute * 60;
    static const qlonglong secondsInDay = secondsInHour * 24;
    static const qlonglong secondsInWeek = secondsInDay * 7;

    qlonglong weeks = seconds / secondsInWeek;
    seconds = seconds % secondsInWeek;
    qlonglong days = seconds / secondsInDay;
    seconds = seconds % secondsInDay;
    qlonglong hours = seconds / secondsInHour;
    seconds = seconds % secondsInHour;
    qlonglong minutes = seconds / secondsInMinute;
    seconds = seconds % secondsInMinute;

    //Let's prettify it a bit.
    //We don't need 1w 1d here, 8d will work better.
    //But for days, hours and minutes it should work as usual.
    if (weeks < 2) {
        days += weeks * 7;
        weeks = 0;
    }

    //We don't need 1w 2h too, just 1w will be fine.
    //So we will truncate everything more than two upmost nonzero values.
    if (weeks > 0) {
        seconds = 0;
        minutes = 0;
        hours = 0;
    } else if (days > 0) {
        seconds = 0;
        minutes = 0;
    } else if (hours > 0) {
        seconds = 0;
    }

    QStringList result;
    if (weeks > 0 && stopAt <= Humanizer::TimeCategory::StopAtWeeks)
        result << QString("%1w").arg(weeks);
    if (days > 0 && stopAt <= Humanizer::TimeCategory::StopAtDays)
        result << QString("%1d").arg(days);
    if (hours > 0 && stopAt <= Humanizer::TimeCategory::StopAtHours)
        result << QString("%1h").arg(hours);
    if (minutes > 0 && stopAt <= Humanizer::TimeCategory::StopAtMinutes)
        result << QString("%1m").arg(minutes);
    if (seconds > 0 && stopAt == Humanizer::TimeCategory::StopAtSeconds)
        result << QString("%1s").arg(seconds);

    if (result.isEmpty()) {
        switch (stopAt) {
        case Humanizer::TimeCategory::StopAtWeeks:
            return "<1w";
        case Humanizer::TimeCategory::StopAtDays:
            return "<1d";
        case Humanizer::TimeCategory::StopAtHours:
            return "<1h";
        case Humanizer::TimeCategory::StopAtMinutes:
            return "<1m";
        case Humanizer::TimeCategory::StopAtSeconds:
            return "0s";
        }
    }

    return result.join(" ");
}

QString Proof::humanizeBytesSize(qlonglong bytes)
{
    static const qlonglong bytesInKilobyte = 1ll << 10;
    static const qlonglong bytesInMegabyte = bytesInKilobyte << 10;
    static const qlonglong bytesInGigabyte = bytesInMegabyte << 10;

    if (bytes >= bytesInGigabyte)
        return QString("%1G").arg((double)bytes / (double)bytesInGigabyte, 0, 'f', 2);
    if (bytes >= bytesInMegabyte)
        return QString("%1M").arg((double)bytes / (double)bytesInMegabyte, 0, 'f', 2);
    if (bytes >= bytesInKilobyte)
        return QString("%1K").arg((double)bytes / (double)bytesInKilobyte, 0, 'f', 2);
    return QString("%1 bytes").arg(bytes);
}

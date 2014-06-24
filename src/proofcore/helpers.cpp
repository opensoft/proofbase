#include "helpers.h"

#include <QStringList>

using namespace Proof;

Humanizer::Humanizer(QObject *parent) :
    QObject(parent)
{
}

Humanizer::~Humanizer()
{
}

QString Humanizer::humanizeTime(qlonglong seconds, Humanizer::TimeCategory stopAt)
{
    //hour = 3600
    //day = 86400
    //week = 604800

    qlonglong weeks = seconds / 604800;
    seconds = seconds % 604800;
    qlonglong days = seconds / 86400;
    seconds = seconds % 86400;
    qlonglong hours = seconds / 3600;
    seconds = seconds % 3600;
    qlonglong minutes = seconds / 60;
    seconds = seconds % 60;

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
    if (weeks > 0 && stopAt <= TimeCategory::StopAtWeeks)
        result << QString("%1w").arg(weeks);
    if (days > 0 && stopAt <= TimeCategory::StopAtDays)
        result << QString("%1d").arg(days);
    if (hours > 0 && stopAt <= TimeCategory::StopAtHours)
        result << QString("%1h").arg(hours);
    if (minutes > 0 && stopAt <= TimeCategory::StopAtMinutes)
        result << QString("%1m").arg(minutes);
    if (seconds > 0 && stopAt == TimeCategory::StopAtSeconds)
        result << QString("%1s").arg(seconds);

    if (result.isEmpty()) {
        switch (stopAt) {
        case TimeCategory::StopAtWeeks:
            return "<1w";
        case TimeCategory::StopAtDays:
            return "<1d";
        case TimeCategory::StopAtHours:
            return "<1h";
        case TimeCategory::StopAtMinutes:
            return "<1m";
        case TimeCategory::StopAtSeconds:
            return "0s";
        }
    }

    return result.join(" ");
}

QString Humanizer::humanizeBytesSize(qlonglong bytes)
{
    //1kb = 1024
    //1mb = 1048576
    //1gb = 1073741824

    if (bytes >= 1073741824)
        return QString("%1 GB").arg((double)bytes/1073741824.0, 0, 'f', 2);
    if (bytes >= 1048576)
        return QString("%1 MB").arg((double)bytes/1048576.0, 0, 'f', 2);
    if (bytes >= 1024)
        return QString("%1 KB").arg((double)bytes/1024.0, 0, 'f', 2);
    return QString("%1 bytes").arg(bytes);
}

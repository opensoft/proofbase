#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include "coreapplication.h"
#include "proofcore_global.h"

#include <QtGlobal>
#include <QCoreApplication>
#include <QTranslator>
#include <QSet>

namespace Proof {
class Settings;
class PROOF_CORE_EXPORT CoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    void initApp(const QStringList &defaultLoggingRules);
    void initTranslator();

    void setLanguage(const QString &currentLanguage);
    QString language() const;

    Settings *settings = nullptr;
    QSet<QString> translationPrefixes;
    QStringList availableLanguages;
    QString currentLanguage = "en";
    QList<QTranslator *> installedTranslators;
    QCoreApplication *q_ptr = nullptr;
};
}

#endif // COREAPPLICATION_P_H

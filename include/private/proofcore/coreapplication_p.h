#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include "coreapplication.h"
#include "proofcore_global.h"

#include <QtGlobal>
#include <QCoreApplication>
#include <QTranslator>
#include <QSet>

#ifndef Q_OS_ANDROID
# include <QtCrypto>
#endif

namespace Proof {
class Settings;
class UpdateManager;
class PROOF_CORE_EXPORT CoreApplicationPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    void initApp(const QStringList &defaultLoggingRules);
    void initTranslator();

    void setLanguage(const QString &currentLanguage);
    QString language() const;

    QString prettifiedApplicationName;
    Settings *settings = nullptr;
    UpdateManager *updateManager = nullptr;
    QSet<QString> translationPrefixes;
    QStringList availableLanguages;
    QString currentLanguage = "en";
    QList<QTranslator *> installedTranslators;
#ifndef Q_OS_ANDROID
    QScopedPointer<QCA::Initializer> qcaInit;
#endif
    QCoreApplication *q_ptr = nullptr;
};
}

#endif // COREAPPLICATION_P_H

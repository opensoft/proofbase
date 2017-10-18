#ifndef COREAPPLICATION_P_H
#define COREAPPLICATION_P_H

#include "proofcore/proofobject_p.h"
#include "proofcore/proofcore_global.h"

#include <QtGlobal>
#include <QTranslator>
#include <QSet>
#include <QDateTime>

#ifndef QCA_DISABLED
# include <QtCrypto>
#endif

namespace Proof {
class Settings;
class UpdateManager;
class CoreApplication;
class PROOF_CORE_EXPORT CoreApplicationPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(CoreApplication)
protected:
    virtual void postInit();

    void initCrashHandler();
    void updatePrettifiedName();
    bool daemonizeIfNeeded();
    void initLogs(bool daemonized);
    void execMigrations();
    void initQca();
    void initTranslator();
    void initUpdateManager();

    void setLanguage(const QString &currentLanguage);

    QString prettifiedApplicationName;
    QDateTime startedAt = QDateTime::currentDateTimeUtc();
    Settings *settings = nullptr;
    UpdateManager *updateManager = nullptr;
    QSet<QString> translationPrefixes;
    QStringList availableLanguages;
    QVariantMap fullLanguageNames;
    QString currentLanguage = QStringLiteral("en");
    QList<QTranslator *> installedTranslators;
    bool initialized = false;
#ifndef QCA_DISABLED
    QScopedPointer<QCA::Initializer> qcaInit;
#endif
};
}

#endif // COREAPPLICATION_P_H

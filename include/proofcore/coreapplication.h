#ifndef PROOF_COREAPPLICATION_H
#define PROOF_COREAPPLICATION_H

#include "proofcore/proofcore_global.h"

#include <QCoreApplication>

#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<Proof::CoreApplication *>(QCoreApplication::instance()))

namespace Proof {
class Settings;
class CoreApplicationPrivate;
class PROOF_CORE_EXPORT CoreApplication : public QCoreApplication
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CoreApplication)
public:
    CoreApplication(int &argc, char **argv, const QString &orgName = QString(), const QString &appName = QString(),
                    const QStringList &defaultLoggingRules = QStringList());
    ~CoreApplication();

    Settings *settings() const;
    void setLanguage(const QString &language);
    QStringList availableLanguages();
signals:
    void languageChanged(const QString &language);
private:
    QScopedPointer<CoreApplicationPrivate> d_ptr;
};

}

#endif // PROOF_COREAPPLICATION_H

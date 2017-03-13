#ifndef PROOF_COREAPPLICATION_H
#define PROOF_COREAPPLICATION_H

#include "proofcore/proofcore_global.h"
#include "proofcore/proofobject.h"

#include <QDateTime>

#define proofApp Proof::CoreApplication::instance()

class QCoreApplication;

namespace Proof {
class UpdateManager;
class Settings;
class CoreApplicationPrivate;

class PROOF_CORE_EXPORT CoreApplication : public ProofObject
{
    Q_OBJECT
    Q_PROPERTY(QString prettifiedApplicationName READ prettifiedApplicationName CONSTANT)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)
    Q_PROPERTY(QVariantMap fullLanguageNames READ fullLanguageNames CONSTANT)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(int languageIndex READ languageIndex NOTIFY languageChanged)
    Q_PROPERTY(QString emptyString READ emptyString NOTIFY languageChanged)
    Q_PROPERTY(Proof::UpdateManager *updateManager READ updateManager CONSTANT)
    Q_PROPERTY(QDateTime startedAt READ startedAt CONSTANT)
    Q_DECLARE_PRIVATE(CoreApplication)
public:
    CoreApplication(int &argc, char **argv,
                    const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QString("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());

    CoreApplication(QCoreApplication *app,
                    const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QString("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());

    ~CoreApplication();

    QString prettifiedApplicationName() const;
    QStringList availableLanguages();
    QVariantMap fullLanguageNames();
    QString language() const;
    void setLanguage(const QString &language);
    int languageIndex() const;
    QString emptyString() const;
    UpdateManager *updateManager() const;
    QDateTime startedAt() const;

    Settings *settings() const;

    Q_INVOKABLE void postInit();
    Q_INVOKABLE int exec();

    static CoreApplication *instance();

signals:
    void languageChanged(const QString &language);

protected:
    CoreApplication(CoreApplicationPrivate &dd, QCoreApplication *app,
                    const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QString("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());
};

}

#endif // PROOF_COREAPPLICATION_H

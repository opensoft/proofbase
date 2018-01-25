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

class PROOF_CORE_EXPORT CoreApplication : public ProofObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
    Q_PROPERTY(QString prettifiedApplicationName READ prettifiedApplicationName CONSTANT)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)
    Q_PROPERTY(QVariantMap fullLanguageNames READ fullLanguageNames CONSTANT)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(int languageIndex READ languageIndex NOTIFY languageChanged)
    Q_PROPERTY(Proof::UpdateManager *updateManager READ updateManager CONSTANT)
    Q_PROPERTY(QDateTime startedAt READ startedAt CONSTANT)
    Q_DECLARE_PRIVATE(CoreApplication)
public:
    CoreApplication(int &argc, char **argv,
                    const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QStringLiteral("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());

    CoreApplication(QCoreApplication *app,
                    const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QStringLiteral("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());

    ~CoreApplication();

    QString prettifiedApplicationName() const;
    QStringList availableLanguages();
    QVariantMap fullLanguageNames();
    QString language() const;
    void setLanguage(const QString &language);
    int languageIndex() const;
    UpdateManager *updateManager() const;
    QDateTime startedAt() const;

    Settings *settings() const;

    Q_INVOKABLE void postInit();
    Q_INVOKABLE int exec();

    static CoreApplication *instance();

    static void addInitializer(const std::function<void()> &initializer);
    //Migration can't use qApp/proofApp since it is not guaranteed that they will exist or be properly configured during migration exec
    //All migrations should be added before app ctor is called
    //Max related version here means version when something is changed. I.e. it is not last non-changed version, it is first changed one
    //Migration may not be called at all if config version is greater or equal than both related app and related proof versions
    //Migration should minimize exec time in case when it was called but version is greater or equal than related
    //arguments: oldAppVersion, oldProofVersion, settings pointer
    //Versions are packed into quint64 using proofcore/helpers/versionhelper.h
    using Migration = std::function<void(quint64, quint64, Settings *)>;
    static void addMigration(quint64 maxRelatedVersion, Migration &&migration);
    static void addMigration(const QString &maxRelatedVersion, Migration &&migration);
    static void addMigrations(const QMap<quint64, QList<Migration>> &migrations);
    static void addMigrations(const QMap<QString, QList<Migration>> &migrations);

signals:
    void languageChanged(const QString &language);

protected:
    CoreApplication(CoreApplicationPrivate &dd, QCoreApplication *app,
                    const QString &orgName = QString(), const QString &appName = QString(),
                    const QString &version = QStringLiteral("0.0.0.0"),
                    const QStringList &defaultLoggingRules = QStringList());
};

}

#endif // PROOF_COREAPPLICATION_H

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QScopedPointer>

#include "proofcore_global.h"

namespace Proof {

class SettingsGroup;
class SettingsPrivate;

class PROOF_CORE_EXPORT Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);
    ~Settings();

    bool isNativeFormatEnabled() const;
    void setNativeFormatEnabled(bool arg);

    void sync();

    SettingsGroup *mainGroup();

signals:
    void nativeFormatEnabledChanged(bool arg);

private:
    Q_DECLARE_PRIVATE(Settings)
    QScopedPointer<SettingsPrivate> d_ptr;
};

}

#endif // SETTINGS_H

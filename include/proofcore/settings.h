#ifndef SETTINGS_H
#define SETTINGS_H

#include <QScopedPointer>

#include "proofobject.h"
#include "proofcore_global.h"

namespace Proof {

class SettingsGroup;
class SettingsPrivate;

class PROOF_CORE_EXPORT Settings : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Settings)
public:
    explicit Settings(QObject *parent = 0);

    bool isNativeFormatEnabled() const;
    void setNativeFormatEnabled(bool arg);
    void sync();
    SettingsGroup *mainGroup();

signals:
    void nativeFormatEnabledChanged(bool arg);

};

}

#endif // SETTINGS_H

proof_add_target_sources(Core
    src/proofcore/settings.cpp
    src/proofcore/settingsgroup.cpp
    src/proofcore/helpers/humanizer.cpp
    src/proofcore/proofobject.cpp
    src/proofcore/proofcore_init.cpp
    src/proofcore/proofglobal.cpp
    src/proofcore/expirator.cpp
    src/proofcore/logs.cpp
    src/proofcore/coreapplication.cpp
    src/proofcore/updatemanager.cpp
    src/proofcore/proofobjectprivatepointer.cpp
    src/proofcore/abstractnotificationhandler.cpp
    src/proofcore/memorystoragenotificationhandler.cpp
    src/proofcore/errornotifier.cpp
)

proof_add_target_headers(Core
    include/proofcore/proofcore_global.h
    include/proofcore/settings.h
    include/proofcore/settingsgroup.h
    include/proofcore/helpers/humanizer.h
    include/proofcore/proofobject.h
    include/proofcore/proofglobal.h
    include/proofcore/objectscache.h
    include/proofcore/expirator.h
    include/proofcore/logs.h
    include/proofcore/coreapplication.h
    include/proofcore/updatemanager.h
    include/proofcore/proofobjectprivatepointer.h
    include/proofcore/abstractnotificationhandler.h
    include/proofcore/memorystoragenotificationhandler.h
    include/proofcore/errornotifier.h
    include/proofcore/helpers/versionhelper.h
    include/proofcore/basic_package.h
)

proof_add_target_private_headers(Core
    include/private/proofcore/proofobject_p.h
    include/private/proofcore/coreapplication_p.h
    include/private/proofcore/abstractnotificationhandler_p.h
    include/private/proofcore/abstractbarcodeconfigurator_p.h
)

find_package(ZLIB REQUIRED)

proof_add_module(Core
    QT_LIBS Core
    PROOF_LIBS Seed
    OTHER_LIBS qca-qt5 ZLIB::ZLIB
)

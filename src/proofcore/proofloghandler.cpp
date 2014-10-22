#include "proofloghandler.h"

#include <QMap>
#include <QFile>
#include <QDateTime>
#include <QtMessageHandler>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>

using namespace Proof;

QString ProofLogHandler::m_logFileBaseName;
QtMessageHandler ProofLogHandler::m_coreHandler = nullptr;

static QMap<QtMsgType, QString> typeToString = {
    { QtDebugMsg, "DEBUG"},
    { QtWarningMsg, "WARNING"},
    { QtCriticalMsg, "CRITICAL"},
    { QtFatalMsg, "FATAL"},
    { QtSystemMsg, "SYSTEM"}
};

ProofLogHandler::ProofLogHandler()
{
    m_coreHandler = qInstallMessageHandler(0);

    QString configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    if (configDir.isEmpty() && QDir::root().mkpath(configDir)) {
        QFile loggingRulesFile(QDir(configDir).absoluteFilePath(qAppName() + ".qtlogging.rules"));
        if (loggingRulesFile.open(QFile::ReadOnly)) {
            QString rules = QString(loggingRulesFile.readAll());
            QLoggingCategory::setFilterRules(rules);
        }
    }
}

ProofLogHandler::~ProofLogHandler()
{
    uninstall();
}


ProofLogHandler *ProofLogHandler::instance()
{
    static ProofLogHandler *_instance = nullptr;
    if (!_instance)
        _instance = new ProofLogHandler();
    return _instance;
}

void ProofLogHandler::install(const QString &logFileBaseName)
{
    m_logFileBaseName = logFileBaseName;
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
        if (!m_logFileBaseName.isEmpty()) {
            QFile logFile(m_logFileBaseName + "." + QDate::currentDate().toString(Qt::ISODate));

            if (logFile.open(QFile::Append | QFile::Text)) {
                QString logLine = QString("[%1][%2] (%3:%4 %5) %6 - %7\n")
                        .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(typeToString[type])
                        .arg(context.file)
                        .arg(context.line)
                        .arg(context.function)
                        .arg(context.category)
                        .arg(message);

                logFile.write(logLine.toLocal8Bit());
            }
            logFile.close();
        }
        if (m_coreHandler)
            m_coreHandler(type, context, message);

    });
}

void ProofLogHandler::uninstall()
{
    qInstallMessageHandler(0);
}

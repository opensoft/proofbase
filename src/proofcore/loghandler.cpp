#include "loghandler.h"

#include <QMap>
#include <QFile>
#include <QDateTime>
#include <QtMessageHandler>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>

using namespace Proof;

QString LogHandler::m_logFileBaseName;
QtMessageHandler LogHandler::m_coreHandler = nullptr;
QMutex LogHandler::m_writeLogMutex;

static QMap<QtMsgType, QString> stringifiedTypes = {
    { QtDebugMsg, "D"},
    { QtWarningMsg, "W"},
    { QtCriticalMsg, "C"},
    { QtFatalMsg, "F"},
    { QtSystemMsg, "S"}
};

LogHandler::LogHandler()
{
    m_coreHandler = qInstallMessageHandler(0);
}

LogHandler::~LogHandler()
{
    uninstall();
}


LogHandler *LogHandler::instance()
{
    static LogHandler _instance;
    return &_instance;
}

void LogHandler::setup()
{
    QString configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    if (configDir.isEmpty() && QDir::root().mkpath(configDir)) {
        QFile loggingRulesFile(QDir(configDir).absoluteFilePath(qAppName() + ".qtlogging.rules"));
        if (loggingRulesFile.open(QFile::ReadOnly)) {
            QString rules = QString(loggingRulesFile.readAll());
            QLoggingCategory::setFilterRules(rules);
        }
    }
    qSetMessagePattern("[%{type}] %{function} - %{message}");
}

void LogHandler::install(const QString &logFileBaseName)
{
    m_logFileBaseName = logFileBaseName;
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
        if (!m_logFileBaseName.isEmpty()) {
            QMutexLocker writeLocker(&m_writeLogMutex);

            QFile logFile(QString("%1.%2.log")
                          .arg(m_logFileBaseName)
                          .arg(QDate::currentDate().toString("yyyyMMdd")));

            if (logFile.open(QFile::Append | QFile::Text)) {
                QString logLine = QString("[%1][%2] (%3:%4 %5) %6 - %7\n")
                        .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(stringifiedTypes[type])
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

void LogHandler::uninstall()
{
    qInstallMessageHandler(0);
}

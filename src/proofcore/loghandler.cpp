#include "loghandler.h"

#include <QMap>
#include <QFile>
#include <QDateTime>
#include <QtMessageHandler>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>

using namespace Proof;

static QString logFileBaseName;
static QtMessageHandler coreHandler = nullptr;
static QMutex logFileWriteMutex;

static QMap<QtMsgType, QString> stringifiedTypes = {
    { QtDebugMsg, "D"},
    { QtWarningMsg, "W"},
    { QtCriticalMsg, "C"},
    { QtFatalMsg, "F"},
    { QtSystemMsg, "S"}
};

LogHandler::LogHandler()
{
    coreHandler = qInstallMessageHandler(0);
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
    qSetMessagePattern("[%{type}][%{function}] %{message}");
}

void LogHandler::install(const QString &fileName)
{
    logFileBaseName = fileName;
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
        if (!logFileBaseName.isEmpty()) {
            QMutexLocker writeLocker(&logFileWriteMutex);

            QFile logFile(QString("%1.%2.log")
                          .arg(logFileBaseName)
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
                logFile.close();
            }
        }
        if (coreHandler)
            coreHandler(type, context, message);
    });
}

void LogHandler::uninstall()
{
    qInstallMessageHandler(0);
}

#include "logs.h"

#include <QMap>
#include <QFile>
#include <QDateTime>
#include <QtMessageHandler>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDir>
#include <QMutex>

static bool isConsoleOutputEnabled = true;
static QString logsStoragePath;
static QString logFileNameBase;
static QtMessageHandler defaultHandler = nullptr;
static QMutex logFileWriteMutex;

static QMap<QtMsgType, QString> stringifiedTypes = {
    { QtDebugMsg, "D"},
    { QtWarningMsg, "W"},
    { QtCriticalMsg, "C"},
    { QtFatalMsg, "F"},
    { QtSystemMsg, "S"}
};

void fileHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (!logFileNameBase.isEmpty()) {
        QMutexLocker writeLocker(&logFileWriteMutex);

        QFile logFile(QString("%1/%2.%3.log")
                      .arg(logsStoragePath)
                      .arg(logFileNameBase)
                      .arg(QDate::currentDate().toString("yyyyMMdd")));

        if (logFile.open(QFile::Append | QFile::Text)) {
            QString logLine = QString("[%1][%2][%3][%4@%5:%6] %7\n")
                    .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                    .arg(stringifiedTypes[type])
                    .arg(context.category)
                    .arg(context.function)
                    .arg(QString(context.file).remove(QRegularExpression("^(\\.\\.[/\\\\])+")))
                    .arg(context.line)
                    .arg(message);

            logFile.write(logLine.toLocal8Bit());
            logFile.close();
        }
    }

    if (isConsoleOutputEnabled && defaultHandler)
        defaultHandler(type, context, message);
}

void consoleHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (isConsoleOutputEnabled && defaultHandler)
        defaultHandler(type, context, message);
}

void Proof::Logs::setup()
{
#ifdef Q_OS_WIN
    //Windows already gives us org/app as part of conf location
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
    QString configPath = QString("%1/%2")
            .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
            .arg(qApp->organizationName());
#endif

    if (!configPath.isEmpty() && QDir::root().mkpath(configPath)) {
        QFile loggingRulesFile(QDir(configPath).absoluteFilePath(qApp->applicationName() + ".qtlogging.rules"));
        if (loggingRulesFile.open(QFile::ReadOnly)) {
            QString rules = QString(loggingRulesFile.readAll());
            QLoggingCategory::setFilterRules(rules);
        } else {
            loggingRulesFile.open(QFile::WriteOnly|QFile::Append);
        }
    }
    qSetMessagePattern("[%{type}][%{function}] %{message}");

    QtMessageHandler oldHandler = qInstallMessageHandler(&consoleHandler);
    if (!defaultHandler)
        defaultHandler = oldHandler;
}

void Proof::Logs::setLogsStoragePath(QString storagePath)
{
    if (storagePath.isEmpty())
        storagePath = QString("%1/%2/prooflogs")
                .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                .arg(qApp->organizationName());
    logsStoragePath = storagePath;
    QDir logsDir = QDir(logsStoragePath);
    logsDir.mkpath(".");
}

void Proof::Logs::setRulesFromString(const QString &rulesString)
{
    QLoggingCategory::setFilterRules(rulesString);
}

void Proof::Logs::setConsoleOutputEnabled(bool enabled)
{
    isConsoleOutputEnabled = enabled;
}

void Proof::Logs::installFileHandler(const QString &fileName)
{
    if (fileName.isEmpty())
        uninstallFileHandler();
    logFileNameBase = fileName;
    QtMessageHandler oldHandler = qInstallMessageHandler(&fileHandler);
    if (!defaultHandler)
        defaultHandler = oldHandler;
}

void Proof::Logs::uninstallFileHandler()
{
    qInstallMessageHandler(&consoleHandler);
}

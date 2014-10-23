#include "loghandler.h"

#include <QMap>
#include <QFile>
#include <QDateTime>
#include <QtMessageHandler>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QRegularExpression>
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
    //TODO: consider add setter for it if will be needed later
    QDir genericDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    genericDataDir.mkpath("Opensoft/prooflogs");
    genericDataDir.cd("Opensoft/prooflogs");

    logFileBaseName = genericDataDir.absoluteFilePath(fileName);
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &message) {
        if (!logFileBaseName.isEmpty()) {
            QMutexLocker writeLocker(&logFileWriteMutex);

            QFile logFile(QString("%1.%2.log")
                          .arg(logFileBaseName)
                          .arg(QDate::currentDate().toString("yyyyMMdd")));

            if (logFile.open(QFile::Append | QFile::Text)) {
                QString logLine = QString("[%1][%2][%3][%4@%5:%6] %7\n")
                        .arg(stringifiedTypes[type])
                        .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
                        .arg(context.category)
                        .arg(context.function)
                        .arg(QString(context.file).remove(QRegularExpression("^(\\.\\.[/\\\\])+")))
                        .arg(context.line)
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

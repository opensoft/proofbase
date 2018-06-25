#include "logs.h"

#include "errornotifier.h"
#include "proofalgorithms.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QMutex>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThreadPool>
#include <QTimer>
#include <QtMessageHandler>

#include <zlib.h>

static const int COMPRESS_TIMEOUT = 24 * 60 * 60 * 1000; // 1 day

static const QMap<QtMsgType, QString> STRINGIFIED_TYPES = {{QtDebugMsg, "D"},
                                                           {QtWarningMsg, "W"},
                                                           {QtCriticalMsg, "C"},
                                                           {QtFatalMsg, "F"},
                                                           {QtInfoMsg, "I"}};

static const QSet<QtMsgType> TYPES_FOR_NOTIFIER = {QtWarningMsg, QtCriticalMsg, QtFatalMsg};
static const QVector<QLatin1String> NOTIFIER_EXCLUDES = {QLatin1String("QML Image"), QLatin1String("Binding loop"),
                                                         QLatin1String("QEGLPlatformContext")};

static bool isConsoleOutputEnabled = true;
static QString logsStoragePath;
static QString logFileNameBase;
static QtMessageHandler defaultHandler = nullptr;
static QMutex logFileWriteMutex;
static QTimer oldLogsArchiver;

static QFile *currentLogFile = nullptr;
static QDate currentLogFileDate;

namespace {
class DetachedArchiver : public QRunnable
{
    Q_DISABLE_COPY(DetachedArchiver)
public:
    DetachedArchiver(const QString &filePath) : m_filePath(filePath) {}

    void run() override
    {
        QFile file(m_filePath);
        if (file.open(QFile::ReadOnly)) {
            QTime compressTimer;
            compressTimer.start();
            qCDebug(proofCoreLoggerLog) << "Compressing" << m_filePath;
            gzFile fi = gzopen(QStringLiteral("%1.gz").arg(m_filePath).toLocal8Bit().constData(), "wb2");
            while (!file.atEnd()) {
                QByteArray fileData = file.read(1048576);
                gzwrite(fi, fileData.constData(), fileData.length());
            }
            gzclose(fi);
            file.remove();
            qCDebug(proofCoreLoggerLog) << m_filePath << "compressed in" << compressTimer.elapsed() << "msecs";
        }
    }

private:
    QString m_filePath;
};

void baseHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (isConsoleOutputEnabled && defaultHandler)
        defaultHandler(type, context, message);

    if (TYPES_FOR_NOTIFIER.contains(type)) {
        Proof::ErrorNotifier::Severity severity = Proof::ErrorNotifier::Severity::Warning;
        switch (type) {
        case QtCriticalMsg:
        case QtFatalMsg:
            severity = Proof::ErrorNotifier::Severity::Critical;
            break;
        case QtWarningMsg:
        default:
            severity = Proof::ErrorNotifier::Severity::Warning;
            break;
        }
        if (!Proof::algorithms::exists(NOTIFIER_EXCLUDES, [message](const auto &x) { return message.contains(x); }))
            Proof::ErrorNotifier::instance()->notify(QString("%1: %2").arg(context.category).arg(message), severity);
    }
}

void fileHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    baseHandler(type, context, message);

    if (!logFileNameBase.isEmpty()) {
        QMutexLocker writeLocker(&logFileWriteMutex);
        if (currentLogFile && (QDate::currentDate() != currentLogFileDate || !currentLogFile->isOpen())) {
            delete currentLogFile;
            currentLogFile = nullptr;
        }
        if (!currentLogFile) {
            currentLogFileDate = QDate::currentDate();
            currentLogFile = new QFile(
                QStringLiteral("%1/%2.%3.log")
                    .arg(logsStoragePath, logFileNameBase, currentLogFileDate.toString(QStringLiteral("yyyyMMdd"))));
            if (!currentLogFile->open(QFile::Append | QFile::Text))
                return;
        }

        QString logLine = QStringLiteral("[%1][%2][%3][%4@%5:%6] %7\n")
                              .arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz")))
                              .arg(STRINGIFIED_TYPES.value(type, QStringLiteral("D")))
                              .arg(context.category)
                              .arg(context.function) // clazy:exclude=qstring-arg
                              .arg(QString(context.file).remove(QRegularExpression(QStringLiteral("^(\\.\\.[/\\\\])+"))))
                              .arg(context.line)
                              .arg(message);

        currentLogFile->write(logLine.toLocal8Bit());
        currentLogFile->flush();
    }
}

void compressOldFiles()
{
    QFileInfoList files = QDir(logsStoragePath).entryInfoList(QDir::Files | QDir::NoSymLinks);

    for (const QFileInfo &file : files) {
        const auto suffix = file.suffix();
        const auto fullSuffix = file.completeSuffix();
        if (suffix != QLatin1String("gz")
            && fullSuffix != QStringLiteral("%1.log").arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd")))
            && fullSuffix != QStringLiteral("%1.log").arg(currentLogFileDate.toString(QStringLiteral("yyyyMMdd")))) {
            QThreadPool::globalInstance()->start(new DetachedArchiver(file.absoluteFilePath()));
        }
    }
}
} // namespace

void Proof::Logs::setup(const QStringList &defaultLoggingRules)
{
#if defined Q_OS_WIN
    //Windows already gives us org/app as part of conf location
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#elif defined Q_OS_ANDROID
    QString configPath = QString("%1/%2")
                             .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                             .arg(qApp->organizationName());
#else
    QString configPath = QStringLiteral("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
                                                     qApp->organizationName());
#endif

    if (!configPath.isEmpty() && QDir::root().mkpath(configPath)) {
        QFile loggingRulesFile(QDir(configPath).absoluteFilePath(qApp->applicationName() + ".qtlogging.rules"));
        if (!loggingRulesFile.exists()) {
            QString defaultRules = QString("proof.core.cache=false\n"
                                           "proof.core.taskchain.extra=false\n"
                                           "proof.core.taskchain.stats=false\n"
                                           "proof.core.tasks.extra=false\n"
                                           "proof.core.tasks.stats=false\n"
                                           "proof.core.futures.*=false\n"
                                           "%1\n")
                                       .arg(defaultLoggingRules.join(QStringLiteral("\n")));
            if (loggingRulesFile.open(QFile::WriteOnly | QFile::Append))
                loggingRulesFile.write(defaultRules.toLatin1());
            QLoggingCategory::setFilterRules(defaultRules);
        } else if (loggingRulesFile.open(QFile::ReadOnly)) {
            QString rules = QString(loggingRulesFile.readAll());
            QLoggingCategory::setFilterRules(rules);
        }
    }
    qSetMessagePattern(QStringLiteral("[%{type}][%{function}] %{message}"));

    QtMessageHandler oldHandler = qInstallMessageHandler(&baseHandler);
    if (!defaultHandler)
        defaultHandler = oldHandler;
}

void Proof::Logs::setLogsStoragePath(QString storagePath)
{
    if (storagePath.isEmpty()) {
        storagePath = QStringLiteral("%1/%2/prooflogs")
                          .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation),
                               qApp->organizationName());
    }
    logsStoragePath = storagePath;
    QDir logsDir = QDir(logsStoragePath);
    logsDir.mkpath(QStringLiteral("."));

    QObject::connect(&oldLogsArchiver, &QTimer::timeout, &oldLogsArchiver, []() { compressOldFiles(); });
    compressOldFiles();
    oldLogsArchiver.setTimerType(Qt::VeryCoarseTimer);
    oldLogsArchiver.start(COMPRESS_TIMEOUT);
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
    qInstallMessageHandler(&baseHandler);
}

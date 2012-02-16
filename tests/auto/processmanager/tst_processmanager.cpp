/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest>
#include <QtCore/QMetaType>
#include <QFileInfo>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeProperty>
#include <QLocalSocket>

#include "process.h"
#include "processbackendmanager.h"
#include "processmanager.h"
#include "processinfo.h"
#include "prelaunchprocessbackendfactory.h"
#include "standardprocessbackendfactory.h"
#include "processbackend.h"
#include "processfrontend.h"
#include "qjsondocument.h"
#include "pipeprocessbackendfactory.h"
#include "socketprocessbackendfactory.h"

#include <signal.h>

QT_USE_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QProcess::ExitStatus);
Q_DECLARE_METATYPE(QProcess::ProcessState);
Q_DECLARE_METATYPE(QProcess::ProcessError);

/******************************************************************************/

const char *exitStatusToString[] = {
    "NormalExit",
    "CrashExit"
};

const char *stateToString[] = {
    "NotRunning",
    "Starting",
    "Running"
};

const char *errorToString[] = {
    "FailedToStart",
    "Crashed",
    "Timedout",
    "WriteError",
    "ReadError",
    "UnknownError"
};

class ErrorSpy : public QObject {
    Q_OBJECT
public:
    ErrorSpy(ProcessBackend *target) {connect(target, SIGNAL(error(QProcess::ProcessError)),
                                              SLOT(handleError(QProcess::ProcessError))); }
    ErrorSpy(ProcessFrontend *target) {connect(target, SIGNAL(error(QProcess::ProcessError)),
                                              SLOT(handleError(QProcess::ProcessError))); }

    int count() const { return m_errors.size(); }
    QProcess::ProcessError at(int i) const { return m_errors.at(i); }
    QString                atString(int i) const { return m_errorStrings.at(i); }

private slots:
    void handleError(QProcess::ProcessError err) {
        m_errors << err;
        ProcessBackend *backend = qobject_cast<ProcessBackend *>(sender());
        if (backend)
            m_errorStrings << backend->errorString();
        else
            m_errorStrings << qobject_cast<ProcessFrontend *>(sender())->errorString();
    }
private:
    QList<QProcess::ProcessError> m_errors;
    QList<QString>                m_errorStrings;
};


bool canCheckProcessState()
{
    QFileInfo finfo("/bin/ps");
    return finfo.exists();
}

static QRegExp gProcessRegex("\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)");

bool isProcessRunning(Q_PID pid, qint64 *uid=0, qint64 *gid=0)
{
    QProcess p;
    p.start("/bin/ps", QStringList() << "-o" << "pid=" << "-o" << "uid="
            << "-o" << "gid=" << "-p" << QString::number(pid));
    if (p.waitForStarted() && p.waitForFinished()) {
        QList<QByteArray> plist = p.readAll().split('\n');
        if (plist.size() == 2 && gProcessRegex.exactMatch(QString::fromLocal8Bit(plist.at(0)))) {
            if (gProcessRegex.cap(1).toLongLong() == pid) {
                if (uid)
                    *uid = gProcessRegex.cap(2).toLongLong();
                if (gid)
                    *gid = gProcessRegex.cap(3).toLongLong();
                return true;
            }
        }
    }

    return false;
}

bool isProcessStopped(Q_PID pid)
{
    QProcess p;
    p.start("/bin/ps", QStringList() << "-o" << "pid=" << "-p" << QString::number(pid));
    if (p.waitForStarted() && p.waitForFinished()) {
        QList<QByteArray> plist = p.readAll().split('\n');
        return plist.size() == 1 && !plist.at(0).size();
    }
    return false;
}

static void waitForInternalProcess(ProcessBackendManager *manager, int num=1, int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out");
        if (manager->internalProcesses().count() == num)
            break;
        QTestEventLoop::instance().enterLoop(1);
    }
}

static void waitForSocket(const QString& socketname, int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out");
        else {
            QLocalSocket socket;
            socket.connectToServer(socketname);
            if (socket.waitForConnected(timeout))
                break;
        }
        QTestEventLoop::instance().enterLoop(1);
    }
}

static void waitForPriority(ProcessBackend *process, int priority, int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out");
        if (process->actualPriority() == priority)
            break;
        QTestEventLoop::instance().enterLoop(1);
    }
}


/**********************************************************************/

class Spy {
public:
    Spy(ProcessBackend *process)
    : stateSpy(process, SIGNAL(stateChanged(QProcess::ProcessState)))
    , startSpy(process, SIGNAL(started()))
    , errorSpy(process)
    , finishedSpy(process, SIGNAL(finished(int, QProcess::ExitStatus)))
    , stdoutSpy(process, SIGNAL(standardOutput(const QByteArray&)))
    , stderrSpy(process, SIGNAL(standardError(const QByteArray&))) {}

    Spy(ProcessFrontend *process)
    : stateSpy(process, SIGNAL(stateChanged(QProcess::ProcessState)))
    , startSpy(process, SIGNAL(started()))
    , errorSpy(process)
    , finishedSpy(process, SIGNAL(finished(int, QProcess::ExitStatus)))
    , stdoutSpy(process, SIGNAL(standardOutput(const QByteArray&)))
    , stderrSpy(process, SIGNAL(standardError(const QByteArray&))) {}

    void check( int startCount, int errorCount, int finishedCount, int stateCount ) {
        QVERIFY(startSpy.count() == startCount);
        QVERIFY(errorSpy.count() == errorCount);
        QVERIFY(finishedSpy.count() == finishedCount);
        QVERIFY(stateSpy.count() == stateCount);
        bool failedToStart = false;
        for (int i = 0 ; i < errorSpy.count() ; i++)
            if (errorSpy.at(i) == QProcess::FailedToStart)
                failedToStart = true;

        if (failedToStart)
            QVERIFY(stateCount <=2);
        if (stateCount > 0)
            QCOMPARE(qVariantValue<QProcess::ProcessState>(stateSpy.at(0).at(0)), QProcess::Starting);
        if (stateCount > 1)
            QCOMPARE(qVariantValue<QProcess::ProcessState>(stateSpy.at(1).at(0)),
                     (failedToStart ? QProcess::NotRunning : QProcess::Running));
        if (stateCount > 2)
            QCOMPARE(qVariantValue<QProcess::ProcessState>(stateSpy.at(2).at(0)), QProcess::NotRunning);
    }

    void checkStdout(const QByteArray s) {
        for (int i = 0 ; i < stdoutSpy.count() ; i++) {
            QByteArray b = stdoutSpy.at(i).at(0).toByteArray();
            if (b == s)
                return;
        }
        QFAIL("String not found");
    }

    void waitStart(int timeout=5000) {
        stopWatch.restart();
        forever {
            if (startSpy.count())
                break;
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            QTestEventLoop::instance().enterLoop(1);
        }
    }

    void waitFailedStart(int timeout=5000) {
        stopWatch.restart();
        forever {
            if (errorSpy.count())
                break;
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            QTestEventLoop::instance().enterLoop(1);
        }
    }

    void waitFinished(int timeout=5000) {
        stopWatch.restart();
        forever {
            if (finishedSpy.count())
                break;
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            QTestEventLoop::instance().enterLoop(1);
        }
    }

    void waitStdout(int timeout=5000) {
        stopWatch.restart();
        int count = stdoutSpy.count();
        forever {
            if (stdoutSpy.count() != count)
                break;
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            QTestEventLoop::instance().enterLoop(1);
        }
    }

    void checkExitCode(int exitCode) {
        QVERIFY(finishedSpy.count() == 1);
        QCOMPARE(qVariantValue<int>(finishedSpy.at(0).at(0)), exitCode);
    }

    void checkExitStatus(QProcess::ExitStatus exitStatus) {
        QVERIFY(finishedSpy.count() == 1);
        QCOMPARE(qVariantValue<QProcess::ExitStatus>(finishedSpy.at(0).at(1)), exitStatus);
    }

    void checkErrors(const QList<QProcess::ProcessError>& list) {
        QCOMPARE(errorSpy.count(), list.count());
        for (int i = 0 ; i < errorSpy.count() ; i++)
            QCOMPARE(errorSpy.at(i), list.at(i));
    }

    void dump() {
        qDebug() << "================================ SPY DUMP ==============================";
        qDebug() << "Start count=" << startSpy.count();
        qDebug() << "Finished count=" << finishedSpy.count();
        for (int i = 0 ; i < finishedSpy.count() ; i++) {
            qDebug() << "....(" << qvariant_cast<int>(finishedSpy.at(i).at(0))
                     << ")" << exitStatusToString[qvariant_cast<QProcess::ExitStatus>(finishedSpy.at(i).at(0))];
        }
        qDebug() << "State count=" << stateSpy.count();
        for (int i = 0 ; i < stateSpy.count() ; i++) {
            qDebug() << "...." << stateToString[qvariant_cast<QProcess::ProcessState>(stateSpy.at(i).at(0))];
        }
        qDebug() << "Error count=" << errorSpy.count();
        for (int i = 0 ; i < errorSpy.count() ; i++) {
            qDebug() << "...." << errorToString[errorSpy.at(i)];
        }
        qDebug() << "================================ ======== ==============================";
    }

    QTime      stopWatch;
    QSignalSpy stateSpy;
    QSignalSpy startSpy;
    ErrorSpy   errorSpy;
    QSignalSpy finishedSpy;
    QSignalSpy stdoutSpy;
    QSignalSpy stderrSpy;
};

/******************************************************************************/

static void writeLine(ProcessBackend *process, const char *command)
{
    QByteArray data(command);
    data += '\n';
    process->write(data);
}

static void writeJson(ProcessBackend *process, const char *command)
{
    QVariantMap map;
    map.insert("command", command);
    process->write(QJsonDocument::fromVariant(map).toBinaryData());
}

static void verifyRunning(ProcessBackend *process)
{
    QVERIFY(process->state() == QProcess::Running);
    pid_t pid = process->pid();
    pid_t pgrp = ::getpgid(pid);
    QVERIFY(pid != 0);
    QCOMPARE(pgrp, pid);

    qint64 uid, gid;
    QVERIFY(isProcessRunning(pid, &uid, &gid));
    QString uidString = qgetenv("TEST_UID");
    QString gidString = qgetenv("TEST_GID");
    if (!uidString.isEmpty())
        QCOMPARE(uid, uidString.toLongLong());
    if (!gidString.isEmpty())
        QCOMPARE(gid, gidString.toLongLong());
}

static void cleanupProcess(ProcessBackend *process)
{
    QVERIFY(process->state() == QProcess::NotRunning);
    QVERIFY(process->parent() == NULL);
    delete process;
}

typedef void (*CommandFunc)(ProcessBackend *, const char *);

static void startAndStopClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);
    func(process, "stop");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

static void startAndKillClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    Q_UNUSED(func);

    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);

    process->stop();
    spy.waitFinished();
    spy.check(1,1,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::CrashExit);
    spy.checkErrors(QList<QProcess::ProcessError>() << QProcess::Crashed);

    cleanupProcess(process);
}

static void startAndCrashClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);

    func(process, "crash");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(2);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

static void failToStartClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    Q_UNUSED(func);
    info.setValue("program", "thisProgramDoesntExist");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitFailedStart();
    spy.check(0,1,0,2);

    cleanupProcess(process);
}

static void echoClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);

    func(process, "echotest");
    spy.waitStdout();
    spy.checkStdout("echotest\n");

    func(process, "stop");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

static void priorityChangeBeforeClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    info.setValue("priority", 19);
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);
    QCOMPARE(process->actualPriority(), 19);

    func(process, "stop");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

static void priorityChangeAfterClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);

    process->setDesiredPriority(19);
    waitForPriority(process, 19);

    func(process, "stop");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

#if defined(Q_OS_LINUX)

static void waitForOom(ProcessBackend *process, int oom, int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (process->actualOomAdjustment() == oom)
            break;
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out");
        QTestEventLoop::instance().enterLoop(1);
    }
}

static void oomChangeBeforeClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    info.setOomAdjustment(500);
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);
    QCOMPARE(process->actualOomAdjustment(), 500);

    func(process, "stop");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

static void oomChangeAfterClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    spy.check(1,0,0,2);

    process->setDesiredOomAdjustment(499);
    waitForOom(process, 499);

    func(process, "stop");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    cleanupProcess(process);
}

#endif // defined(Q_OS_LINUX)


typedef void (*clientFunc)(ProcessBackendManager *, ProcessInfo, CommandFunc);

static void fixUidGid(ProcessInfo& info)
{
    QString uidString = qgetenv("TEST_UID");
    QString gidString = qgetenv("TEST_GID");
    if (!uidString.isEmpty())
        info.setUid(uidString.toLongLong());
    if (!gidString.isEmpty())
        info.setGid(gidString.toLongLong());
}

static void standardFactoryTest( clientFunc func )
{
    ProcessBackendManager *manager = new ProcessBackendManager;

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    fixUidGid(info);
    manager->addFactory(new StandardProcessBackendFactory);

    func(manager, info, writeLine);
    delete manager;
}

static void prelaunchFactoryTest( clientFunc func )
{
    ProcessBackendManager *manager = new ProcessBackendManager;

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    manager->addFactory(new PrelaunchProcessBackendFactory(info));

    // Verify that there is a prelaunched process
    QVERIFY(manager->memoryRestricted() == false);
    waitForInternalProcess(manager);

    info.setValue("prelaunch", "true");
    fixUidGid(info);
    func(manager, info, writeJson);
    delete manager;
}

static void prelaunchRestrictedTest( clientFunc func )
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->setMemoryRestricted(true);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    manager->addFactory(new PrelaunchProcessBackendFactory(info));

    QVERIFY(manager->memoryRestricted() == true);

    info.setValue("prelaunch", "true");
    fixUidGid(info);
    func(manager, info, writeJson);
    delete manager;
}

static void pipeLauncherTest( clientFunc func )
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    manager->addFactory(new PipeProcessBackendFactory(info));

    // Wait for the factory to have launched a pipe
    waitForInternalProcess(manager);
    QVERIFY(manager->internalProcesses().count() == 1);

    ProcessInfo info2;
    info2.setValue("program", "testClient/testClient");
    info2.setValue("pipe", "true");
    fixUidGid(info2);
    func(manager, info2, writeLine);
    delete manager;
}


static void socketLauncherTest( clientFunc func )
{
    QProcess *remote = new QProcess;
    QString socketName = QLatin1String("/tmp/socketlauncher");
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher", QStringList() << socketName);
    QVERIFY(remote->waitForStarted());
    waitForSocket(socketName);

    ProcessBackendManager *manager = new ProcessBackendManager;
    SocketProcessBackendFactory *factory = new SocketProcessBackendFactory;
    factory->setSocketName(socketName);
    manager->addFactory(factory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    fixUidGid(info);
    func(manager, info, writeLine);

    delete manager;
    delete remote;
}


/******************************************************************************/

class tst_ProcessManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void standardStartAndStop()         { standardFactoryTest(startAndStopClient); }
    void standardStartAndKill()         { standardFactoryTest(startAndKillClient); }
    void standardStartAndCrash()        { standardFactoryTest(startAndCrashClient); }
    void standardFailToStart()          { standardFactoryTest(failToStartClient); }
    void standardEcho()                 { standardFactoryTest(echoClient); }
    void standardPriorityChangeBefore() { standardFactoryTest(priorityChangeBeforeClient); }
    void standardPriorityChangeAfter()  { standardFactoryTest(priorityChangeAfterClient); }
#if defined(Q_OS_LINUX)
    void standardOomChangeBefore()      { standardFactoryTest(oomChangeBeforeClient); }
    void standardOomChangeAfter()       { standardFactoryTest(oomChangeAfterClient); }
#endif

    void prelaunchStartAndStop()         { prelaunchFactoryTest(startAndStopClient); }
    void prelaunchStartAndKill()         { prelaunchFactoryTest(startAndKillClient); }
    void prelaunchStartAndCrash()        { prelaunchFactoryTest(startAndCrashClient); }
    void prelaunchEcho()                 { prelaunchFactoryTest(echoClient); }
    void prelaunchPriorityChangeBefore() { prelaunchFactoryTest(priorityChangeBeforeClient); }
    void prelaunchPriorityChangeAfter()  { prelaunchFactoryTest(priorityChangeAfterClient); }
#if defined(Q_OS_LINUX)
    void prelaunchOomChangeBefore()      { prelaunchFactoryTest(oomChangeBeforeClient); }
    void prelaunchOomChangeAfter()       { prelaunchFactoryTest(oomChangeAfterClient); }
#endif

    void prelaunchRestrictedStartAndStop()         { prelaunchRestrictedTest(startAndStopClient); }
    void prelaunchRestrictedStartAndKill()         { prelaunchRestrictedTest(startAndKillClient); }
    void prelaunchRestrictedStartAndCrash()        { prelaunchRestrictedTest(startAndCrashClient); }
    void prelaunchRestrictedEcho()                 { prelaunchRestrictedTest(echoClient); }
    void prelaunchRestrictedPriorityChangeBefore() { prelaunchRestrictedTest(priorityChangeBeforeClient); }
    void prelaunchRestrictedPriorityChangeAfter()  { prelaunchRestrictedTest(priorityChangeAfterClient); }
#if defined(Q_OS_LINUX)
    void prelaunchRestrictedOomChangeBefore()      { prelaunchRestrictedTest(oomChangeBeforeClient); }
    void prelaunchRestrictedOomChangeAfter()       { prelaunchRestrictedTest(oomChangeAfterClient); }
#endif

    void pipeLauncherStartAndStop()         { pipeLauncherTest(startAndStopClient); }
    void pipeLauncherStartAndKill()         { pipeLauncherTest(startAndKillClient); }
    void pipeLauncherStartAndCrash()        { pipeLauncherTest(startAndCrashClient); }
    void pipeLauncherEcho()                 { pipeLauncherTest(echoClient); }
    void pipeLauncherPriorityChangeBefore() { pipeLauncherTest(priorityChangeBeforeClient); }
    void pipeLauncherPriorityChangeAfter()  { pipeLauncherTest(priorityChangeAfterClient); }
#if defined(Q_OS_LINUX)
    void pipeLauncherOomChangeBefore()      { pipeLauncherTest(oomChangeBeforeClient); }
    void pipeLauncherOomChangeAfter()       { pipeLauncherTest(oomChangeAfterClient); }
#endif

    void socketLauncherStartAndStop()         { socketLauncherTest(startAndStopClient); }
    void socketLauncherStartAndKill()         { socketLauncherTest(startAndKillClient); }
    void socketLauncherStartAndCrash()        { socketLauncherTest(startAndCrashClient); }
    void socketLauncherEcho()                 { socketLauncherTest(echoClient); }
    void socketLauncherPriorityChangeBefore() { socketLauncherTest(priorityChangeBeforeClient); }
    void socketLauncherPriorityChangeAfter()  { socketLauncherTest(priorityChangeAfterClient); }
#if defined(Q_OS_LINUX)
    void socketLauncherOomChangeBefore()      { socketLauncherTest(oomChangeBeforeClient); }
    void socketLauncherOomChangeAfter()       { socketLauncherTest(oomChangeAfterClient); }
#endif

    void prelaunchChildAbort();

    void frontend();
    void subclassFrontend();
};


/**********************************************************************/

void tst_ProcessManager::initTestCase()
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessError");
}

void tst_ProcessManager::prelaunchChildAbort()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory(info);
    manager->addFactory(factory);

    // The factory should not have launched
    QVERIFY(manager->internalProcesses().count() == 0);

    waitForInternalProcess(manager, 1, factory->launchInterval() + 2000);
    Q_PID pid = manager->internalProcesses().at(0);
    // Kill the prelaunched process and verify that it is restarted
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForInternalProcess(manager, 1, factory->launchInterval() + 2000);
    delete manager;
}

void tst_ProcessManager::frontend()
{
    ProcessManager *manager = new ProcessManager;
    manager->addBackendFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessFrontend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    // Now send a "stop" message
    process->write("stop\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QCOMPARE(manager->size(), 1);
    delete process;
    QCOMPARE(manager->size(), 0);
    delete manager;
}

class TestProcess : public ProcessFrontend {
    Q_OBJECT
    Q_PROPERTY(QString magic READ magic WRITE setMagic NOTIFY magicChanged)
public:
    TestProcess(ProcessBackend *backend, QObject *parent=0) : ProcessFrontend(backend, parent) {}

    QString magic() const { return m_magic; }
    void    setMagic(const QString&s) { if (m_magic != s) { m_magic=s; emit magicChanged(); }}
signals:
    void magicChanged();
private:
    QString m_magic;
};

class TestManager : public ProcessManager {
    Q_OBJECT
    Q_PROPERTY(QString magic READ magic WRITE setMagic NOTIFY magicChanged)
public:
    TestManager(QObject *parent=0) : ProcessManager(parent) {}
    virtual Q_INVOKABLE TestProcess *createFrontend(ProcessBackend *backend) {return new TestProcess(backend);}
    QString magic() const { return m_magic; }
    void    setMagic(const QString&s) { if (m_magic != s) { m_magic=s; emit magicChanged(); }}
signals:
    void    magicChanged();
private:
    QString m_magic;
};

void tst_ProcessManager::subclassFrontend()
{
    TestManager *manager = new TestManager;
    manager->addBackendFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessFrontend *process = manager->create(info);
    QVERIFY(process);

    QVERIFY(process->setProperty("magic", 42));
    QCOMPARE(process->property("magic").toDouble(), 42.0);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    // Now send a "stop" message
    process->write("stop\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QCOMPARE(manager->size(), 1);
    delete process;
    QCOMPARE(manager->size(), 0);
    delete manager;
}

QTEST_MAIN(tst_ProcessManager)

#include "tst_processmanager.moc"

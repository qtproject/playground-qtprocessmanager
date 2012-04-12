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
#include "timeoutidledelegate.h"
#include "procutils.h"

#include <signal.h>

QT_USE_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QProcess::ExitStatus);
Q_DECLARE_METATYPE(QProcess::ProcessState);
Q_DECLARE_METATYPE(QProcess::ProcessError);

const int kProcessCount = 10;

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

static void waitForTimeout(int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (stopWatch.elapsed() >= timeout)
            break;
        QTestEventLoop::instance().enterLoop(1);
    }
}

static void waitForThreadCount(Q_PID pid, int count, int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (ProcUtils::getThreadCount(pid) == count)
            break;
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out waiting for thread count");
        QTestEventLoop::instance().enterLoop(1);
    }
}

static void waitForInternalProcess(ProcessBackendManager *manager, int num=1, int timeout=5000)
{
    QObject::connect(manager, SIGNAL(internalProcessesChanged()),
                     &QTestEventLoop::instance(), SLOT(exitLoop()));
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out");
        if (manager->internalProcesses().count() == num)
            break;
        QTestEventLoop::instance().enterLoop(1);
    }
    QObject::disconnect(manager, SIGNAL(internalProcessesChanged()),
                     &QTestEventLoop::instance(), SLOT(exitLoop()));
}

static void waitForInternalProcess(ProcessManager *manager, int num=1, int timeout=5000)
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

static void waitForSignal(QSignalSpy& spy, int count=1, int timeout=5000)
{
    QTime stopWatch;
    stopWatch.start();
    forever {
        if (spy.count() >= count)
            break;
        if (stopWatch.elapsed() >= timeout)
            QFAIL("Timed out");
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
        QCOMPARE(startSpy.count(), startCount);
        QCOMPARE(errorSpy.count(), errorCount);
        QCOMPARE(finishedSpy.count(), finishedCount);
        QCOMPARE(stateSpy.count(), stateCount);
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

    void waitStart(int timeout=5000) { waitForSignal(startSpy, 1, timeout); }
    void waitFinished(int timeout=5000) { waitForSignal(finishedSpy, 1, timeout); }
    void waitStdout(int timeout=5000) { waitForSignal(stdoutSpy, stdoutSpy.count() + 1, timeout); }

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

static void startAndStopMultiple(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    ProcessBackend *plist[kProcessCount];
    Spy            *slist[kProcessCount];

    for (int i = 0 ; i < kProcessCount ; i++) {
        ProcessBackend *process = manager->create(info);
        QVERIFY(process);
        QVERIFY(process->state() == QProcess::NotRunning);
        Spy *spy = new Spy(process);
        plist[i] = process;
        slist[i] = spy;
    }

    for (int i = 0 ; i < kProcessCount ; i++ )
        plist[i]->start();

    for (int i = 0 ; i < kProcessCount ; i++ ) {
        slist[i]->waitStart();
        verifyRunning(plist[i]);
        slist[i]->check(1,0,0,2);
    }

    for (int i = 0 ; i < kProcessCount ; i++ )
        func(plist[i], "stop");

    for (int i = 0 ; i < kProcessCount ; i++ ) {
        slist[i]->waitFinished();
        slist[i]->check(1,0,1,3);
        slist[i]->checkExitCode(0);
        slist[i]->checkExitStatus(QProcess::NormalExit);
    }

    for (int i = 0 ; i < kProcessCount ; i++) {
        cleanupProcess(plist[i]);
        delete slist[i];
    }
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

/* Dynamically check to see if oomAdjustments are available */

static bool canRunOomAdjustment()
{
#if defined(Q_OS_LINUX)
    QFile file(QString::fromLatin1("/proc/%1/oom_score_adj").arg(::getpid()));
    return file.exists();
#endif
    return false;
}

static void oomChangeBeforeClient(ProcessBackendManager *manager, ProcessInfo info, CommandFunc func)
{
    if (!canRunOomAdjustment())
        return;

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
    if (!canRunOomAdjustment())
        return;

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


typedef void (*clientFunc)(ProcessBackendManager *, ProcessInfo, CommandFunc);
typedef void (*infoFunc)(ProcessInfo&);

static void fixUidGid(ProcessInfo& info)
{
    QString uidString = qgetenv("TEST_UID");
    QString gidString = qgetenv("TEST_GID");
    if (!uidString.isEmpty())
        info.setUid(uidString.toLongLong());
    if (!gidString.isEmpty())
        info.setGid(gidString.toLongLong());
}

static void makeTough(ProcessInfo& info)
{
    QStringList args = info.arguments();
    args << QStringLiteral("-noterm");
    info.setArguments(args);
}

static void standardTest( clientFunc func, infoFunc infoFixup=0 )
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    if (infoFixup)
        infoFixup(info);
    fixUidGid(info);

    func(manager, info, writeLine);
    delete manager;
}

static void prelaunchTest( clientFunc func, infoFunc infoFixup=0 )
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->setIdleDelegate(new TimeoutIdleDelegate);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    if (infoFixup)
        infoFixup(info);

    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // Verify that there is a prelaunched process
    QVERIFY(manager->memoryRestricted() == false);
    waitForInternalProcess(manager);

    fixUidGid(info);
    func(manager, info, writeJson);
    delete manager;
}

static void prelaunchRestrictedTest( clientFunc func, infoFunc infoFixup=0 )
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->setMemoryRestricted(true);
    manager->setIdleDelegate(new TimeoutIdleDelegate);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    if (infoFixup)
        infoFixup(info);

    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    QVERIFY(manager->memoryRestricted() == true);

    fixUidGid(info);
    func(manager, info, writeJson);
    delete manager;
}

static void pipeLauncherTest( clientFunc func, infoFunc infoFixup=0 )
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    PipeProcessBackendFactory *factory = new PipeProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // Wait for the factory to have launched a pipe
    waitForInternalProcess(manager);
    QVERIFY(manager->internalProcesses().count() == 1);

    ProcessInfo info2;
    info2.setValue("program", "testClient/testClient");
    info2.setValue("pipe", "true");
    if (infoFixup)
        infoFixup(info2);
    fixUidGid(info2);
    func(manager, info2, writeLine);
    delete manager;
}


static void socketLauncherTest( clientFunc func, QStringList args=QStringList(), infoFunc infoFixup=0  )
{
    QProcess *remote = new QProcess;
    QString socketName = QStringLiteral("/tmp/socketlauncher");
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher", args << socketName);
    QVERIFY(remote->waitForStarted());
    waitForSocket(socketName);

    ProcessBackendManager *manager = new ProcessBackendManager;
    SocketProcessBackendFactory *factory = new SocketProcessBackendFactory;
    factory->setSocketName(socketName);
    manager->addFactory(factory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    if (infoFixup)
        infoFixup(info);
    fixUidGid(info);
    func(manager, info, writeLine);

    delete manager;
    delete remote;
}

static void socketSchemaTest( clientFunc func, infoFunc infoFixup=0 )
{
    QStringList args;
    args << "-validate-inbound" << "../../../schema/remote/inbound"
         << "-validate-outbound" << "../../../schema/remote/outbound"
         << "-warn" << "-drop";
    socketLauncherTest(func, args, infoFixup);
}

static void forkLauncherTest( clientFunc func, infoFunc infoFixup=0  )
{
#if defined(Q_OS_LINUX)
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testForkLauncher/testForkLauncher");
    PipeProcessBackendFactory *factory = new PipeProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // Wait for the factory to have launched a pipe
    waitForInternalProcess(manager);
    QVERIFY(manager->internalProcesses().count() == 1);

    ProcessInfo info2;
    fixUidGid(info2);
    if (infoFixup)
        infoFixup(info2);
    func(manager, info2, writeLine);
    delete manager;
#else
    Q_UNUSED(func);
    Q_UNUSED(infoFixup);
#endif
}

static void preforkLauncherTest( clientFunc func, infoFunc infoFixup=0 )
{
#if defined(Q_OS_LINUX)
    QProcess *remote = new QProcess;
    QString socketName = QStringLiteral("/tmp/preforklauncher");
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args;
    args << "--" << "testPreforkLauncher/testPreforkLauncher" << socketName
         << "--" << "testForkLauncher/testForkLauncher";
    remote->start("testPrefork/testPrefork", args);
    QVERIFY(remote->waitForStarted());
    waitForSocket(socketName);

    ProcessBackendManager *manager = new ProcessBackendManager;
    SocketProcessBackendFactory *factory = new SocketProcessBackendFactory;
    factory->setSocketName(socketName);
    manager->addFactory(factory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    if (infoFixup)
        infoFixup(info);
    fixUidGid(info);
    func(manager, info, writeLine);

    delete manager;
    delete remote;
#else
    Q_UNUSED(func);
    Q_UNUSED(infoFixup);
#endif
}







/******************************************************************************/

class tst_ProcessManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void standardStartAndStop()         { standardTest(startAndStopClient); }
    void standardStartAndStopMultiple() { standardTest(startAndStopMultiple); }
    void standardStartAndKill()         { standardTest(startAndKillClient); }
    void standardStartAndKillTough()    { standardTest(startAndKillClient, makeTough); }
    void standardStartAndCrash()        { standardTest(startAndCrashClient); }
    void standardFailToStart()          { standardTest(failToStartClient); }
    void standardEcho()                 { standardTest(echoClient); }
    void standardPriorityChangeBefore() { standardTest(priorityChangeBeforeClient); }
    void standardPriorityChangeAfter()  { standardTest(priorityChangeAfterClient); }
    void standardOomChangeBefore()      { standardTest(oomChangeBeforeClient); }
    void standardOomChangeAfter()       { standardTest(oomChangeAfterClient); }

    void prelaunchStartAndStop()         { prelaunchTest(startAndStopClient); }
    void prelaunchStartAndStopMultiple() { prelaunchTest(startAndStopMultiple); }
    void prelaunchStartAndKill()         { prelaunchTest(startAndKillClient); }
    void prelaunchStartAndKillTough()    { prelaunchTest(startAndKillClient, makeTough); }
    void prelaunchStartAndCrash()        { prelaunchTest(startAndCrashClient); }
    void prelaunchEcho()                 { prelaunchTest(echoClient); }
    void prelaunchPriorityChangeBefore() { prelaunchTest(priorityChangeBeforeClient); }
    void prelaunchPriorityChangeAfter()  { prelaunchTest(priorityChangeAfterClient); }
    void prelaunchOomChangeBefore()      { prelaunchTest(oomChangeBeforeClient); }
    void prelaunchOomChangeAfter()       { prelaunchTest(oomChangeAfterClient); }

    void prelaunchRestrictedStartAndStop()         { prelaunchRestrictedTest(startAndStopClient); }
    void prelaunchRestrictedStartAndStopMultiple() { prelaunchRestrictedTest(startAndStopMultiple); }
    void prelaunchRestrictedStartAndKill()         { prelaunchRestrictedTest(startAndKillClient); }
    void prelaunchRestrictedStartAndKillTough()    { prelaunchRestrictedTest(startAndKillClient, makeTough); }
    void prelaunchRestrictedStartAndCrash()        { prelaunchRestrictedTest(startAndCrashClient); }
    void prelaunchRestrictedEcho()                 { prelaunchRestrictedTest(echoClient); }
    void prelaunchRestrictedPriorityChangeBefore() { prelaunchRestrictedTest(priorityChangeBeforeClient); }
    void prelaunchRestrictedPriorityChangeAfter()  { prelaunchRestrictedTest(priorityChangeAfterClient); }
    void prelaunchRestrictedOomChangeBefore()      { prelaunchRestrictedTest(oomChangeBeforeClient); }
    void prelaunchRestrictedOomChangeAfter()       { prelaunchRestrictedTest(oomChangeAfterClient); }

    void pipeLauncherStartAndStop()         { pipeLauncherTest(startAndStopClient); }
    void pipeLauncherStartAndStopMultiple() { pipeLauncherTest(startAndStopMultiple); }
    void pipeLauncherStartAndKill()         { pipeLauncherTest(startAndKillClient); }
    void pipeLauncherStartAndKillTough()    { pipeLauncherTest(startAndKillClient, makeTough); }
    void pipeLauncherStartAndCrash()        { pipeLauncherTest(startAndCrashClient); }
    void pipeLauncherEcho()                 { pipeLauncherTest(echoClient); }
    void pipeLauncherPriorityChangeBefore() { pipeLauncherTest(priorityChangeBeforeClient); }
    void pipeLauncherPriorityChangeAfter()  { pipeLauncherTest(priorityChangeAfterClient); }
    void pipeLauncherOomChangeBefore()      { pipeLauncherTest(oomChangeBeforeClient); }
    void pipeLauncherOomChangeAfter()       { pipeLauncherTest(oomChangeAfterClient); }

    void socketLauncherStartAndStop()         { socketLauncherTest(startAndStopClient); }
    void socketLauncherStartAndStopMultiple() { socketLauncherTest(startAndStopMultiple); }
    void socketLauncherStartAndKill()         { socketLauncherTest(startAndKillClient); }
    void socketLauncherStartAndKillTough()    { socketLauncherTest(startAndKillClient, QStringList(), makeTough); }
    void socketLauncherStartAndCrash()        { socketLauncherTest(startAndCrashClient); }
    void socketLauncherEcho()                 { socketLauncherTest(echoClient); }
    void socketLauncherPriorityChangeBefore() { socketLauncherTest(priorityChangeBeforeClient); }
    void socketLauncherPriorityChangeAfter()  { socketLauncherTest(priorityChangeAfterClient); }
    void socketLauncherOomChangeBefore()      { socketLauncherTest(oomChangeBeforeClient); }
    void socketLauncherOomChangeAfter()       { socketLauncherTest(oomChangeAfterClient); }

    void socketSchemaStartAndStop()         { socketSchemaTest(startAndStopClient); }
    void socketSchemaStartAndStopMultiple() { socketSchemaTest(startAndStopMultiple); }
    void socketSchemaStartAndKill()         { socketSchemaTest(startAndKillClient); }
    void socketSchemaStartAndKillTough()    { socketSchemaTest(startAndKillClient, makeTough); }
    void socketSchemaStartAndCrash()        { socketSchemaTest(startAndCrashClient); }
    void socketSchemaEcho()                 { socketSchemaTest(echoClient); }
    void socketSchemaPriorityChangeBefore() { socketSchemaTest(priorityChangeBeforeClient); }
    void socketSchemaPriorityChangeAfter()  { socketSchemaTest(priorityChangeAfterClient); }
    void socketSchemaOomChangeBefore()      { socketSchemaTest(oomChangeBeforeClient); }
    void socketSchemaOomChangeAfter()       { socketSchemaTest(oomChangeAfterClient); }

    void forkLauncherStartAndStop()         { forkLauncherTest(startAndStopClient); }
    void forkLauncherStartAndStopMultiple() { forkLauncherTest(startAndStopMultiple); }
    void forkLauncherStartAndKill()         { forkLauncherTest(startAndKillClient); }
    void forkLauncherStartAndKillTough()    { forkLauncherTest(startAndKillClient, makeTough); }
    void forkLauncherStartAndCrash()        { forkLauncherTest(startAndCrashClient); }
    void forkLauncherEcho()                 { forkLauncherTest(echoClient); }
    void forkLauncherPriorityChangeBefore() { forkLauncherTest(priorityChangeBeforeClient); }
    void forkLauncherPriorityChangeAfter()  { forkLauncherTest(priorityChangeAfterClient); }
    void forkLauncherOomChangeBefore()      { forkLauncherTest(oomChangeBeforeClient); }
    void forkLauncherOomChangeAfter()       { forkLauncherTest(oomChangeAfterClient); }

    void preforkLauncherStartAndStop()         { preforkLauncherTest(startAndStopClient); }
    void preforkLauncherStartAndStopMultiple() { preforkLauncherTest(startAndStopMultiple); }
    void preforkLauncherStartAndKill()         { preforkLauncherTest(startAndKillClient); }
    void preforkLauncherStartAndKillTough()    { preforkLauncherTest(startAndKillClient, makeTough); }
    void preforkLauncherStartAndCrash()        { preforkLauncherTest(startAndCrashClient); }
    void preforkLauncherEcho()                 { preforkLauncherTest(echoClient); }
    void preforkLauncherPriorityChangeBefore() { preforkLauncherTest(priorityChangeBeforeClient); }
    void preforkLauncherPriorityChangeAfter()  { preforkLauncherTest(priorityChangeAfterClient); }
    void preforkLauncherOomChangeBefore()      { preforkLauncherTest(oomChangeBeforeClient); }
    void preforkLauncherOomChangeAfter()       { preforkLauncherTest(oomChangeAfterClient); }

    void prelaunchChildAbort();
    void prelaunchThreadPriority();
    void prelaunchWaitIdleTest();

    void prelaunchForPipeLauncherIdle();
    void prelaunchForPipeLauncherMemory();
    void prelaunchForPipeLauncherMultiple();

    void prelaunchForSocketLauncherIdle();
    void prelaunchForSocketLauncherMemory();
    void prelaunchForSocketLauncherMultiple();

    void frontend();
    void frontendWaitIdleTest();
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
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    manager->setIdleDelegate(delegate);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // The factory should not have launched
    QCOMPARE(manager->internalProcesses().count(), 0);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);
    Q_PID pid = manager->internalProcesses().at(0);

    // Kill the prelaunched process and verify that it is restarted
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForSignal(spy);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
    delete manager;
}

#include <sys/time.h>
#include <sys/resource.h>

void tst_ProcessManager::prelaunchThreadPriority()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    manager->setIdleDelegate(delegate);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    info.setValue("arguments", QStringList() << "-threads");
    info.setValue("priority", 19);
    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // Start the prelaunch process and verify that all threads
    // have been created and are at the correct priority
    waitForInternalProcess(manager);
    Q_PID pid = manager->internalProcesses().at(0);
    waitForThreadCount(pid, 5);
    foreach (qint32 priority, ProcUtils::getThreadPriorities(pid))
        QCOMPARE(priority, 19);

    info.setValue("priority", 5);
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    verifyRunning(process);
    QCOMPARE(process->actualPriority(), 5);
    foreach (qint32 priority, ProcUtils::getThreadPriorities(process->pid()))
        QCOMPARE(priority, 5);

    writeJson(process, "stop");
    spy.waitFinished();
    cleanupProcess(process);

    delete manager;
}

void tst_ProcessManager::prelaunchWaitIdleTest()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    delegate->setEnabled(false);
    manager->setIdleDelegate(delegate);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // The prelaunch process should not launch, even after a timeout
    QCOMPARE(manager->internalProcesses().count(), 0);
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 0);

    // Now the prelaunch process should launch
    delegate->setEnabled(true);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Kill the prelaunched process and verify that it is restarted
    Q_PID pid = manager->internalProcesses().at(0);
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForSignal(spy);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);

    delete manager;
}

/*
  The pipe launcher holds a prelaunch backend factory
  We control the prelaunch process by turning on and off the IdleDelegate
 */

void tst_ProcessManager::prelaunchForPipeLauncherIdle()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    delegate->setEnabled(false);
    manager->setIdleDelegate(delegate);

    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    info.setValue("arguments",
                  QStringList() << QStringLiteral("-prelaunch")
                  << QStringLiteral("testPrelaunch/testPrelaunch"));
    PipeProcessBackendFactory *factory = new PipeProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // Wait for the factory have launched a pipe
    waitForInternalProcess(manager);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Verify that the prelaunch has not started, even after the interval
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Now the prelaunch process should launch
    delegate->setEnabled(true);
    waitForInternalProcess(manager, 2, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 2);

    // Kill the prelaunched process and verify that it is restarted
    Q_PID pid = manager->internalProcesses().at(1);
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 1);
    waitForSignal(spy);
    waitForInternalProcess(manager, 2, delegate->idleInterval() + 2000);

    // Kill the prelaunched process and keep it dead - otherwise we'll leave a hanging child
    delegate->setEnabled(false);
    pid = manager->internalProcesses().at(1);
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 1);
    waitForSignal(spy, 2);
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    delete manager;
}

/*
  The pipe launcher holds a prelaunch backend factory
  We control the prelaunch process by turning on and off MemoryRestricted
 */

void tst_ProcessManager::prelaunchForPipeLauncherMemory()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    manager->setIdleDelegate(delegate);
    manager->setMemoryRestricted(true);

    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    info.setValue("arguments",
                  QStringList() << QStringLiteral("-prelaunch")
                  << QStringLiteral("testPrelaunch/testPrelaunch"));
    PipeProcessBackendFactory *factory = new PipeProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addFactory(factory);

    // Wait for the factory have launched a pipe
    waitForInternalProcess(manager);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Verify that the prelaunch has not started, even after the interval
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Now the prelaunch process should launch
    manager->setMemoryRestricted(false);
    waitForInternalProcess(manager, 2, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 2);

    // Kill the prelaunched process and verify that it is restarted
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    Q_PID pid = manager->internalProcesses().at(1);
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 1);
    waitForSignal(spy);
    waitForInternalProcess(manager, 2, delegate->idleInterval() + 2000);

    // Kill the prelaunched process and keep it dead - otherwise we'll leave a hanging child
    manager->setMemoryRestricted(true);
    waitForInternalProcess(manager, 1);
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    delete manager;
}

/*
  Each pipe launcher holds a prelaunch backend factory
 */

void tst_ProcessManager::prelaunchForPipeLauncherMultiple()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    manager->setMemoryRestricted(true);
    manager->setIdleDelegate(delegate);

    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    info.setValue("arguments",
                  QStringList() << QStringLiteral("-prelaunch")
                  << QStringLiteral("testPrelaunch/testPrelaunch"));

    for (int i = 0 ; i < kProcessCount ; i++ ) {
        PipeProcessBackendFactory *factory = new PipeProcessBackendFactory;
        factory->setProcessInfo(info);
        manager->addFactory(factory);
    }

    // Wait for the factory have launched a pipe
    qDebug() << "Waiting for" << kProcessCount << "pipe launchers";
    waitForInternalProcess(manager, kProcessCount);

    // Verify that the prelaunch has not started, even after the interval
    qDebug() << "Ensure that nothing prelaunches" << manager->internalProcesses().count() << "of" << kProcessCount;
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), kProcessCount);

    // Now the prelaunch process should launch
    qDebug() << "Turn on the prelaunches" << manager->internalProcesses();
    manager->setMemoryRestricted(false);
    waitForInternalProcess(manager, 2*kProcessCount, delegate->idleInterval() * kProcessCount + 2000);

    // Shut off the prelaunches
    qDebug() << "Shut off the prelaunches" << manager->internalProcesses();
    manager->setMemoryRestricted(true);
    waitForInternalProcess(manager, kProcessCount, delegate->idleInterval() + 2000);

    delete manager;
}


/*
  The socket launcher holds a prelaunch backend factory
  We control the prelaunch process by turning on and off the IdleDelegate
 */

void tst_ProcessManager::prelaunchForSocketLauncherIdle()
{
    QProcess *remote = new QProcess;
    QString socketName = QStringLiteral("/tmp/socketlauncher");
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher",
                  QStringList() << QStringLiteral("-prelaunch") << QStringLiteral("testPrelaunch/testPrelaunch")
                  << socketName);
    QVERIFY(remote->waitForStarted());
    waitForSocket(socketName);

    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    delegate->setEnabled(false);
    manager->setIdleDelegate(delegate);

    SocketProcessBackendFactory *factory = new SocketProcessBackendFactory;
    factory->setSocketName(socketName);
    manager->addFactory(factory);

    // Verify that the prelaunch has not started, even after the interval
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 0);

    // Now the prelaunch process should launch
    delegate->setEnabled(true);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Kill the prelaunched process and verify that it is restarted
    Q_PID pid = manager->internalProcesses().at(0);
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForSignal(spy);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);

    // Kill the prelaunched process and keep it dead - otherwise we'll leave a hanging child
    delegate->setEnabled(false);
    pid = manager->internalProcesses().at(0);
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForSignal(spy, 2);
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 0);

    delete manager;
    delete remote;
}

/*
  The socket launcher holds a prelaunch backend factory
  We control the prelaunch process by turning on and off MemoryRestricted
 */

void tst_ProcessManager::prelaunchForSocketLauncherMemory()
{
    QProcess *remote = new QProcess;
    QString socketName = QStringLiteral("/tmp/socketlauncher");
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher",
                  QStringList() << QStringLiteral("-prelaunch") << QStringLiteral("testPrelaunch/testPrelaunch")
                  << socketName);
    QVERIFY(remote->waitForStarted());
    waitForSocket(socketName);

    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    manager->setIdleDelegate(delegate);
    manager->setMemoryRestricted(true);

    SocketProcessBackendFactory *factory = new SocketProcessBackendFactory;
    factory->setSocketName(socketName);
    manager->addFactory(factory);

    // Verify that the prelaunch has not started, even after the interval
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 0);

    // Now the prelaunch process should launch
    manager->setMemoryRestricted(false);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);

    // Kill the prelaunched process and verify that it is restarted
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    Q_PID pid = manager->internalProcesses().at(0);
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForSignal(spy);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);

    // Kill the prelaunched process and keep it dead - otherwise we'll leave a hanging child
    qDebug() << "Set memory to restricted and wait for the process to die";
    manager->setMemoryRestricted(true);
    waitForInternalProcess(manager, 0);
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 0);

    delete manager;
    delete remote;
}

/*
  Each socket launcher holds a prelaunch backend factory
 */

void tst_ProcessManager::prelaunchForSocketLauncherMultiple()
{
    QProcess *remote = new QProcess;
    QString socketName = QStringLiteral("/tmp/socketlauncher");
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher",
                  QStringList() << QStringLiteral("-prelaunch") << QStringLiteral("testPrelaunch/testPrelaunch")
                  << socketName);
    QVERIFY(remote->waitForStarted());
    waitForSocket(socketName);

    ProcessBackendManager *manager = new ProcessBackendManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    delegate->setIdleInterval(250);
    manager->setIdleDelegate(delegate);
    manager->setMemoryRestricted(true);

    for (int i = 0 ; i < kProcessCount ; i++ ) {
        SocketProcessBackendFactory *factory = new SocketProcessBackendFactory;
        factory->setSocketName(socketName);
        manager->addFactory(factory);
    }

    // Verify that the prelaunch has not started, even after the interval
    waitForTimeout(delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 0);

    // Now the prelaunch process should launch
    manager->setMemoryRestricted(false);
    waitForInternalProcess(manager, kProcessCount, delegate->idleInterval() * kProcessCount + 2000);

    // Shut off the prelaunches
    manager->setMemoryRestricted(true);
    waitForInternalProcess(manager, 0, delegate->idleInterval() + 2000);

    delete manager;
    delete remote;
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

void tst_ProcessManager::frontendWaitIdleTest()
{
    ProcessManager *manager = new ProcessManager;
    TimeoutIdleDelegate *delegate = new TimeoutIdleDelegate;
    manager->setIdleDelegate(delegate);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    PrelaunchProcessBackendFactory *factory = new PrelaunchProcessBackendFactory;
    factory->setProcessInfo(info);
    manager->addBackendFactory(factory);

    // The factory should not have launched
    QCOMPARE(manager->internalProcesses().count(), 0);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
    QCOMPARE(manager->internalProcesses().count(), 1);
    Q_PID pid = manager->internalProcesses().at(0);

    // Kill the prelaunched process and verify that it is restarted
    QSignalSpy spy(manager, SIGNAL(internalProcessError(QProcess::ProcessError)));
    ::kill(pid, SIGKILL);
    waitForInternalProcess(manager, 0);
    waitForSignal(spy);
    waitForInternalProcess(manager, 1, delegate->idleInterval() + 2000);
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

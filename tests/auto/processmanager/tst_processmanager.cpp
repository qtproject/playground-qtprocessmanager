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

QT_USE_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QProcess::ExitStatus);
Q_DECLARE_METATYPE(QProcess::ProcessState);
Q_DECLARE_METATYPE(QProcess::ProcessError);

class tst_ProcessManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void startAndKill();
    void failToStart();
    void startAndStop();
    void startAndCrash();
    void echo();
    void priorityChangeBeforeStart();
    void priorityChangeAfterStart();
    void prelaunch();
    void prelaunchRestricted();
    void pipeLauncher();
    void pipeLauncherCrash();
    void socketLauncher();
    void socketLauncherKill();
    void socketLauncherCrash();
    void frontend();
    void subclassFrontend();
};

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
        qDebug() << "Checking stdout for" << s;
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
            QTestEventLoop::instance().enterLoop(1);
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            if (startSpy.count())
                break;
        }
    }

    void waitFailedStart(int timeout=5000) {
        stopWatch.restart();
        forever {
            QTestEventLoop::instance().enterLoop(1);
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            if (errorSpy.count())
                break;
        }
    }

    void waitFinished(int timeout=5000) {
        stopWatch.restart();
        forever {
            QTestEventLoop::instance().enterLoop(1);
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            if (finishedSpy.count())
                break;
        }
    }

    void waitStdout(int timeout=5000) {
        stopWatch.restart();
        int count = stdoutSpy.count();
        forever {
            QTestEventLoop::instance().enterLoop(1);
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
            if (stdoutSpy.count() != count) {
                break;
            }
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
        for (int i = 0 ; i < errorSpy.count() ; i++) {
            QCOMPARE(errorSpy.at(i), list.at(i));
            qDebug() << "Checking for error:" << list.at(i) << "string=" << errorSpy.atString(i);
        }
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

bool canCheckProcessState()
{
    QFileInfo finfo("/bin/ps");
    return finfo.exists();
}

bool isProcessRunning(Q_PID pid)
{
    QProcess p;
    p.start("ps", QStringList() << "-o" << "pid=" << "-p" << QString::number(pid));
    if (p.waitForStarted() && p.waitForFinished())
            return p.readAll().split('\n').at(0).toDouble() == pid;

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

/*********/

void tst_ProcessManager::initTestCase()
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessError");
}

void tst_ProcessManager::startAndKill()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    process->stop();
    spy.waitFinished();
    spy.check(1, 1, 1, 3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::CrashExit);
    spy.checkErrors(QList<QProcess::ProcessError>() << QProcess::Crashed);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}


void tst_ProcessManager::failToStart()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "thisProgramDoesntExist");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitFailedStart();
    spy.check(0,1,0,2);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}


void tst_ProcessManager::startAndStop()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    process->write("echo\n");

    // Now send a "stop" message
    process->write("stop\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::startAndCrash()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    process->write("crash\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(2);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::echo()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();

    process->write("echotest\n");
    spy.waitStdout();
    spy.checkStdout("echotest\n");

    process->write("stop\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::priorityChangeBeforeStart()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    info.setValue("priority", 19);
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();

    QVERIFY(process->actualPriority() == 19);

    process->write("stop\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::priorityChangeAfterStart()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new StandardProcessBackendFactory);

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();

    process->setDesiredPriority(19);
    QVERIFY(process->actualPriority() == 19);

    process->write("stop\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::prelaunch()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    manager->addFactory(new PrelaunchProcessBackendFactory(info));

    // The factory should not have launched a prelaunch yet
    QVERIFY(manager->memoryRestricted() == false);
    QVERIFY(manager->internalProcesses().count() == 0);

    qDebug() << "Waiting for 2 seconds";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 2000)
        QTestEventLoop::instance().enterLoop(1);

    // Verify that there is a prelaunched process
    QVERIFY(manager->memoryRestricted() == false);
    QVERIFY(manager->internalProcesses().count() == 1);

    info.setValue("prelaunch", "true");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);
    QVERIFY(process->state() == QProcess::NotRunning);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    QVERIFY(process->state() == QProcess::Running);

    QVariantMap map;
    map.insert("command", "stop");
    process->write(QJsonDocument::fromVariant(map).toBinaryData());
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::prelaunchRestricted()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->setMemoryRestricted(true);

    ProcessInfo info;
    info.setValue("program", "testPrelaunch/testPrelaunch");
    manager->addFactory(new PrelaunchProcessBackendFactory(info));

    // The factory should not have launched a prelaunch yet
    QVERIFY(manager->memoryRestricted() == true);
    QVERIFY(manager->internalProcesses().count() == 0);

    qDebug() << "Waiting for 2 seconds";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 2000)
        QTestEventLoop::instance().enterLoop(1);

    // The factory should still not have prelaunched anything
    QVERIFY(manager->memoryRestricted() == true);
    QVERIFY(manager->internalProcesses().count() == 0);

    info.setValue("prelaunch", "true");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    QVariantMap map;
    map.insert("command", "stop");
    process->write(QJsonDocument::fromVariant(map).toBinaryData());
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}

void tst_ProcessManager::pipeLauncher()
{
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    manager->addFactory(new PipeProcessBackendFactory(info, "testClient"));

    qDebug() << "Waiting for 500 ms let pipe start";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 500)
        QTestEventLoop::instance().enterLoop(1);

    // The factory should have launched a pipe by now
    QVERIFY(manager->internalProcesses().count() == 1);

    ProcessInfo info2;
    info2.setValue("program", "testClient/testClient");
    info2.setValue("pipe", "true");
    ProcessBackend *process = manager->create(info2);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();

    qDebug() << "Checking post started";
    spy.check(1,0,0,2);

    qDebug() << "Sending echo command";
    process->write("echo\n");

    qDebug() << "Sending stop command";
    process->write("stop\n");

    qDebug() << "Waiting for finished";
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
}


void tst_ProcessManager::pipeLauncherCrash()
{
    QString socketname = "/tmp/tst_socket";
    ProcessBackendManager *manager = new ProcessBackendManager;
    ProcessInfo info;
    info.setValue("program", "testPipeLauncher/testPipeLauncher");
    manager->addFactory(new PipeProcessBackendFactory(info, "testClient"));

    qDebug() << "Waiting for 500 ms let pipe start";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 500)
        QTestEventLoop::instance().enterLoop(1);

    // The factory should have launched a pipe by now
    QVERIFY(manager->internalProcesses().count() == 1);

    ProcessInfo info2;
    info2.setValue("program", "testClient/testClient");
    info2.setValue("pipe", "true");
    ProcessBackend *process = manager->create(info2);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();

    qDebug() << "Checking post started";
    spy.check(1,0,0,2);

    QList<Q_PID> plist = manager->internalProcesses();
    QCOMPARE(plist.count(), 1);

    if (canCheckProcessState()) {
        qDebug() << "Verifying that all processes are running";
        foreach (Q_PID pid, plist)
            QVERIFY(isProcessRunning(pid));
    }

    qDebug() << "Deleting manager process";
    delete manager;

    qDebug() << "Waiting for 1000 ms to let the pipe stop";
    waitTime.restart();
    while (waitTime.elapsed() < 1000)
        QTestEventLoop::instance().enterLoop(1);

    if (canCheckProcessState()) {
        foreach (Q_PID pid, plist) {
            qDebug() << "Checking process" << pid;
            QVERIFY(isProcessStopped(pid));
        }
    }

    delete process;
    qDebug() << "Deleted process";
}


void tst_ProcessManager::socketLauncher()
{
    QProcess *remote = new QProcess;
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher");
    QVERIFY(remote->waitForStarted());

    qDebug() << "Waiting for 500 ms to let testSocketLauncher start";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 500)
        QTestEventLoop::instance().enterLoop(1);

    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new SocketProcessBackendFactory("/tmp/socketlauncher"));

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();

    qDebug() << "Checking post started";
    spy.check(1,0,0,2);

    qDebug() << "Sending echo command";
    process->write("echo\n");

    qDebug() << "Sending stop command";
    process->write("stop\n");

    qDebug() << "Waiting for finished";
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
    delete remote;
}

void tst_ProcessManager::socketLauncherKill()
{
    QProcess *remote = new QProcess;
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher");
    QVERIFY(remote->waitForStarted());

    qDebug() << "Waiting for 500 ms to let testSocketLauncher start";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 500)
        QTestEventLoop::instance().enterLoop(1);

    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new SocketProcessBackendFactory("/tmp/socketlauncher"));

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);
    process->stop();
    spy.waitFinished();
    spy.check(1,1,1,3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::CrashExit);
    spy.checkErrors(QList<QProcess::ProcessError>() << QProcess::Crashed);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
    delete remote;
}

void tst_ProcessManager::socketLauncherCrash()
{
    QProcess *remote = new QProcess;
    remote->setProcessChannelMode(QProcess::ForwardedChannels);
    remote->start("testSocketLauncher/testSocketLauncher");
    QVERIFY(remote->waitForStarted());

    qDebug() << "Waiting for 500 ms to let testSocketLauncher start";
    QTime waitTime;
    waitTime.start();
    while (waitTime.elapsed() < 500)
        QTestEventLoop::instance().enterLoop(1);

    ProcessBackendManager *manager = new ProcessBackendManager;
    manager->addFactory(new SocketProcessBackendFactory("/tmp/socketlauncher"));

    ProcessInfo info;
    info.setValue("program", "testClient/testClient");
    ProcessBackend *process = manager->create(info);
    QVERIFY(process);

    Spy spy(process);
    process->start();
    spy.waitStart();
    spy.check(1,0,0,2);

    process->write("crash\n");
    spy.waitFinished();
    spy.check(1,0,1,3);
    spy.checkExitCode(2);
    spy.checkExitStatus(QProcess::NormalExit);

    QVERIFY(process->parent() == NULL);
    delete process;
    delete manager;
    delete remote;
}


void tst_ProcessManager::frontend()
{
    QString socketname = "/tmp/tst_socket";
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
    QString socketname = "/tmp/tst_socket";
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

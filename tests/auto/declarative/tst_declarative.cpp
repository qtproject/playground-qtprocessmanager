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
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlProperty>
#include <QQmlContext>
#include <QProcess>
#include <QLocalSocket>

#include <qjsonuidrangeauthority.h>

#include "declarativeprocessmanager.h"
#include "declarativematchdelegate.h"
#include "declarativerewritedelegate.h"
#include "standardprocessbackendfactory.h"
#include "prelaunchprocessbackendfactory.h"
#include "socketprocessbackendfactory.h"
#include "processfrontend.h"
#include "processbackend.h"
#include "process.h"
#include "timeoutidledelegate.h"

QT_USE_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QProcess::ExitStatus);
Q_DECLARE_METATYPE(QProcess::ProcessState);
Q_DECLARE_METATYPE(QProcess::ProcessError);

class tst_DeclarativeProcessManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void basic();
    void prelaunch();
    void matchDelegate();
    void socketLauncher();
    void socketRangeLauncher();

    void match();
    void rewrite();
    void timeoutIdleDelegate();
};


void tst_DeclarativeProcessManager::initTestCase()
{
    const char *uri = "Test";
    DeclarativeProcessManager::registerTypes(uri);

    qRegisterMetaType<QProcess::ProcessState>();
    qRegisterMetaType<QProcess::ExitStatus>();
    qRegisterMetaType<QProcess::ProcessError>();
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

static void waitForChange(QSignalSpy& spy, int count, int timeout=4000)
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


class Spy {
public:
    Spy(ProcessFrontend *process)
        : stateSpy(process, SIGNAL(stateChanged(QProcess::ProcessState)))
        , startSpy(process, SIGNAL(started()))
        , errorSpy(process, SIGNAL(error(QProcess::ProcessError)))
        , finishedSpy(process, SIGNAL(finished(int, QProcess::ExitStatus))) {}

    void check( int startCount, int errorCount, int finishedCount, int stateCount ) {
//        qDebug() << "XXX errors: ";
//        for (int i = 0 ; i < errorSpy.count() ; i++)
//           qDebug() << "\tXXX : " << (qVariantValue<QProcess::ProcessError>(errorSpy.at(i).at(0)));


        QVERIFY(startSpy.count() == startCount);
        QVERIFY(errorSpy.count() == errorCount);
        QVERIFY(finishedSpy.count() == finishedCount);
        QVERIFY(stateSpy.count() == stateCount);
        bool failedToStart = false;
        for (int i = 0 ; i < errorSpy.count() ; i++)
            if (qVariantValue<QProcess::ProcessError>(errorSpy.at(i).at(0)) == QProcess::FailedToStart)
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
            QCOMPARE(qVariantValue<QProcess::ProcessError>(errorSpy.at(i).at(0)), list.at(i));
    }

    QTime      stopWatch;
    QSignalSpy stateSpy;
    QSignalSpy startSpy;
    QSignalSpy errorSpy;
    QSignalSpy finishedSpy;
};

static void _frontendTest(const QString& filename)
{
    QQmlEngine    engine;
    QQmlComponent component(&engine, QUrl::fromLocalFile(filename));
    if (component.isError())
        qWarning() << component.errors();

    DeclarativeProcessManager *manager = qobject_cast<DeclarativeProcessManager*>(component.create());
    QVERIFY(manager != NULL);

    for (int i = 0 ; i < 3 ; i++ ) {
        QVariant name;
        QVERIFY(QMetaObject::invokeMethod(manager, "makeProcess", Q_RETURN_ARG(QVariant, name)));
        ProcessFrontend *frontend = manager->processForName(name.toString());
        QVERIFY(frontend);

        Spy spy(frontend);
        QVERIFY(QMetaObject::invokeMethod(manager, "startProcess", Q_ARG(QVariant, name)));
        spy.waitStart();

        QVERIFY(QMetaObject::invokeMethod(manager, "stopProcess", Q_ARG(QVariant, name)));
        spy.waitFinished();
        spy.check(1, 1, 1, 3);
        spy.checkExitCode(0);
        spy.checkExitStatus(QProcess::CrashExit);
        spy.checkErrors(QList<QProcess::ProcessError>() << QProcess::Crashed);
    }
}

void tst_DeclarativeProcessManager::basic()
{
    _frontendTest("data/testfrontend.qml");
}

void tst_DeclarativeProcessManager::prelaunch()
{
    _frontendTest("data/testprelaunch.qml");
}

void tst_DeclarativeProcessManager::matchDelegate()
{
    _frontendTest("data/testmatch.qml");
}

void tst_DeclarativeProcessManager::socketLauncher()
{
    QProcess remote;
    remote.setProcessChannelMode(QProcess::ForwardedChannels);
    remote.start("testSocketLauncher/testSocketLauncher", QStringList() << "data/testsocket.qml");
    QVERIFY(remote.waitForStarted());
    waitForSocket("/tmp/socket_launcher");

    _frontendTest("data/testsocketfrontend.qml");
}

void tst_DeclarativeProcessManager::socketRangeLauncher()
{
    QProcess remote;
    remote.setProcessChannelMode(QProcess::ForwardedChannels);
    remote.start("testSocketLauncher/testSocketLauncher", QStringList() << "data/testrangesocket.qml");
    QVERIFY(remote.waitForStarted());
    waitForSocket("/tmp/socket_launcher");

    _frontendTest("data/testsocketfrontend.qml");
}

const char *kMatchTest = "import QtQuick 2.0; import Test 1.0; \n"
"PmScriptMatch { \n"
"  script: { \n"
"     if ( model.program == \"goodprogram\" ) return true; \n"
"     if ( model.program == \"badprogram\" ) return false; \n"
"     if ( model.priority > 10 ) return true; \n"
"     if ( model.environment[\"debug\"] ) return true; \n"
"     return false; \n"
"  }\n"
"}";

void tst_DeclarativeProcessManager::match()
{
    QQmlEngine    engine;
    QQmlContext   context(&engine);
    QQmlComponent component(&engine);
    component.setData(kMatchTest, QUrl());
    QCOMPARE(component.status(), QQmlComponent::Ready);
    DeclarativeMatchDelegate *delegate = qobject_cast<DeclarativeMatchDelegate *>(component.create());
    QVERIFY(delegate);

    ProcessInfo info;
    info.setProgram("goodprogram");
    QCOMPARE(delegate->matches(info), true);

    info.setProgram("badprogram");
    QCOMPARE(delegate->matches(info), false);

    info.setProgram("aprogram");
    info.setPriority(20);
    QCOMPARE(delegate->matches(info), true);

    info.setPriority(0);
    QCOMPARE(delegate->matches(info), false);

    QVariantMap env = info.environment();
    env.insert("debug", "true");
    info.setEnvironment(env);
    QCOMPARE(delegate->matches(info), true);
}

const char *kRewriteTest = "import QtQuick 2.0; import Test 1.0; \n"
"PmScriptRewrite { \n"
"  script: { \n"
"     model.program = \"foo-\"+model.program; \n"
"     var oldargs = model.arguments; \n"
"     oldargs.unshift(\"puppy\"); \n"
"     model.arguments = oldargs; \n"
"     var env = model.environment; \n"
"     env[\"friendly\"] = \"yes\"; \n"
"     env[\"noiselevel\"] = 85; \n"
"     model.environment = env; \n"
"  }\n"
"}\n";

void tst_DeclarativeProcessManager::rewrite()
{
    QQmlEngine    engine;
    QQmlContext   context(&engine);
    QQmlComponent component(&engine);
    component.setData(kRewriteTest, QUrl());
    QCOMPARE(component.status(), QQmlComponent::Ready);
    DeclarativeRewriteDelegate *delegate = qobject_cast<DeclarativeRewriteDelegate *>(component.create());
    QVERIFY(delegate);

    ProcessInfo info;
    info.setProgram("goodprogram");
    info.setArguments(QStringList() << "cat");
    QVariantMap env;
    env.insert(QStringLiteral("gdb"), QStringLiteral("false"));
    env.insert(QStringLiteral("fed"), QStringLiteral("true"));
    env.insert(QStringLiteral("IQ"), 120);
    info.setEnvironment(env);

    delegate->rewrite(info);
    QCOMPARE(info.program(), QStringLiteral("foo-goodprogram"));
    QStringList args = info.arguments();
    QCOMPARE(args.size(), 2);
    QCOMPARE(args.at(0), QStringLiteral("puppy"));
    QCOMPARE(args.at(1), QStringLiteral("cat"));
    QVERIFY(info.environment().contains(QStringLiteral("gdb")));
    QVERIFY(info.environment().contains(QStringLiteral("friendly")));
    QCOMPARE(info.environment().value(QStringLiteral("friendly")).toString(), QStringLiteral("yes"));
    QVERIFY(info.environment().contains(QStringLiteral("noiselevel")));
    QCOMPARE(info.environment().value(QStringLiteral("noiselevel")).toInt(), 85);
}


const char *kTimeoutIdleDelegateTest = "import QtQuick 2.0; import Test 1.0; \n"
"TimeoutIdleDelegate { \n"
"     id: idled\n"
"     idleInterval: 1000\n"
"     enabled: false\n"
"}\n";

void tst_DeclarativeProcessManager::timeoutIdleDelegate()
{
    QQmlEngine    engine;
    QQmlContext   context(&engine);
    QQmlComponent component(&engine);
    component.setData(kTimeoutIdleDelegateTest, QUrl());
    QCOMPARE(component.status(), QQmlComponent::Ready);
    TimeoutIdleDelegate *delegate = qobject_cast<TimeoutIdleDelegate *>(component.create());
    QVERIFY(delegate);

    QSignalSpy spyIdleInterval(delegate, SIGNAL(idleIntervalChanged()));
    QSignalSpy spyIdleCpu(delegate, SIGNAL(idleCpuAvailable()));
    QSignalSpy spyEnabled(delegate, SIGNAL(enabledChanged()));

    waitForTimeout(2000);
    QCOMPARE(spyIdleCpu.count(), 0);

    delegate->requestIdleCpu(true);
    waitForTimeout(2000);
    QCOMPARE(spyIdleCpu.count(), 0);

    delegate->setEnabled(true);
    waitForChange(spyIdleCpu, 1);
    waitForChange(spyIdleCpu, 2);

    delete delegate;
}


QTEST_MAIN(tst_DeclarativeProcessManager)

#include "tst_declarative.moc"

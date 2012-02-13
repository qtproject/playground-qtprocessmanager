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

#include "declarativeprocessmanager.h"
#include "standardprocessbackendfactory.h"
#include "processfrontend.h"
#include "processbackend.h"
#include "process.h"

QT_USE_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QProcess::ExitStatus);
Q_DECLARE_METATYPE(QProcess::ProcessState);
Q_DECLARE_METATYPE(QProcess::ProcessError);

// QML_DECLARE_TYPE(ProcessBackendFactory)
// QML_DECLARE_TYPE(StandardProcessBackendFactory)

class tst_DeclarativeProcessManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void basic();
};


void tst_DeclarativeProcessManager::initTestCase()
{
    qDebug() << "Registering types";
    const char *uri = "Test";
    qmlRegisterType<ProcessBackendFactory>();
    qmlRegisterType<StandardProcessBackendFactory>(uri, 1, 0, "StandardProcessBackendFactory");
    qmlRegisterType<ProcessFrontend>();
    qmlRegisterType<DeclarativeProcessManager>(uri, 1, 0, "DeclarativeProcessManager");
    qmlRegisterUncreatableType<Process>(uri, 1, 0, "Process", "Don't try to make this");

    qRegisterMetaType<QProcess::ProcessState>();
    qRegisterMetaType<QProcess::ExitStatus>();
    qRegisterMetaType<QProcess::ProcessError>();

    qRegisterMetaType<ProcessFrontend*>("ProcessFrontend*");
    qRegisterMetaType<const ProcessFrontend*>("const ProcessFrontend*");
    qRegisterMetaType<ProcessBackend*>("ProcessBackend*");
    qRegisterMetaType<ProcessInfo*>("ProcessInfo*");
}


class Spy {
public:
    Spy(ProcessFrontend *process)
        : stateSpy(process, SIGNAL(stateChanged(QProcess::ProcessState)))
        , startSpy(process, SIGNAL(started()))
        , errorSpy(process, SIGNAL(error(QProcess::ProcessError)))
        , finishedSpy(process, SIGNAL(finished(int, QProcess::ExitStatus))) {}

    void check( int startCount, int errorCount, int finishedCount, int stateCount ) {
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
            QTestEventLoop::instance().enterLoop(1);
            if (stopWatch.elapsed() >= timeout)
                QFAIL("Timed out");
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

void tst_DeclarativeProcessManager::basic()
{
    QDeclarativeEngine    engine;
    QDeclarativeComponent component(&engine, QUrl::fromLocalFile("data/testfrontend.qml"));
    if (component.isError())
        qWarning() << component.errors();
    DeclarativeProcessManager *manager = qobject_cast<DeclarativeProcessManager*>(component.create());
    QVERIFY(manager != NULL);

    QVariant name;
    QVERIFY(QMetaObject::invokeMethod(manager, "makeProcess", Q_RETURN_ARG(QVariant, name)));
    ProcessFrontend *frontend = manager->processForName(name.toString());
    QVERIFY(frontend);
    qDebug() << "Internal frontend object" << frontend;

    Spy spy(frontend);
    QVERIFY(QMetaObject::invokeMethod(manager, "startProcess", Q_ARG(QVariant, name)));
    spy.waitStart();
    return;

    QVERIFY(QMetaObject::invokeMethod(manager, "stopProcess", Q_ARG(QVariant, name)));
    spy.waitFinished();
    spy.check(1, 1, 1, 3);
    spy.checkExitCode(0);
    spy.checkExitStatus(QProcess::CrashExit);
    spy.checkErrors(QList<QProcess::ProcessError>() << QProcess::Crashed);
}


QTEST_MAIN(tst_DeclarativeProcessManager)

#include "tst_declarative.moc"

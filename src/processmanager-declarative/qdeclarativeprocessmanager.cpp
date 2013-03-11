/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarativeprocessmanager.h"
#include "qprocessbackendfactory.h"
#include "qprocessfrontend.h"

#include "qprelaunchprocessbackend.h"
#include "qremoteprocessbackend.h"
#include "qstandardprocessbackend.h"

#include "qcpuidledelegate.h"
#include "qtimeoutidledelegate.h"
#include "qgdbrewritedelegate.h"
#include "qinfomatchdelegate.h"
#include "qkeymatchdelegate.h"
#include "qpipelauncher.h"
#include "qpipeprocessbackendfactory.h"
#include "qpreforkprocessbackendfactory.h"
#include "qprelaunchprocessbackendfactory.h"
#include "qsocketlauncher.h"
#include "qstandardprocessbackendfactory.h"
#include "qsocketprocessbackendfactory.h"

#include "qdeclarativematchdelegate.h"
#include "qdeclarativesocketlauncher.h"
#include "qdeclarativerewritedelegate.h"

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass PmManager QDeclarativeProcessManager
  \brief The PmManager class encapsulates ways of creating and tracking processes
         suitable for QtQml programs.

  Only a single PmManager object should be loaded at one time.

  Typical use of the PmManager class is as follows:

  \code
  import QtQuick 2.0
  import ProcessManager 1.0

  PmManager {
     id: myProcessManager

     factories: [
        GdbProcessBackendFactory {},
        StandardProcessBackendFactory {}
     ]
  }
  \endcode
*/

/*!
  \qmlproperty list<ProcessBackendFactory> PmManager::factories
  List of ProcessBackendFactory objects.

  The order of the list is important.  When launching a new
  process, each factory will be consulted in order to select
  the factory that will launch the process.
 */

/*!
  \class QDeclarativeProcessManager
  \brief The QDeclarativeProcessManager class is a ProcessManager designed to be embedded
         in a QML context.
 */

/*!
  \fn QDeclarativeProcessManager::registerTypes(const char *uri)
  \brief Register all QML data types for the process manager

  Register all types with the QML object system.  Pass the name the
  library to register as \a uri.
 */

void QDeclarativeProcessManager::registerTypes(const char *uri)
{
    // Non-creatable types
    qmlRegisterType<QIdleDelegate>();
    qmlRegisterType<QMatchDelegate>();
    qmlRegisterType<QPrelaunchProcessBackend>();
    qmlRegisterType<QProcessBackend>();
    qmlRegisterType<QProcessFrontend>();
    qmlRegisterType<QProcessBackendFactory>();
    qmlRegisterType<QRemoteProcessBackend>();
    qmlRegisterType<QRemoteProcessBackendFactory>();
    qmlRegisterType<QRewriteDelegate>();
    qmlRegisterType<QStandardProcessBackend>();
    qmlRegisterType<QUnixProcessBackend>();

    qRegisterMetaType<QProcessFrontend*>("QProcessFrontend*");
    qRegisterMetaType<QProcessInfo*>("QProcessInfo*");
    qRegisterMetaType<QMatchDelegate*>("QMatchDelegate*");
    qRegisterMetaType<QRewriteDelegate*>("QRewriteDelegate*");

    // Non-creatable, with enum values
    qmlRegisterUncreatableType<QPmProcess>(uri, 1, 0, "QPmProcess", QStringLiteral("Don't try to make this"));

    // Types registered from the Core library
    qmlRegisterType<QCpuIdleDelegate>(uri, 1, 0, "CpuIdleDelegate");
    qmlRegisterType<QGdbRewriteDelegate>(uri, 1, 0, "GdbRewriteDelegate");
    qmlRegisterType<QInfoMatchDelegate>(uri, 1, 0, "InfoMatchDelegate");
    qmlRegisterType<QKeyMatchDelegate>(uri, 1, 0, "KeyMatchDelegate");
    qmlRegisterType<QPipeLauncher>(uri, 1, 0, "PipeLauncher");
    qmlRegisterType<QPipeProcessBackendFactory>(uri, 1, 0, "PipeProcessBackendFactory");
    qmlRegisterType<QPreforkProcessBackendFactory>(uri, 1, 0, "PreforkProcessBackendFactory");
    qmlRegisterType<QPrelaunchProcessBackendFactory>(uri, 1, 0, "PrelaunchProcessBackendFactory");
    qmlRegisterType<QProcessBackendManager>(uri, 1, 0, "ProcessBackendManager");
    qmlRegisterType<QProcessInfo>(uri, 1, 0, "ProcessInfo");
    qmlRegisterType<QProcessManager>(uri, 1, 0, "ProcessManager");
    qmlRegisterType<QSocketLauncher>(uri, 1, 0, "SocketLauncher");
    qmlRegisterType<QSocketProcessBackendFactory>(uri, 1, 0, "SocketProcessBackendFactory");
    qmlRegisterType<QStandardProcessBackendFactory>(uri, 1, 0, "StandardProcessBackendFactory");
    qmlRegisterType<QTimeoutIdleDelegate>(uri, 1, 0, "TimeoutIdleDelegate");

    // Types registered from the Declarative library
    qmlRegisterType<QDeclarativeMatchDelegate>(uri, 1, 0, "PmScriptMatch");
    qmlRegisterType<QDeclarativeProcessManager>(uri, 1, 0, "PmManager");
    qmlRegisterType<QDeclarativeSocketLauncher>(uri, 1, 0, "PmLauncher");
    qmlRegisterType<QDeclarativeRewriteDelegate>(uri, 1, 0, "PmScriptRewrite");
}

/*!
  Construct a QDeclarativeProcessManager with an optional \a parent
*/

QDeclarativeProcessManager::QDeclarativeProcessManager(QObject *parent)
  : QProcessManager(parent)
{
}

/*!
    \internal
*/
void QDeclarativeProcessManager::classBegin()
{
}

/*!
    \internal
*/
void QDeclarativeProcessManager::componentComplete()
{
}

/*!
  \internal
*/
void QDeclarativeProcessManager::append_factory(QQmlListProperty<QProcessBackendFactory> *list,
                                               QProcessBackendFactory *factory)
{
    QDeclarativeProcessManager *manager = static_cast<QDeclarativeProcessManager *>(list->object);
    if (factory && manager)
        manager->addBackendFactory(factory);
}

/*!
    \internal
 */

QQmlListProperty<QProcessBackendFactory> QDeclarativeProcessManager::factories()
{
    return QQmlListProperty<QProcessBackendFactory>(this, NULL, append_factory, 0, 0, 0);
}

/*!
  Raise the processAboutToStart() signal.
*/

void QDeclarativeProcessManager::processFrontendAboutToStart()
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processAboutToStart(frontend->name());
    QProcessManager::processFrontendAboutToStart();
}

/*!
  Raise the processAboutToStop() signal.
*/

void QDeclarativeProcessManager::processFrontendAboutToStop()
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processAboutToStop(frontend->name());
    QProcessManager::processFrontendAboutToStop();
}

/*!
  Raise the processStarted() signal.
*/

void QDeclarativeProcessManager::processFrontendStarted()
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processStarted(frontend->name());
    QProcessManager::processFrontendStarted();
}

/*!
  Raise the processError() signal.
  Pass through the \a error value.
*/

void QDeclarativeProcessManager::processFrontendError(QProcess::ProcessError error)
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processError(frontend->name(), static_cast<int>(error));
    QProcessManager::processFrontendError(error);
}

/*!
  Raise the processFinished() signal.  Pass through the
  \a exitCode and \a exitStatus values.
*/

void QDeclarativeProcessManager::processFrontendFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processFinished(frontend->name(), exitCode, exitStatus);
    QProcessManager::processFrontendFinished(exitCode, exitStatus);
}

/*!
  Raise the processStateChanged() signal.  Pass through the
  \a state value.
*/

void QDeclarativeProcessManager::processFrontendStateChanged(QProcess::ProcessState state)
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processStateChanged(frontend->name(), state);
    QProcessManager::processFrontendStateChanged(state);
}

/*!
  Raise the processFrontendDestroyed() signal.
*/

void QDeclarativeProcessManager::processFrontendDestroyed()
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        emit processDestroyed(frontend->name());
    QProcessManager::processFrontendDestroyed();
}

/*!
    \fn void QDeclarativeProcessManager::processAboutToStart(const QString& name)
    This signal is emitted when a process is about to start.
    The \a name may be used to retrieve the ProcessFrontend
    object.

    \sa processForName()
*/

/*!
    \fn void QDeclarativeProcessManager::processAboutToStop(const QString& name)
    This signal is emitted when a process is about to stop
    The \a name may be used to retrieve the ProcessFrontend
    object.

    \sa processForName()
*/

/*!
    \fn void QDeclarativeProcessManager::processStarted(const QString& name)
    This signal is emitted once a process has started.
    The \a name may be used to retrieve the ProcessFrontend
    object.

    \sa processForName()
*/

/*!
    \fn void QDeclarativeProcessManager::processError(const QString& name, int error)
    This signal is emitted when a process experiences an \a error.
    The \a name may be used to retrieve the ProcessFrontend
    object.  The \a error value can be compared to the QProcess::ProcessError
    enumeration (it has been cast to an integer to resolve a QML issue).

    \sa processForName()
*/

/*!
    \fn void QDeclarativeProcessManager::processFinished(const QString& name, int exitCode, int exitStatus)
    This signal is emitted when a process finishes. The \a exitCode
    and \a exitStatus match the QProcess values.
    The \a name may be used to retrieve the ProcessFrontend
    object.  The \a exitStatus value can be compared with a QProcess::ExitStatus
    enumeration (it has been cast to an integer to resolve a QML issue).

    \sa processForName()
*/

/*!
    \fn void QDeclarativeProcessManager::processStateChanged(const QString& name, int state)
    This signal is emitted when a process has a state change to \a state.
    The \a name may be used to retrieve the ProcessFrontend
    object.  The \a state value can be compared with QProcess::ProcessState values
    (it has been cast to an integer to resolve a QML issue).

    \sa processForName()
*/

/*!
    \fn void QDeclarativeProcessManager::processDestroyed(const QString &name)
    This signal is emitted when a process has been destroyed
    The \a name cannot be used to retrieve the ProcessFrontend
    object because it no longer exists.
*/



#include "moc_qdeclarativeprocessmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

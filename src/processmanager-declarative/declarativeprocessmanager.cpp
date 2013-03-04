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

#include "declarativeprocessmanager.h"
#include "processbackendfactory.h"
#include "processfrontend.h"

#include "prelaunchprocessbackend.h"
#include "remoteprocessbackend.h"
#include "standardprocessbackend.h"

#include "cpuidledelegate.h"
#include "timeoutidledelegate.h"
#include "gdbrewritedelegate.h"
#include "infomatchdelegate.h"
#include "keymatchdelegate.h"
#include "pipelauncher.h"
#include "pipeprocessbackendfactory.h"
#include "preforkprocessbackendfactory.h"
#include "prelaunchprocessbackendfactory.h"
#include "socketlauncher.h"
#include "standardprocessbackendfactory.h"
#include "socketprocessbackendfactory.h"

#include "declarativematchdelegate.h"
#include "declarativesocketlauncher.h"
#include "declarativerewritedelegate.h"

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass PmManager DeclarativeProcessManager
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
  \class DeclarativeProcessManager
  \brief The DeclarativeProcessManager class is a ProcessManager designed to be embedded
         in a QML context.
 */

/*!
  \fn DeclarativeProcessManager::registerTypes(const char *uri)
  \brief Register all QML data types for the process manager

  Register all types with the QML object system.  Pass the name the
  library to register as \a uri.
 */

void DeclarativeProcessManager::registerTypes(const char *uri)
{
    // Non-creatable types
    qmlRegisterType<IdleDelegate>();
    qmlRegisterType<MatchDelegate>();
    qmlRegisterType<PrelaunchProcessBackend>();
    qmlRegisterType<ProcessBackend>();
    qmlRegisterType<ProcessFrontend>();
    qmlRegisterType<ProcessBackendFactory>();
    qmlRegisterType<RemoteProcessBackend>();
    qmlRegisterType<RemoteProcessBackendFactory>();
    qmlRegisterType<RewriteDelegate>();
    qmlRegisterType<StandardProcessBackend>();
    qmlRegisterType<UnixProcessBackend>();

    qRegisterMetaType<ProcessFrontend*>("ProcessFrontend*");
    qRegisterMetaType<ProcessInfo*>("ProcessInfo*");
    qRegisterMetaType<MatchDelegate*>("MatchDelegate*");
    qRegisterMetaType<RewriteDelegate*>("RewriteDelegate*");

    // Non-creatable, with enum values
    qmlRegisterUncreatableType<Process>(uri, 1, 0, "Process", QStringLiteral("Don't try to make this"));

    // Types registered from the Core library
    qmlRegisterType<CpuIdleDelegate>(uri, 1, 0, "CpuIdleDelegate");
    qmlRegisterType<GdbRewriteDelegate>(uri, 1, 0, "GdbRewriteDelegate");
    qmlRegisterType<InfoMatchDelegate>(uri, 1, 0, "InfoMatchDelegate");
    qmlRegisterType<KeyMatchDelegate>(uri, 1, 0, "KeyMatchDelegate");
    qmlRegisterType<PipeLauncher>(uri, 1, 0, "PipeLauncher");
    qmlRegisterType<PipeProcessBackendFactory>(uri, 1, 0, "PipeProcessBackendFactory");
    qmlRegisterType<PreforkProcessBackendFactory>(uri, 1, 0, "PreforkProcessBackendFactory");
    qmlRegisterType<PrelaunchProcessBackendFactory>(uri, 1, 0, "PrelaunchProcessBackendFactory");
    qmlRegisterType<ProcessBackendManager>(uri, 1, 0, "ProcessBackendManager");
    qmlRegisterType<ProcessInfo>(uri, 1, 0, "ProcessInfo");
    qmlRegisterType<ProcessManager>(uri, 1, 0, "ProcessManager");
    qmlRegisterType<SocketLauncher>(uri, 1, 0, "SocketLauncher");
    qmlRegisterType<SocketProcessBackendFactory>(uri, 1, 0, "SocketProcessBackendFactory");
    qmlRegisterType<StandardProcessBackendFactory>(uri, 1, 0, "StandardProcessBackendFactory");
    qmlRegisterType<TimeoutIdleDelegate>(uri, 1, 0, "TimeoutIdleDelegate");

    // Types registered from the Declarative library
    qmlRegisterType<DeclarativeMatchDelegate>(uri, 1, 0, "PmScriptMatch");
    qmlRegisterType<DeclarativeProcessManager>(uri, 1, 0, "PmManager");
    qmlRegisterType<DeclarativeSocketLauncher>(uri, 1, 0, "PmLauncher");
    qmlRegisterType<DeclarativeRewriteDelegate>(uri, 1, 0, "PmScriptRewrite");
}

/*!
  Construct a DeclarativeProcessManager with an optional \a parent
*/

DeclarativeProcessManager::DeclarativeProcessManager(QObject *parent)
  : ProcessManager(parent)
{
}

/*!
    \internal
*/
void DeclarativeProcessManager::classBegin()
{
}

/*!
    \internal
*/
void DeclarativeProcessManager::componentComplete()
{
}

/*!
  \internal
*/
void DeclarativeProcessManager::append_factory(QQmlListProperty<ProcessBackendFactory> *list,
                                               ProcessBackendFactory *factory)
{
    DeclarativeProcessManager *manager = static_cast<DeclarativeProcessManager *>(list->object);
    if (factory && manager)
        manager->addBackendFactory(factory);
}

/*!
    \internal
 */

QQmlListProperty<ProcessBackendFactory> DeclarativeProcessManager::factories()
{
    return QQmlListProperty<ProcessBackendFactory>(this, NULL, append_factory, 0, 0, 0);
}

/*!
  Raise the processAboutToStart() signal.
*/

void DeclarativeProcessManager::processFrontendAboutToStart()
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processAboutToStart(frontend->name());
    ProcessManager::processFrontendAboutToStart();
}

/*!
  Raise the processAboutToStop() signal.
*/

void DeclarativeProcessManager::processFrontendAboutToStop()
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processAboutToStop(frontend->name());
    ProcessManager::processFrontendAboutToStop();
}

/*!
  Raise the processStarted() signal.
*/

void DeclarativeProcessManager::processFrontendStarted()
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processStarted(frontend->name());
    ProcessManager::processFrontendStarted();
}

/*!
  Raise the processError() signal.
  Pass through the \a error value.
*/

void DeclarativeProcessManager::processFrontendError(QProcess::ProcessError error)
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processError(frontend->name(), static_cast<int>(error));
    ProcessManager::processFrontendError(error);
}

/*!
  Raise the processFinished() signal.  Pass through the
  \a exitCode and \a exitStatus values.
*/

void DeclarativeProcessManager::processFrontendFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processFinished(frontend->name(), exitCode, exitStatus);
    ProcessManager::processFrontendFinished(exitCode, exitStatus);
}

/*!
  Raise the processStateChanged() signal.  Pass through the
  \a state value.
*/

void DeclarativeProcessManager::processFrontendStateChanged(QProcess::ProcessState state)
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processStateChanged(frontend->name(), state);
    ProcessManager::processFrontendStateChanged(state);
}

/*!
  Raise the processFrontendDestroyed() signal.
*/

void DeclarativeProcessManager::processFrontendDestroyed()
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        emit processDestroyed(frontend->name());
    ProcessManager::processFrontendDestroyed();
}

/*!
    \fn void DeclarativeProcessManager::processAboutToStart(const QString& name)
    This signal is emitted when a process is about to start.
    The \a name may be used to retrieve the ProcessFrontend
    object.

    \sa processForName()
*/

/*!
    \fn void DeclarativeProcessManager::processAboutToStop(const QString& name)
    This signal is emitted when a process is about to stop
    The \a name may be used to retrieve the ProcessFrontend
    object.

    \sa processForName()
*/

/*!
    \fn void DeclarativeProcessManager::processStarted(const QString& name)
    This signal is emitted once a process has started.
    The \a name may be used to retrieve the ProcessFrontend
    object.

    \sa processForName()
*/

/*!
    \fn void DeclarativeProcessManager::processError(const QString& name, int error)
    This signal is emitted when a process experiences an \a error.
    The \a name may be used to retrieve the ProcessFrontend
    object.  The \a error value can be compared to the QProcess::ProcessError
    enumeration (it has been cast to an integer to resolve a QML issue).

    \sa processForName()
*/

/*!
    \fn void DeclarativeProcessManager::processFinished(const QString& name, int exitCode, int exitStatus)
    This signal is emitted when a process finishes. The \a exitCode
    and \a exitStatus match the QProcess values.
    The \a name may be used to retrieve the ProcessFrontend
    object.  The \a exitStatus value can be compared with a QProcess::ExitStatus
    enumeration (it has been cast to an integer to resolve a QML issue).

    \sa processForName()
*/

/*!
    \fn void DeclarativeProcessManager::processStateChanged(const QString& name, int state)
    This signal is emitted when a process has a state change to \a state.
    The \a name may be used to retrieve the ProcessFrontend
    object.  The \a state value can be compared with QProcess::ProcessState values
    (it has been cast to an integer to resolve a QML issue).

    \sa processForName()
*/

/*!
    \fn void DeclarativeProcessManager::processDestroyed(const QString &name)
    This signal is emitted when a process has been destroyed
    The \a name cannot be used to retrieve the ProcessFrontend
    object because it no longer exists.
*/



#include "moc_declarativeprocessmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

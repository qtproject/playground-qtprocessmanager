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

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass ProcessManager DeclarativeProcessManager
  \brief The ProcessManager class encapsulates ways of creating and tracking processes.

  Only a single ProcessManager class should be loaded at one time.

  Typical use of the ProcessManager class is as follows:

  \code
  import QtQuick 2.0
  import ProcessManager 1.0

  ProcessManager {
     id: myProcessManager

     factories: [
        GdbProcessBackendFactory {},
        StandardProcessBackendFactory {}
     ]
  }
  \endcode

*/

/*!
  \qmlproperty list<ProcessBackendFactory> ProcessManager::factories
  \brief The factories assigned to this process manager

   The factories property is an ordered list of ProcessBackendFactory objects.
*/


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
void append_factory(QDeclarativeListProperty<ProcessBackendFactory> *list,
                    ProcessBackendFactory *value)
{
    DeclarativeProcessManager *manager = static_cast<DeclarativeProcessManager *>(list->object);
    ProcessBackendFactory *factory = qobject_cast<ProcessBackendFactory *>(value);
    if (factory)
        manager->addBackendFactory(factory);
}

/*!
    \internal
 */

QDeclarativeListProperty<ProcessBackendFactory> DeclarativeProcessManager::factories()
{
    return QDeclarativeListProperty<ProcessBackendFactory>(this, NULL, append_factory);
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

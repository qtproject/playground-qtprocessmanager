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

#include "remoteprocessbackendfactory.h"
#include "remoteprocessbackend.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kRemoteTimerInterval = 1000;

/*!
  \class RemoteProcessBackendFactory
  \brief The RemoteProcessBackendFactory class creates RemoteProcessBackend objects

  The RemoteProcessBackendFactory communicates with a remote process to control
  other, child processes.
  The factory communicates with the remote process by sending and receiving
  JSON-formated messages.  The remote process should respond to the following
  commands:

  \table
  \header
    \o Command
    \o Action
  \row
    \o \c{{ "command": "start", "id": NUM, "info": PROCESSINFO }}
    \o Start a new process running.  The \bold{token} attribute will
       be used to refer to this process in the future.
  \row
    \o \c{{ "command": "stop", "id": NUM, "timeout": NUM }}
    \o Stop the remote process with an optional timeout.
       The remote process should be sent a SIGTERM, followed by
       a SIGKILL after 'timeout' milliseconds
  \row
    \o \c{{ "command": "set", "id": NUM, "key": STRING, "value": NUM }}
    \o Set a process key/value pair.  Currently the \bold{key} can be
       either "priority" or "oomAdjustment".
  \row
    \o \c{{ "command": "write", "id": NUM, "data": STRING }}
    \o Write a data string to the remote process.  We assume that the
       data string is a valid local 8 bit string.
  \endtable

  The following are events that are sent by the remote process
  to the RemoteProcessBackendFactory:

  \table
  \header
    \o Event
    \o Description
  \row
    \o \c{{ "event": "started", "id": NUM, "pid": NUM }}
    \o This process has started.  This maps to the QProcess::started()
       signal, but also includes the PID of the new process.  This should
       be the first event returned after a \bold{start} command
  \row
    \o \c{{ "event": "finished", "id": NUM, "exitCode": NUM, "exitStatus": NUM }}
    \o The process has exited.  This is the last event returned for a given
       process number.
  \row
    \o \c{{ "event": "error", "id": NUM, "error": NUM }}
    \o The process has an error that maps to QProcess::ProcessError
  \row
    \o \c{{ "event": "stateChanged", "id": NUM, "state": NUM }}
    \o The process has an event that maps to a QProcess::ProcessState change.
  \row
    \o \c{{ "event": "output", "id": NUM, "stdout": STRING, "stderr": STRING }}
    \o The process has written data to stdout and/or stderr.
  \endtable
*/

/*!
  Construct a RemoteProcessBackendFactory with optional \a parent.
*/

RemoteProcessBackendFactory::RemoteProcessBackendFactory(QObject *parent)
    : ProcessBackendFactory(parent)
    , m_idCount(100)
{
}

/*!
   Destroy this and child objects.
*/

RemoteProcessBackendFactory::~RemoteProcessBackendFactory()
{
    foreach (RemoteProcessBackend *backend, m_backendMap)
        backend->factoryDestroyed();
}

/*!
  Construct a RemoteProcessBackend from a ProcessInfo \a info record with \a parent.
*/

ProcessBackend * RemoteProcessBackendFactory::create(const ProcessInfo& info, QObject *parent)
{
    int id = m_idCount++;
    RemoteProcessBackend *backend = new RemoteProcessBackend(info, this, id, parent);
    m_backendMap.insert(id, backend);
    return backend;
}

/*!
  Receive a remote \a message and dispatch it to the correct recipient.
  Call this function from your subclass to properly dispatch messages.
 */

void RemoteProcessBackendFactory::receive(const QJsonObject& message)
{
    int id = message.value(QLatin1String("id")).toDouble();
    if (m_backendMap.contains(id))
        m_backendMap.value(id)->receive(message);
}

/*!
  \fn bool RemoteProcessBackendFactory::send(const QJsonObject& message)

  Send a \a message to the remote process.  This method must be implemented in a
  child process.  Return true if the message can be sent.
 */

/*!
  \internal
 */

void RemoteProcessBackendFactory::backendDestroyed(int id)
{
    if (m_backendMap.remove(id) != 1)
        qCritical("Missing remote process backend");
}

#include "moc_remoteprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

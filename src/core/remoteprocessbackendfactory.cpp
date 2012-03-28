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
#include "remoteprotocol.h"

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
    \li Command
    \li Action
  \row
    \li \c{{ "command": "start", "id": NUM, "info": PROCESSINFO }}
    \li Start a new process running.  The \b{token} attribute will
       be used to refer to this process in the future.
  \row
    \li \c{{ "command": "stop", "id": NUM, "timeout": NUM }}
    \li Stop the remote process with an optional timeout.
       The remote process should be sent a SIGTERM, followed by
       a SIGKILL after 'timeout' milliseconds
  \row
    \li \c{{ "command": "set", "id": NUM, "key": STRING, "value": NUM }}
    \li Set a process key/value pair.  Currently the \b{key} can be
       either "priority" or "oomAdjustment".
  \row
    \li \c{{ "command": "write", "id": NUM, "data": STRING }}
    \li Write a data string to the remote process.  We assume that the
       data string is a valid local 8 bit string.
  \row
    \li \c{{ "command": "memory", "restricted": bool }}
    \li Let the remote process know if memory use is restricted.
  \endtable

  The following are events that are sent by the remote process
  to the RemoteProcessBackendFactory:

  \table
  \header
    \li Event
    \li Description
  \row
    \li \c{{ "event": "started", "id": NUM, "pid": NUM }}
    \li This process has started.  This maps to the QProcess::started()
       signal, but also includes the PID of the new process.  This should
       be the first event returned after a \b{start} command
  \row
    \li \c{{ "event": "finished", "id": NUM, "exitCode": NUM, "exitStatus": NUM }}
    \li The process has exited.  This is the last event returned for a given
       process number.
  \row
    \li \c{{ "event": "error", "id": NUM, "error": NUM }}
    \li The process has an error that maps to QProcess::ProcessError
  \row
    \li \c{{ "event": "stateChanged", "id": NUM, "state": NUM }}
    \li The process has an event that maps to a QProcess::ProcessState change.
  \row
    \li \c{{ "event": "output", "id": NUM, "stdout": STRING, "stderr": STRING }}
    \li The process has written data to stdout and/or stderr.
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
  Idle CPU is available.  Send the message over the wire
 */

void RemoteProcessBackendFactory::idleCpuAvailable()
{
    QJsonObject object;
    object.insert(RemoteProtocol::remote(), RemoteProtocol::idlecpuavailable());
    send(object);
}

/*!
  Tell the remote factory that memory is restricted and space should be freed up.
 */

void RemoteProcessBackendFactory::handleMemoryRestrictionChange()
{
    QJsonObject object;
    object.insert(RemoteProtocol::remote(), RemoteProtocol::memory());
    object.insert(RemoteProtocol::restricted(), m_memoryRestricted);
    send(object);
}

/*!
  Return a list of local internal processes.  This is separate from the
  remote internal process list, which comes from the remote host.
  You should override this function is a subclass if your subclass is
  holding a process object
*/

PidList RemoteProcessBackendFactory::localInternalProcesses() const
{
    return PidList();
}

/*!
  Handle first connection to the remote process.
  This function should be called by subclasses.
 */

void RemoteProcessBackendFactory::handleConnected()
{
    handleMemoryRestrictionChange();  // Sends command="memory" message
}

/*!
  Receive a remote \a message and dispatch it to the correct recipient.
  Call this function from your subclass to properly dispatch messages.
 */

void RemoteProcessBackendFactory::receive(const QJsonObject& message)
{
   // qDebug() << Q_FUNC_INFO << message;

    QString remote = message.value(RemoteProtocol::remote()).toString();
    if (remote == RemoteProtocol::idlecpurequested())
        setIdleCpuRequest(message.value(RemoteProtocol::request()).toBool());
    else if (remote == RemoteProtocol::internalprocesses()) {
        PidList plist = localInternalProcesses() +
            arrayToPidList(message.value(RemoteProtocol::processes()).toArray());
        qSort(plist);
        setInternalProcesses(plist);
    }
    else if (remote == RemoteProtocol::internalprocesserror()) {
        int value = (int) message.value(RemoteProtocol::processError()).toDouble();
        emit internalProcessError(static_cast<QProcess::ProcessError>(value));
    }
    else {
        int id = message.value(QLatin1String("id")).toDouble();
        if (m_backendMap.contains(id))
            m_backendMap.value(id)->receive(message);
    }
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

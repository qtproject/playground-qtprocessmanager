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

#include <QDebug>
#include <signal.h>

#include "qsocketlauncher.h"
#include "qlauncherclient.h"
#include "qremoteprotocol.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QSocketLauncher
  \brief The QSocketLauncher class accepts incoming socket connections and starts processes.
  \inmodule QtProcessManager

  The QSocketLauncher class accepts incoming socket connections and starts and stops
  processes based on JSON-formatted messages.  Each connection gets its own set of
  processes.  When a connection is dropped, all processes associated with that connection
  are killed.
 */

/*!
  Construct a QSocketLauncher with optional \a parent.
  The socket launcher opens a QJsonServer object.
 */
QSocketLauncher::QSocketLauncher(QObject *parent)
    : QProcessBackendManager(parent)
{
    m_server  = new QtAddOn::QtJsonStream::QJsonServer(this);

    connect(m_server, SIGNAL(messageReceived(const QString&, const QJsonObject&)),
            SLOT(messageReceived(const QString&, const QJsonObject&)));
    connect(m_server, SIGNAL(connectionAdded(const QString&)),
            SLOT(connectionAdded(const QString&)));
    connect(m_server, SIGNAL(connectionRemoved(const QString&)),
            SLOT(connectionRemoved(const QString&)));

    setIdleDelegate(0);  // Clear the idle delegate and get this from the master
}

/*!
  Listen to TCP socket connections on \a port.
  The optional \a authority object is used to authenticate incoming connections.
  Return true if the port can be used.
*/
bool QSocketLauncher::listen(int port, QtAddOn::QtJsonStream::QJsonAuthority *authority)
{
    return m_server->listen(port, authority);
}

/*!
  Listen to Unix local socket connections on \a socketname.
  The optional \a authority object is used to authenticate incoming connections.
  Return true if the server can be started.
*/
bool QSocketLauncher::listen(const QString& socketname, QtAddOn::QtJsonStream::QJsonAuthority *authority)
{
    return m_server->listen(socketname, authority);
}

/*!
  Return the internal QJsonServer object.
 */

QtAddOn::QtJsonStream::QJsonServer * QSocketLauncher::server() const
{
    return m_server;
}

/*!
 \internal
*/
void QSocketLauncher::connectionAdded(const QString& identifier)
{
    QLauncherClient *client = new QLauncherClient(this);
    connect(client, SIGNAL(send(const QJsonObject&)), SLOT(send(const QJsonObject&)));
    m_idToClient.insert(identifier, client);
    m_clientToId.insert(client, identifier);

    // Send our current idle request and internal process list
    if (!idleDelegate()) {
        QJsonObject object;
        object.insert(QRemoteProtocol::remote(), QRemoteProtocol::idlecpurequested());
        object.insert(QRemoteProtocol::request(), idleCpuRequest());
        sendToClient(object, client);
    }

    QJsonObject object;
    object.insert(QRemoteProtocol::remote(), QRemoteProtocol::internalprocesses());
    object.insert(QRemoteProtocol::processes(), pidListToArray(internalProcesses()));
    sendToClient(object, client);
}

/*!
 \internal
*/
void QSocketLauncher::connectionRemoved(const QString& identifier)
{
    QLauncherClient *client = m_idToClient.take(identifier);
    if (client) {
        m_clientToId.take(client);
        delete client;
    }
}

/*!
  \internal

  We override this function to send our idle cpu request back to the
  originator.  This is a little odd, in that we send the idle cpu
  request back to all connected clients.  Hopefully only one will
  respond.

  We only send the idlecpurequest message if we don't have an idle delegate
 */

void QSocketLauncher::handleIdleCpuRequest()
{
    if (!idleDelegate()) {
        QJsonObject object;
        object.insert(QRemoteProtocol::remote(), QRemoteProtocol::idlecpurequested());
        object.insert(QRemoteProtocol::request(), idleCpuRequest());
        m_server->broadcast(object);
    }
}

/*!
  \internal

  We override this function to send our updated list of internal
  processes to all clients
*/

void QSocketLauncher::handleInternalProcessChange()
{
    QJsonObject object;
    object.insert(QRemoteProtocol::remote(), QRemoteProtocol::internalprocesses());
    object.insert(QRemoteProtocol::processes(), pidListToArray(internalProcesses()));
    m_server->broadcast(object);
}

/*!
  \internal
  We override this function to forward internal process errors
 */

void QSocketLauncher::handleInternalProcessError(QProcess::ProcessError error)
{
    QJsonObject object;
    object.insert(QRemoteProtocol::remote(), QRemoteProtocol::internalprocesserror());
    object.insert(QRemoteProtocol::processError(), error);
    m_server->broadcast(object);
}

/*!
 \internal
*/

void QSocketLauncher::messageReceived(const QString& identifier, const QJsonObject& message)
{
    QString remote = message.value(QRemoteProtocol::remote()).toString();
    if (remote == QRemoteProtocol::halt())
        qDebug() << Q_FUNC_INFO << "Received halt request; ignoring";
    else if ( remote == QRemoteProtocol::memory() )
        setMemoryRestricted(message.value(QRemoteProtocol::restricted()).toBool());
    else if ( remote == QRemoteProtocol::idlecpuavailable() ) {
        if (!idleDelegate())
            idleCpuAvailable();
    }
    else {
        QLauncherClient *client = m_idToClient.value(identifier);
        if (client)
            client->receive(message);
    }
}

/*!
 \internal

 This function should only be called from a signal raised in QLauncherClient
*/
void QSocketLauncher::send(const QJsonObject& message)
{
    sendToClient(message, qobject_cast<QLauncherClient *>(sender()));
}

/*!
  \internal
 */

void QSocketLauncher::sendToClient(const QJsonObject& message, QLauncherClient *client)
{
    Q_ASSERT(client);
    Q_ASSERT(m_server);
    m_server->send(m_clientToId.value(client), message);
}


#include "moc_qsocketlauncher.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

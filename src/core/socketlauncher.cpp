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

#include <signal.h>

#include "socketlauncher.h"
#include "launcherclient.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class SocketLauncher
  \brief The SocketLauncher class accepts incoming socket connections and starts processes.

  The SocketLauncher class accepts incoming socket connections and starts and stops
  processes based on JSON-formatted messages.  Each connection gets its own set of
  processes.  When a connection is dropped, all processes associated with that connection
  are killed.
 */

/*!
  Construct a SocketLauncher with optional \a parent.
  The socket launcher opens a JsonServer object.
 */
SocketLauncher::SocketLauncher(QObject *parent)
    : ProcessBackendManager(parent)
{
    m_server  = new QtAddOn::JsonStream::JsonServer(this);

    connect(m_server, SIGNAL(messageReceived(const QString&, const QJsonObject&)),
            SLOT(messageReceived(const QString&, const QJsonObject&)));
    connect(m_server, SIGNAL(connectionAdded(const QString&)),
            SLOT(connectionAdded(const QString&)));
    connect(m_server, SIGNAL(connectionRemoved(const QString&)),
            SLOT(connectionRemoved(const QString&)));
}

/*!
  Listen to TCP socket connections on \a port.
  The optional \a authority object is used to authenticate incoming connections.
  Return true if the port can be used.
*/
bool SocketLauncher::listen(int port, QtAddOn::JsonStream::JsonAuthority *authority)
{
    return m_server->listen(port, authority);
}

/*!
  Listen to Unix local socket connections on \a socketname.
  The optional \a authority object is used to authenticate incoming connections.
  Return true if the server can be started.
*/
bool SocketLauncher::listen(const QString& socketname, QtAddOn::JsonStream::JsonAuthority *authority)
{
    return m_server->listen(socketname, authority);
}

/*!
  Return the internal JsonServer object.
 */

QtAddOn::JsonStream::JsonServer * SocketLauncher::server() const
{
    return m_server;
}

/*!
 \internal
*/
void SocketLauncher::connectionAdded(const QString& identifier)
{
    LauncherClient *client = new LauncherClient(this);
    connect(client, SIGNAL(send(const QJsonObject&)), SLOT(send(const QJsonObject&)));
    m_idToClient.insert(identifier, client);
    m_clientToId.insert(client, identifier);
}

/*!
 \internal
*/
void SocketLauncher::connectionRemoved(const QString& identifier)
{
    LauncherClient *client = m_idToClient.take(identifier);
    if (client) {
        m_clientToId.take(client);
        delete client;
    }
}

/*!
 \internal
*/
void SocketLauncher::messageReceived(const QString& identifier, const QJsonObject& message)
{
    LauncherClient *client = m_idToClient.value(identifier);
    if (client)
        client->receive(message);
}

/*!
 \internal
*/
void SocketLauncher::send(const QJsonObject& message)
{
    LauncherClient *client = qobject_cast<LauncherClient *>(sender());
    Q_ASSERT(client);
    Q_ASSERT(m_server);
    m_server->send(m_clientToId.value(client), message);
}

#include "moc_socketlauncher.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

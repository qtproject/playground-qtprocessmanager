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
#include <QLocalSocket>
#include <QJsonDocument>
#include <QtEndian>

#include "socketprocessbackendfactory.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class SocketProcessBackendFactory
  \brief The SocketProcessBackendFactory class creates RemoteProcessBackend objects

  The SocketProcessBackendFactory connects over a Unix local socket to
  an already-running "launcher" server.
*/

/*!
  Construct a SocketProcessBackendFactory with optional \a parent.
  Connect to an application launcher listening on Unix local socket \a socketname
*/

SocketProcessBackendFactory::SocketProcessBackendFactory(const QString& socketname,
                                                             QObject *parent)
    : RemoteProcessBackendFactory(parent)
{
    m_socket = new QLocalSocket(this);
    connect(m_socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
    m_socket->connectToServer(socketname);
}

/*!
   Destroy this and child objects.
*/

SocketProcessBackendFactory::~SocketProcessBackendFactory()
{
}

/*!
  The SocketProcessBackendFactory can only match if it has a valid
  connection to the socket.  The \a info parameter is passed to the
  standard matching algorithm.
*/

bool SocketProcessBackendFactory::canCreate(const ProcessInfo& info) const
{
    return m_socket->isValid() && RemoteProcessBackendFactory::canCreate(info);
}

/*!
  Read data from the socket
*/

void SocketProcessBackendFactory::readyRead()
{
    m_buffer.append(m_socket->readAll());
    while (m_buffer.size() >= 12) {   // QJsonDocuments are at least this large
        qint32 message_size = qFromLittleEndian(((qint32 *)m_buffer.data())[2]) + 8;
        if (m_buffer.size() < message_size)
            break;
        QByteArray msg = m_buffer.left(message_size);
        m_buffer = m_buffer.mid(message_size);
        receive(QJsonDocument::fromBinaryData(msg).object());
    }
}

/*!
  \internal
 */

void SocketProcessBackendFactory::disconnected()
{
    qWarning("Launcher process socket disconnected");
}

/*!
  \internal
 */

bool SocketProcessBackendFactory::send(const QJsonObject& message)
{
    return (m_socket->isValid() &&
            m_socket->write(QJsonDocument(message).toBinaryData()) != -1);
}

#include "moc_socketprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

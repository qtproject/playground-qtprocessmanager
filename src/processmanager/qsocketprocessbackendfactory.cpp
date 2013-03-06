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

#include "qsocketprocessbackendfactory.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QSocketProcessBackendFactory
  \brief The QSocketProcessBackendFactory class creates QRemoteProcessBackend objects
  \inmodule QtProcessManager

  The QSocketProcessBackendFactory connects over a Unix local socket to
  an already-running "launcher" server.
*/

/*!
  \property QSocketProcessBackendFactory::socketName
  The name of the Unix local socket that this factory should connect to.
 */

/*!
  Construct a QSocketProcessBackendFactory with optional \a parent.
*/

QSocketProcessBackendFactory::QSocketProcessBackendFactory(QObject *parent)
    : QRemoteProcessBackendFactory(parent)
{
    m_socket = new QLocalSocket(this);
    connect(m_socket, SIGNAL(connected()), SLOT(connected()));
    connect(m_socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(m_socket, SIGNAL(readyRead()), SLOT(readyRead()));
}

/*!
   Destroy this and child objects.
*/

QSocketProcessBackendFactory::~QSocketProcessBackendFactory()
{
}

/*!
  The QSocketProcessBackendFactory can only match if it has a valid
  connection to the socket.  The \a info parameter is passed to the
  standard matching algorithm.
*/

bool QSocketProcessBackendFactory::canCreate(const QProcessInfo& info) const
{
    return m_socket->isValid() && QRemoteProcessBackendFactory::canCreate(info);
}

/*!
  Returns the current socket name.  An empty string indicates that no socket has been set.
*/

QString QSocketProcessBackendFactory::socketName() const
{
    return m_socket->serverName();
}

/*!
  Set the socket name and connect to the server.  If the connection fails,
  you will not be notified, but the socketName() function will return null.
*/

void QSocketProcessBackendFactory::setSocketName(const QString& socketname)
{
    if (socketname != m_socket->serverName()) {
        m_socket->abort();
        m_socket->connectToServer(socketname);
        emit socketNameChanged();
    }
}

/*!
  Read data from the socket
*/

void QSocketProcessBackendFactory::readyRead()
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

void QSocketProcessBackendFactory::connected()
{
    handleConnected();
}

/*!
  \internal
 */

void QSocketProcessBackendFactory::disconnected()
{
    qWarning("Launcher process socket disconnected");
}

/*!
  \internal
 */

bool QSocketProcessBackendFactory::send(const QJsonObject& message)
{
    return (m_socket->isValid() &&
            m_socket->write(QJsonDocument(message).toBinaryData()) != -1);
}

/*!
  \fn void QSocketProcessBackendFactory::socketNameChanged()
  Signal emitted when the socket name has been changed
*/

#include "moc_qsocketprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

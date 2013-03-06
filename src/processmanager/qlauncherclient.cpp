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

#include "qlauncherclient.h"
#include "qremoteprotocol.h"
#include "qprocessbackend.h"
#include "qprocessbackendmanager.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QLauncherClient
  \brief The QLauncherClient class handles a single Launcher client.
  \inmodule QtProcessManager
 */

/*!
  Create a new QLauncherClient with QProcessBackendManager \a manager
 */

QLauncherClient::QLauncherClient(QProcessBackendManager *manager)
    : QObject(manager)
    , m_manager(manager)
{
}

/*!
  Process an incoming \a message
 */

void QLauncherClient::receive(const QJsonObject& message)
{
    // qDebug() << Q_FUNC_INFO << message;
    QString cmd = message.value(QRemoteProtocol::command()).toString();
    int id = message.value(QRemoteProtocol::id()).toDouble();
    if ( cmd == QRemoteProtocol::start() ) {
        QProcessInfo info(message.value(QRemoteProtocol::info()).toObject().toVariantMap());
        QProcessBackend *backend = m_manager->create(info, this);
        if (backend) {
            connect(backend, SIGNAL(started()), SLOT(started()));
            connect(backend, SIGNAL(finished(int, QProcess::ExitStatus)),
                    SLOT(finished(int, QProcess::ExitStatus)));
            connect(backend, SIGNAL(error(QProcess::ProcessError)), SLOT(error(QProcess::ProcessError)));
            connect(backend, SIGNAL(stateChanged(QProcess::ProcessState)),
                    SLOT(stateChanged(QProcess::ProcessState)));
            connect(backend, SIGNAL(standardOutput(const QByteArray&)),
                    SLOT(standardOutput(const QByteArray&)));
            connect(backend, SIGNAL(standardError(const QByteArray&)),
                    SLOT(standardError(const QByteArray&)));
            m_idToBackend.insert(id, backend);
            m_backendToId.insert(backend, id);
            backend->start();
        }
    }
    else if ( cmd == QRemoteProtocol::stop() ) {
        QProcessBackend *backend = m_idToBackend.value(id);
        if (backend) {
            int timeout = message.value(QRemoteProtocol::timeout()).toDouble();
            backend->stop(timeout);
        }
    }
    else if ( cmd == QRemoteProtocol::set() ) {
        QProcessBackend *backend = m_idToBackend.value(id);
        if (backend) {
            QString key = message.value(QRemoteProtocol::key()).toString();
            int value   = message.value(QRemoteProtocol::value()).toDouble();
            if (key == QRemoteProtocol::priority())
                backend->setDesiredPriority(value);
            else if (key == QRemoteProtocol::oomAdjustment())
                backend->setDesiredOomAdjustment(value);
        }
    }
    else if ( cmd == QRemoteProtocol::write() ) {
        QByteArray data = QByteArray::fromBase64(message.value(QRemoteProtocol::data()).toString().toLocal8Bit());
        QProcessBackend *backend = m_idToBackend.value(id);
        if (backend)
            backend->write(data);
    }
}

/*!
  \internal
 */

void QLauncherClient::started()
{
    QProcessBackend *backend = qobject_cast<QProcessBackend *>(sender());
    QJsonObject msg;
    msg.insert(QRemoteProtocol::event(), QRemoteProtocol::started());
    msg.insert(QRemoteProtocol::id(), m_backendToId.value(backend));
    msg.insert(QRemoteProtocol::pid(), (double) backend->pid());
    emit send(msg);
}

/*!
  \internal
 */

void QLauncherClient::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcessBackend *backend = qobject_cast<QProcessBackend *>(sender());
    QJsonObject msg;
    msg.insert(QRemoteProtocol::event(), QRemoteProtocol::finished());
    msg.insert(QRemoteProtocol::id(), m_backendToId.value(backend));
    msg.insert(QRemoteProtocol::exitCode(), exitCode);
    msg.insert(QRemoteProtocol::exitStatus(), exitStatus);
    emit send(msg);
}

/*!
  \internal
 */

void QLauncherClient::error(QProcess::ProcessError err)
{
    QProcessBackend *backend = qobject_cast<QProcessBackend *>(sender());
    QJsonObject msg;
    msg.insert(QRemoteProtocol::event(), QRemoteProtocol::error());
    msg.insert(QRemoteProtocol::id(), m_backendToId.value(backend));
    msg.insert(QRemoteProtocol::error(), err);
    msg.insert(QRemoteProtocol::errorString(), backend->errorString());
    emit send(msg);
}

/*!
  \internal
 */

void QLauncherClient::stateChanged(QProcess::ProcessState state)
{
    QProcessBackend *backend = qobject_cast<QProcessBackend *>(sender());
    QJsonObject msg;
    msg.insert(QRemoteProtocol::event(), QRemoteProtocol::stateChanged());
    msg.insert(QRemoteProtocol::id(), m_backendToId.value(backend));
    msg.insert(QRemoteProtocol::stateChanged(), state);
    emit send(msg);
}

/*!
  \internal
 */

void QLauncherClient::standardOutput(const QByteArray& data)
{
    QProcessBackend *backend = qobject_cast<QProcessBackend *>(sender());
    QJsonObject msg;
    msg.insert(QRemoteProtocol::event(), QRemoteProtocol::output());
    msg.insert(QRemoteProtocol::id(), m_backendToId.value(backend));
    msg.insert(QRemoteProtocol::standardout(), QString::fromLocal8Bit(data.data(), data.size()));
    emit send(msg);
}

/*!
  \internal
 */

void QLauncherClient::standardError(const QByteArray& data)
{
    QProcessBackend *backend = qobject_cast<QProcessBackend *>(sender());
    QJsonObject msg;
    msg.insert(QRemoteProtocol::event(), QRemoteProtocol::output());
    msg.insert(QRemoteProtocol::id(), m_backendToId.value(backend));
    msg.insert(QRemoteProtocol::standarderror(), QString::fromLocal8Bit(data.data(), data.size()));
    emit send(msg);
}

/*!
  \fn void QLauncherClient::send(const QJsonObject& message)

  Send a \a message to the remote controller.
*/

#include "moc_qlauncherclient.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

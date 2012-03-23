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
#include <QDebug>
#include <QtEndian>
#include <QJsonDocument>

#include <jsonpipe.h>

#include "pipelauncher.h"
#include "launcherclient.h"
#include "remoteprotocol.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class PipeLauncher
  \brief The PipeLauncher class accepts input from STDIN and writes data to STDOUT

  The PipeLauncher class is a ProcessBackendManager controlled over Unix
  pipes.  It accepts JSON-formatted commands over STDIN and returns
  JSON-formatted messages over STDOUT.
 */

/*!
  Construct a PipeLauncher with optional \a parent.
  The PipeLauncher reads JSON-formatted messages from stdin and
  writes replies on stdout.
*/

PipeLauncher::PipeLauncher(QObject *parent)
    : ProcessBackendManager(parent)
{
    m_pipe   = new QtAddOn::JsonStream::JsonPipe(this);
    m_client = new LauncherClient(this);

    connect(m_pipe, SIGNAL(messageReceived(const QJsonObject&)),
            SLOT(receive(const QJsonObject&)));
    connect(m_client, SIGNAL(send(const QJsonObject&)),
            m_pipe, SLOT(send(const QJsonObject&)));

    m_pipe->setFds(STDIN_FILENO, STDOUT_FILENO);

    // Clear the idle delegate - we'll get this from the master
    setIdleDelegate(0);
}

/*!
  \internal

  We override this function to send our idle cpu request back to the
  originator.  The \a request variable will be \c{true} if Idle CPU events are needed.
  We only forward the request if we don't have an idle delegate.
 */

void PipeLauncher::handleIdleCpuRequest()
{
    Q_ASSERT(m_client);

    if (!idleDelegate()) {
        QJsonObject object;
        object.insert(RemoteProtocol::remote(), RemoteProtocol::idlecpurequested());
        object.insert(RemoteProtocol::request(), idleCpuRequest());
        m_client->send(object);
    }
}

/*!
  \internal

  We override this function to send our updated list of internal
  processes back.  Please note that we do not include our own process -
  if that should be on the list, it's up to the controlling side to add it.
*/

void PipeLauncher::handleInternalProcessChange()
{
    Q_ASSERT(m_client);

    QJsonObject object;
    object.insert(RemoteProtocol::remote(), RemoteProtocol::internalprocesses());
    object.insert(RemoteProtocol::processes(), pidListToArray(internalProcesses()));
    m_client->send(object);
}

/*!
  \internal
 */

void PipeLauncher::receive(const QJsonObject& message)
{
    QString remote = message.value(RemoteProtocol::remote()).toString();
    if (remote == RemoteProtocol::halt())  // ### TODO: Should halt children
        exit(0);
    else if ( remote == RemoteProtocol::memory() )
        setMemoryRestricted(message.value(RemoteProtocol::restricted()).toBool());
    else if ( remote == RemoteProtocol::idlecpuavailable() ) {
        if (!idleDelegate())
            idleCpuAvailable();  // Only accept idlecpuavailable if we have no idleDelegate
    }
    else
        m_client->receive(message);
}

#include "moc_pipelauncher.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

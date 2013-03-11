/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <QDebug>
#include <QtEndian>
#include <QJsonDocument>

#include "qpipelauncher.h"
#include "qlauncherclient.h"
#include "qremoteprotocol.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QPipeLauncher
  \brief The QPipeLauncher class accepts input from STDIN and writes data to STDOUT
  \inmodule QtProcessManager

  The QPipeLauncher class is a QProcessBackendManager controlled over Unix
  pipes.  It accepts JSON-formatted commands over STDIN and returns
  JSON-formatted messages over STDOUT.
 */

/*!
  Construct a QPipeLauncher with optional \a parent.
  The QPipeLauncher reads JSON-formatted messages from stdin and
  writes replies on stdout.
*/

QPipeLauncher::QPipeLauncher(QObject *parent)
    : QProcessBackendManager(parent)
{
    m_pipe   = new QtAddOn::QtJsonStream::QJsonPipe(this);
    m_client = new QLauncherClient(this);

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

void QPipeLauncher::handleIdleCpuRequest()
{
    Q_ASSERT(m_client);

    if (!idleDelegate()) {
        QJsonObject object;
        object.insert(QRemoteProtocol::remote(), QRemoteProtocol::idlecpurequested());
        object.insert(QRemoteProtocol::request(), idleCpuRequest());
        m_client->send(object);
    }
}

/*!
  \internal

  We override this function to send our updated list of internal
  processes back.  Please note that we do not include our own process -
  if that should be on the list, it's up to the controlling side to add it.
*/

void QPipeLauncher::handleInternalProcessChange()
{
    Q_ASSERT(m_client);

    QJsonObject object;
    object.insert(QRemoteProtocol::remote(), QRemoteProtocol::internalprocesses());
    object.insert(QRemoteProtocol::processes(), pidListToArray(internalProcesses()));
    m_client->send(object);
}

/*!
  \internal

  We override this function to forward internal process errors
 */

void QPipeLauncher::handleInternalProcessError(QProcess::ProcessError error)
{
    Q_ASSERT(m_client);

    QJsonObject object;
    object.insert(QRemoteProtocol::remote(), QRemoteProtocol::internalprocesserror());
    object.insert(QRemoteProtocol::processError(), error);
    m_client->send(object);
}

/*!
  \internal
 */

void QPipeLauncher::receive(const QJsonObject& message)
{
    QString remote = message.value(QRemoteProtocol::remote()).toString();
    if (remote == QRemoteProtocol::halt())  // ### TODO: Should halt children
        exit(0);
    else if ( remote == QRemoteProtocol::memory() )
        setMemoryRestricted(message.value(QRemoteProtocol::restricted()).toBool());
    else if ( remote == QRemoteProtocol::idlecpuavailable() ) {
        if (!idleDelegate())
            idleCpuAvailable();  // Only accept idlecpuavailable if we have no idleDelegate
    }
    else
        m_client->receive(message);
}

#include "moc_qpipelauncher.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

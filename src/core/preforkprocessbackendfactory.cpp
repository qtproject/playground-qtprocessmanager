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

#include "preforkprocessbackendfactory.h"
#include "prefork.h"
#include "remoteprocessbackend.h"
#include "remoteprotocol.h"

#include <QDebug>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kPreforkTimerInterval = 1000;

/*!
  \class PreforkProcessBackendFactory
  \brief The PreforkProcessBackendFactory class connects to a preforked client

  The PreforkProcessBackendFactory class can only be used in a process
  that has been started by the \l{Prefork} class.  Each
  PreforkProcessBackendFactory object must be associated with a single
  child process started from the \l{Prefork} class.  The association is
  done by setting the \l{index} property to match a valid index from
  the \l{Prefork} singleton object.

  The PreforkProcessBackendFactory communicates with the child process
  using the same protocol as the \l{PipeProcessBackendFactory} (simple
  JSON-formatted messages).
*/

/*!
  \property PreforkProcessBackendFactory::index
  Index into the prefork instance of which process this should be
  connected to.
*/

/*!
  Construct a PreforkProcessBackendFactory with optional \a parent.
*/

PreforkProcessBackendFactory::PreforkProcessBackendFactory(QObject *parent)
    : RemoteProcessBackendFactory(parent)
    , m_index(-1)
{
    m_pipe   = new QtAddOn::JsonStream::JsonPipe(this);
    connect(m_pipe, SIGNAL(messageReceived(const QJsonObject&)),
            SLOT(receive(const QJsonObject&)));
    handleConnected();
}

/*!
   Destroy this and child objects.
*/

PreforkProcessBackendFactory::~PreforkProcessBackendFactory()
{
    if (m_index >= 0) {
        const PreforkChildData *data = Prefork::instance()->at(m_index);
        if (data) {
            QJsonObject message;
            message.insert(RemoteProtocol::remote(), RemoteProtocol::halt());
            m_pipe->send(message);
            m_pipe->waitForBytesWritten();
        }
    }
}

/*!
  Return the process index
 */

int PreforkProcessBackendFactory::index() const
{
    return m_index;
}

/*!
  Set the prefork index to \a index
 */

void PreforkProcessBackendFactory::setIndex(int index)
{
    Prefork *prefork = Prefork::instance();
    Q_ASSERT(prefork);

    if (index >= 0 && index < prefork->size()) {
        m_index = index;
        const PreforkChildData *data = prefork->at(index);
        m_pipe->setFds(data->out, data->in);
        emit indexChanged();
    }
    else
        qWarning() << Q_FUNC_INFO << "index out of range";
}

/*!
  Return the local process
 */

PidList PreforkProcessBackendFactory::localInternalProcesses() const
{
    Prefork *prefork = Prefork::instance();
    Q_ASSERT(prefork);

    QList<Q_PID> list;
    if (m_index >= 0 && m_index < prefork->size()) {
        const PreforkChildData *data = prefork->at(m_index);
        list << data->pid;
    }
    return list;
}

/*!
  Send a \a message to the preforked process
 */

bool PreforkProcessBackendFactory::send(const QJsonObject& message)
{
    return m_pipe->send(message);
}

/*!
  \fn PreforkProcessBackendFactory::indexChanged()
  This signal is emitted when the index is changed.
 */

#include "moc_preforkprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

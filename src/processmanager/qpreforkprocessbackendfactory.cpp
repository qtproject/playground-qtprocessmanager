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

#include "qpreforkprocessbackendfactory.h"
#include "qprefork.h"
#include "qremoteprotocol.h"

#include <QDebug>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kPreforkTimerInterval = 1000;

/*!
  \class QPreforkProcessBackendFactory
  \brief The QPreforkProcessBackendFactory class connects to a preforked client
  \inmodule QtProcessManager

  The QPreforkProcessBackendFactory class can only be used in a process
  that has been started by the \l{QPrefork} class.  Each
  QPreforkProcessBackendFactory object must be associated with a single
  child process started from the \l{QPrefork} class.  The association is
  done by setting the \l{index} property to match a valid index from
  the \l{QPrefork} singleton object.

  The QPreforkProcessBackendFactory communicates with the child process
  using the same protocol as the \l{QPipeProcessBackendFactory} (simple
  JSON-formatted messages).
*/

/*!
  \property QPreforkProcessBackendFactory::index
  Index into the prefork instance of which process this should be
  connected to.
*/

/*!
  Construct a QPreforkProcessBackendFactory with optional \a parent.
*/

QPreforkProcessBackendFactory::QPreforkProcessBackendFactory(QObject *parent)
    : QRemoteProcessBackendFactory(parent)
    , m_index(-1)
{
    m_pipe   = new QtAddOn::QtJsonStream::QJsonPipe(this);
    connect(m_pipe, SIGNAL(messageReceived(const QJsonObject&)),
            SLOT(receive(const QJsonObject&)));
    handleConnected();
}

/*!
   Destroy this and child objects.
*/

QPreforkProcessBackendFactory::~QPreforkProcessBackendFactory()
{
    if (m_index >= 0) {
        const QPreforkChildData *data = QPrefork::instance()->at(m_index);
        if (data) {
            QJsonObject message;
            message.insert(QRemoteProtocol::remote(), QRemoteProtocol::halt());
            m_pipe->send(message);
            m_pipe->waitForBytesWritten();
        }
    }
}

/*!
  Return the process index
 */

int QPreforkProcessBackendFactory::index() const
{
    return m_index;
}

/*!
  Set the prefork index to \a index
 */

void QPreforkProcessBackendFactory::setIndex(int index)
{
    QPrefork *prefork = QPrefork::instance();
    Q_ASSERT(prefork);

    if (index >= 0 && index < prefork->size()) {
        m_index = index;
        const QPreforkChildData *data = prefork->at(index);
        m_pipe->setFds(data->out, data->in);
        emit indexChanged();
    }
    else
        qWarning() << Q_FUNC_INFO << "index out of range";
}

/*!
  Return the local process
 */

QPidList QPreforkProcessBackendFactory::localInternalProcesses() const
{
    QPrefork *prefork = QPrefork::instance();
    Q_ASSERT(prefork);

    QList<Q_PID> list;
    if (m_index >= 0 && m_index < prefork->size()) {
        const QPreforkChildData *data = prefork->at(m_index);
        list << data->pid;
    }
    return list;
}

/*!
  Send a \a message to the preforked process
 */

bool QPreforkProcessBackendFactory::send(const QJsonObject& message)
{
    return m_pipe->send(message);
}

/*!
  \fn QPreforkProcessBackendFactory::indexChanged()
  This signal is emitted when the index is changed.
 */

#include "moc_qpreforkprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

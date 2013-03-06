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

#include "qpipeprocessbackendfactory.h"
#include "qremoteprocessbackend.h"
#include "qremoteprotocol.h"
#include "qprocessinfo.h"

#include <QDebug>
#include <QJsonDocument>
#include <QFileInfo>
#include <QtEndian>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kPipeTimerInterval = 1000;

/*!
  \class QPipeProcessBackendFactory
  \brief The QPipeProcessBackendFactory class forks a new process to launch applications.
  \inmodule QtProcessManager

  The QPipeProcessBackendFactory launches a persistent pipe process.
  The factory communicates with the pipe process by sending and receiving
  messages over stdin/stdout.
*/

/*!
  \property QPipeProcessBackendFactory::processInfo
  \brief QProcessInfo record used to create the pipe process
 */

/*!
  Construct a QPipeProcessBackendFactory with optional \a parent.
  You must set a QProcessInfo object before this factory will be activated.
*/

QPipeProcessBackendFactory::QPipeProcessBackendFactory(QObject *parent)
    : QRemoteProcessBackendFactory(parent)
    , m_process(NULL)
    , m_info(NULL)
{
}

/*!
   Destroy this and child objects.
*/

QPipeProcessBackendFactory::~QPipeProcessBackendFactory()
{
    stopRemoteProcess();
}

/*!
  \internal

  The m_process process is NOT a child of the factory to avoid
  stranding grandchildren (which would happen if we summarily
  kill it).  Instead, we send it a "stop" message and count on
  the remote process to kill itself.
 */

void QPipeProcessBackendFactory::stopRemoteProcess()
{
    if (m_process && m_process->state() == QProcess::Running) {
        QJsonObject object;
        object.insert(QRemoteProtocol::remote(), QRemoteProtocol::halt());
        m_process->write(QJsonDocument(object).toBinaryData());
        m_process->waitForBytesWritten();  // Block until they have been written
        m_process = NULL;
    }
}

/*!
  Return true if the QPipeProcessBackendFactory can create a process
  that matches \a info.  The default implementation only checks that a
  valid info object has been previously set in the factory, and then
  passes the decision on to the default implementation, which should
  probably have some kind of QMatchDelegate installed.
*/

bool QPipeProcessBackendFactory::canCreate(const QProcessInfo &info) const
{
    if (!m_info || !m_process || m_process->state() != QProcess::Running)
        return false;

    return QRemoteProcessBackendFactory::canCreate(info);
}

/*!
    Sets the QProcessInfo that is used to create the pipe process to \a processInfo.
    An internal copy is made of the \a processInfo object.
    This routine will start the pipe process.

    TODO:  If you set the process info twice, you may end up with local
    process backend objects that are invalid and refer to children that
    don't exist.
 */

void QPipeProcessBackendFactory::setProcessInfo(QProcessInfo *processInfo)
{
    if (m_info != processInfo) {
        if (m_info) {
            delete m_info;
            m_info = NULL;
        }

        stopRemoteProcess();

        if (processInfo) {
            m_info = new QProcessInfo(*processInfo);
            m_info->setParent(this);

            m_process = new QProcess;  // Note that we do NOT own the pipe process
            m_process->setReadChannel(QProcess::StandardOutput);
            connect(m_process, SIGNAL(readyReadStandardOutput()),
                    this, SLOT(pipeReadyReadStandardOutput()));
            connect(m_process, SIGNAL(readyReadStandardError()),
                    this, SLOT(pipeReadyReadStandardError()));
            connect(m_process, SIGNAL(started()), this, SLOT(pipeStarted()));
            connect(m_process,SIGNAL(error(QProcess::ProcessError)),
                    this,SLOT(pipeError(QProcess::ProcessError)));
            connect(m_process,SIGNAL(finished(int, QProcess::ExitStatus)),
                    this,SLOT(pipeFinished(int, QProcess::ExitStatus)));
            connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)),
                    this,SLOT(pipeStateChanged(QProcess::ProcessState)));

            QProcessEnvironment env;
            QMapIterator<QString, QVariant> it(m_info->environment());
            while (it.hasNext()) {
                it.next();
                env.insert(it.key(), it.value().toString());
            }
            m_process->setProcessEnvironment(env);
            m_process->setWorkingDirectory(m_info->workingDirectory());
            m_process->start(m_info->program(), m_info->arguments());
        }
        emit processInfoChanged();
    }
}

/*!
    Sets the QProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
 */
void QPipeProcessBackendFactory::setProcessInfo(QProcessInfo& processInfo)
{
    setProcessInfo(&processInfo);
}

/*!
  Return the pipe process information
 */

QProcessInfo *QPipeProcessBackendFactory::processInfo() const
{
    return m_info;
}

/*!
  Return the local process if it is running
 */

QPidList QPipeProcessBackendFactory::localInternalProcesses() const
{
    QPidList result;
    if (m_process && m_process->state() == QProcess::Running)
        result << m_process->pid();
    return result;
}

/*!
  Send \a message to a pipe process.
 */
bool QPipeProcessBackendFactory::send(const QJsonObject& message)
{
    // qDebug() << Q_FUNC_INFO << message;
    return (m_process->state() == QProcess::Running &&
            m_process->write(QJsonDocument(message).toBinaryData()) != -1);
}


void QPipeProcessBackendFactory::pipeReadyReadStandardOutput()
{
    m_buffer.append(m_process->readAllStandardOutput());
    while (m_buffer.size() >= 12) {   // QJsonDocuments are at least this large
        if (QJsonDocument::BinaryFormatTag != *((uint *) m_buffer.data()))
            qFatal("ERROR in receive buffer: %s", m_buffer.data());
        qint32 message_size = qFromLittleEndian(((qint32 *)m_buffer.data())[2]) + 8;
        if (m_buffer.size() < message_size)
            break;
        QByteArray msg = m_buffer.left(message_size);
        m_buffer = m_buffer.mid(message_size);
        receive(QJsonDocument::fromBinaryData(msg).object());
    }
}

void QPipeProcessBackendFactory::pipeReadyReadStandardError()
{
    const QByteArray byteArray = m_process->readAllStandardError();
    QList<QByteArray> lines = byteArray.split('\n');
    foreach (const QByteArray& line, lines) {
        if (line.size())
            qDebug() << "PIPE STDERR" << line;
    }
}

void QPipeProcessBackendFactory::pipeStarted()
{
    handleConnected();
}

void QPipeProcessBackendFactory::pipeError(QProcess::ProcessError error)
{
    qWarning("Pipe process error: %d", error);
}

void QPipeProcessBackendFactory::pipeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCritical("Pipe process died, exit code=%d status=%d", exitCode, exitStatus);
    delete m_process;
    m_process = NULL;
}

void QPipeProcessBackendFactory::pipeStateChanged(QProcess::ProcessState)
{
    // This may result in a small amount of extra because not all transitions
    // result in a change in the number of internal processes.
    setInternalProcesses(localInternalProcesses());
}


/*!
  \fn void QPipeProcessBackendFactory::processInfoChanged()
  This signal is emitted when the internal QProcessInfo record is
  changed.
 */

#include "moc_qpipeprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

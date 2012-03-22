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

#include "pipeprocessbackendfactory.h"
#include "remoteprocessbackend.h"
#include "remoteprotocol.h"
#include "processinfo.h"

#include <QDebug>
#include <QJsonDocument>
#include <QFileInfo>
#include <QtEndian>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kPipeTimerInterval = 1000;

/*!
  \class PipeProcessBackendFactory
  \brief The PipeProcessBackendFactory class forks a new process to launch applications.

  The PipeProcessBackendFactory launches a persistent pipe process.
  The factory communicates with the pipe process by sending and receiving
  messages over stdin/stdout.
*/

/*!
  \property PipeProcessBackendFactory::processInfo
  \brief ProcessInfo record used to create the pipe process
 */

/*!
  Construct a PipeProcessBackendFactory with optional \a parent.
  You must set a ProcessInfo object before this factory will be activated.
*/

PipeProcessBackendFactory::PipeProcessBackendFactory(QObject *parent)
    : RemoteProcessBackendFactory(parent)
    , m_process(NULL)
    , m_info(NULL)
{
}

/*!
   Destroy this and child objects.
*/

PipeProcessBackendFactory::~PipeProcessBackendFactory()
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

void PipeProcessBackendFactory::stopRemoteProcess()
{
    if (m_process && m_process->state() == QProcess::Running) {
        QJsonObject object;
        object.insert(RemoteProtocol::remote(), RemoteProtocol::stop());
        m_process->write(QJsonDocument(object).toBinaryData());
        m_process->waitForBytesWritten();  // Block until they have been written
        m_process = NULL;
    }
}

/*!
  Return true if the PipeProcessBackendFactory can create a process
  that matches \a info.  The default implementation only checks that a
  valid info object has been previously set in the factory, and then
  passes the decision on to the default implementation, which should
  probably have some kind of MatchDelegate installed.
*/

bool PipeProcessBackendFactory::canCreate(const ProcessInfo &info) const
{
    if (!m_info || !m_process || m_process->state() != QProcess::Running)
        return false;

    return RemoteProcessBackendFactory::canCreate(info);
}

/*!
  If there is a pipe process running, it will be returned here.
 */

QList<Q_PID> PipeProcessBackendFactory::internalProcesses()
{
    QList<Q_PID> list;
    if (m_process && m_process->state() == QProcess::Running)
        list << m_process->pid();
    return list;
}

/*!
    Sets the ProcessInfo that is used to create the pipe process to \a processInfo.
    An internal copy is made of the \a processInfo object.
    This routine will start the pipe process.

    TODO:  If you set the process info twice, you may end up with local
    process backend objects that are invalid and refer to children that
    don't exist.
 */

void PipeProcessBackendFactory::setProcessInfo(ProcessInfo *processInfo)
{
    if (m_info != processInfo) {
        if (m_info) {
            delete m_info;
            m_info = NULL;
        }

        stopRemoteProcess();

        if (processInfo) {
            m_info = new ProcessInfo(*processInfo);
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
    Sets the ProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
 */
void PipeProcessBackendFactory::setProcessInfo(ProcessInfo& processInfo)
{
    setProcessInfo(&processInfo);
}

/*!
  Return the pipe process information
 */

ProcessInfo *PipeProcessBackendFactory::processInfo() const
{
    return m_info;
}

/*!
  Send \a message to a pipe process.
 */
bool PipeProcessBackendFactory::send(const QJsonObject& message)
{
    if (m_process->state() != QProcess::Running) {
        qCritical("Pipe process not running");
        return false;
    }
    if (m_process->write(QJsonDocument(message).toBinaryData()) == -1)  {
        qCritical("Unable to write to pipe process");
        return false;
    }
    return true;
}


void PipeProcessBackendFactory::pipeReadyReadStandardOutput()
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

void PipeProcessBackendFactory::pipeReadyReadStandardError()
{
    const QByteArray byteArray = m_process->readAllStandardError();
    QList<QByteArray> lines = byteArray.split('\n');
    foreach (const QByteArray& line, lines) {
        if (line.size())
            qDebug() << "PIPE STDERR" << line;
    }
}

void PipeProcessBackendFactory::pipeStarted()
{
}

void PipeProcessBackendFactory::pipeError(QProcess::ProcessError error)
{
    qWarning("Pipe process error: %d", error);
}

void PipeProcessBackendFactory::pipeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCritical("Pipe process died, exit code=%d status=%d", exitCode, exitStatus);
    delete m_process;
    m_process = NULL;
}

void PipeProcessBackendFactory::pipeStateChanged(QProcess::ProcessState state)
{
    Q_UNUSED(state);
}


/*!
  \fn void PipeProcessBackendFactory::processInfoChanged()
  This signal is emitted when the internal ProcessInfo record is
  changed.
 */

#include "moc_pipeprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

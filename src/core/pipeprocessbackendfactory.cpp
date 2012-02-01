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
  Construct a PipeProcessBackendFactory with optional \a parent.
  The \a info ProcessInfo is used to start the pipe process.
  The \a program is used to match program names for create() requests.
*/

PipeProcessBackendFactory::PipeProcessBackendFactory(const ProcessInfo& info,
                                                           const QString& program,
                                                           QObject *parent)
    : RemoteProcessBackendFactory(parent)
    , m_process(NULL)
    , m_program(program)
{
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
    QMapIterator<QString, QVariant> it(info.environment());
    while (it.hasNext()) {
        it.next();
        env.insert(it.key(), it.value().toString());
    }
    m_process->setProcessEnvironment(env);
    m_process->setWorkingDirectory(info.workingDirectory());
    m_process->start(info.program(), info.arguments());
}

/*!
   Destroy this and child objects.
*/

PipeProcessBackendFactory::~PipeProcessBackendFactory()
{
    // ### Note: The m_process process is NOT a child of the
    //           factory to avoid stranding grandchildren
    //           However, we do send it a "stop" message before we exit
    if (m_process) {
        QJsonObject object;
        object.insert(QLatin1String("remote"), QLatin1String("stop"));
        m_process->write(QJsonDocument(object).toBinaryData());
        m_process->waitForBytesWritten();  // Block until they have been written
        m_process = NULL;
    }
}

/*!
  The PipeProcessBackendFactory will match the ProcessInfo \a info
  if there is a \c info.pipe attribute set to "true" (the string) and
  if the \c info.program attribute matches the program original specified
  when creating the factory.
*/

bool PipeProcessBackendFactory::canCreate(const ProcessInfo& info) const
{
    QString program = QFileInfo(info.program()).fileName();
    return (m_process &&
            m_process->state() == QProcess::Running &&
            info.value("pipe").toString() == "true" &&
            program == m_program);
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
  Send \a message to a pipe process.
 */
bool PipeProcessBackendFactory::send(const QJsonObject& message)
{
    // qDebug() << Q_FUNC_INFO << message;
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
//    qDebug() << "Pipe process started";
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
//    qDebug() << "Pipe process state change" << state;
}

#include "moc_pipeprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

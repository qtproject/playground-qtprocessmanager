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



#include "unixprocessbackend.h"
#include "unixsandboxprocess.h"
#include "procutils.h"
#include <sys/resource.h>
#include <errno.h>
#include <QDebug>
#include <QFile>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class UnixProcessBackend
    \brief The UnixProcessBackend class wraps a QProcess object
*/

/*!
    Construct a UnixProcessBackend with ProcessInfo \a info and optional \a parent
*/

UnixProcessBackend::UnixProcessBackend(const ProcessInfo& info, QObject *parent)
    : ProcessBackend(info, parent)
    , m_process(0)
{
}

/*!
  Destroy this process object.
  Any created QProcess is a child of this object, so it will be automatically terminated.
*/

UnixProcessBackend::~UnixProcessBackend()
{
}

/*!
    Returns the PID of this process. If the process has not started up yet properly, its PID will be 0.
*/
Q_PID UnixProcessBackend::pid() const
{
    if (m_process)
        return m_process->pid();
    return 0;
}

/*!
    Return the actual process priority (if running)
*/

qint32 UnixProcessBackend::actualPriority() const
{
    if (m_process) {
        errno = 0;   // getpriority can return -1, so we clear errno
        int result = getpriority(PRIO_PROCESS, m_process->pid());
        if (!errno)
            return result;
    }
    return ProcessBackend::actualPriority();
}

/*!
    Set the process priority to \a priority
*/

void UnixProcessBackend::setDesiredPriority(qint32 priority)
{
    ProcessBackend::setDesiredPriority(priority);
    if (m_process) {
        // ### Is this always correct?  Could we have an m_process without a pid?
        if (setpriority(PRIO_PROCESS, m_process->pid(), priority))
            qWarning() << "Failed to set process priority from " << actualPriority() <<
                          "to" << priority << " : errno = " << errno;
    }
}

#if defined(Q_OS_LINUX)

/*!
    Return the process oomAdjustment
*/

qint32 UnixProcessBackend::actualOomAdjustment() const
{
    // ### TODO: What if m_process doesn't have a valid PID yet?

    if (m_process) {
        bool ok;
        qint32 result = ProcUtils::oomAdjustment(m_process->pid(), &ok);
        if (ok)
            return result;
        qWarning() << "Unable to read oom adjustment for" << m_process->pid();
    }
    return ProcessBackend::actualOomAdjustment();
}

/*!
    Set the process /proc/<pid>/oom_score_adj to \a oomAdjustment
*/

void UnixProcessBackend::setDesiredOomAdjustment(qint32 oomAdjustment)
{
    ProcessBackend::setDesiredOomAdjustment(oomAdjustment);
    if (m_process) {
        if (!ProcUtils::setOomAdjustment(m_process->pid(), oomAdjustment))
            qWarning() << "Unable to set oom adjustment for" << m_process->pid();
    }
}

#endif // defined(Q_OS_LINUX)

/*!
    Returns the state of the process.
    The base class always returns NotRunning.
*/
QProcess::ProcessState UnixProcessBackend::state() const
{
    return m_process ? m_process->state() : QProcess::NotRunning;
}

/*!
    Internal function to create the QProcess.
    Returns true if a process was created.
*/
bool UnixProcessBackend::createProcess()
{
    if (m_process) {
        qWarning() << "Can't restart process!";
        return false;
    }

    if (m_info.contains(ProcessInfoConstants::Uid) || m_info.contains(ProcessInfoConstants::Gid))
        m_process = new UnixSandboxProcess(m_info.uid(), m_info.gid(), this);
    else
        m_process = new QProcess(this);

    m_process->setReadChannel(QProcess::StandardOutput);
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readyReadStandardOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()),
            this, SLOT(readyReadStandardError()));
    connect(&m_killTimer, SIGNAL(timeout()), this, SLOT(killTimeout()));

    connect(m_process, SIGNAL(started()), this, SLOT(unixProcessStarted()));
    connect(m_process,SIGNAL(error(QProcess::ProcessError)),
            this,SLOT(unixProcessError(QProcess::ProcessError)));
    connect(m_process,SIGNAL(finished(int, QProcess::ExitStatus)),
            this,SLOT(unixProcessFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(stateChanged(QProcess::ProcessState)),
            this,SLOT(unixProcessStateChanged(QProcess::ProcessState)));
    return true;
}

/*!
    Internal function to start the QProcess running.
*/

void UnixProcessBackend::startProcess()
{
    QProcessEnvironment env;
    QMapIterator<QString, QVariant> it(m_info.environment());
    while (it.hasNext()) {
        it.next();
        env.insert(it.key(), it.value().toString());
    }
    m_process->setProcessEnvironment(env);
    m_process->setWorkingDirectory(m_info.workingDirectory());
    m_process->start(m_info.program(), m_info.arguments());
    // qDebug() << Q_FUNC_INFO << "Started process" << m_info.program();
}

/*!
    Attempts to stop a process by giving it a \a timeout time to die, measured in milliseconds.

    If the process does not die in the given time limit, it is killed.

    \sa finished()
*/

void UnixProcessBackend::stop(int timeout)
{
    Q_ASSERT(m_process);

    if (m_process->state() != QProcess::NotRunning) {
        if (timeout > 0) {
        m_process->terminate();
            m_killTimer.start(timeout);
    }
    else
        m_process->kill();
    }
}

/*!
  Writes at most \a maxSize bytes of data from \a data to the device.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
*/
qint64 UnixProcessBackend::write(const char *data, qint64 maxSize)
{
    if (m_process)
    return m_process->write(data, maxSize);
    return 0;
}

/*!
    Override this in subclasses.  Make sure you call the parent class.
    Your subclass should emit \sa started()
*/
void UnixProcessBackend::handleProcessStarted()
{
    if (m_info.contains("priority") && setpriority(PRIO_PROCESS, m_process->pid(), m_info.priority()))
        qWarning() << "Failed to set process priority at startup from " << actualPriority() <<
            "to" << m_info.priority()  << " : errno = " << errno;
    if (m_info.contains("oomAdjustment") &&
        !ProcUtils::setOomAdjustment(m_process->pid(), m_info.oomAdjustment()))
        qWarning() << "Failed to set process oom score at startup from " << actualOomAdjustment() <<
            "to" << m_info.oomAdjustment();
}
/*!
    Override this in subclasses.  Make sure you call the parent class with \a error.
    Your subclass should emit \sa error()
*/
void UnixProcessBackend::handleProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
}

/*!
    Override this in subclasses.  Make sure you call the parent class with \a exitCode
    and \a exitStatus.  Your subclass should emit \sa finished()
*/
void UnixProcessBackend::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    m_killTimer.stop();
}

/*!
    Override this in subclasses.  Make sure you call the parent class with \a state.
    Your subclass should emit \sa stateChanged()
*/
void UnixProcessBackend::handleProcessStateChanged(QProcess::ProcessState state)
{
    Q_UNUSED(state);
    m_killTimer.stop();
}

/*!
    \internal
*/
void UnixProcessBackend::unixProcessStarted()
{
    handleProcessStarted();
}

/*!
    \internal
*/
void UnixProcessBackend::unixProcessError(QProcess::ProcessError error)
{
    handleProcessError(error);
}

/*!
    \internal
*/
void UnixProcessBackend::unixProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    handleProcessFinished(exitCode, exitStatus);
}

/*!
    \internal
*/
void UnixProcessBackend::unixProcessStateChanged(QProcess::ProcessState state)
{
    handleProcessStateChanged(state);
}

/*!
    \internal
*/
void UnixProcessBackend::killTimeout()
{
    if (m_process && m_process->state() == QProcess::Running)
    m_process->kill();
}

/*!
    \internal
*/
void UnixProcessBackend::readyReadStandardOutput()
{
    handleStandardOutput(m_process->readAllStandardOutput());
}

/*!
    \internal
*/
void UnixProcessBackend::readyReadStandardError()
{
    handleStandardError(m_process->readAllStandardError());
}

/*!
    \internal
 */
QString UnixProcessBackend::errorString() const
{
    return m_process->errorString();
}

#include "moc_unixprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

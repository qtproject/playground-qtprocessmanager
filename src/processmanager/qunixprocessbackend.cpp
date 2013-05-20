/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qunixprocessbackend.h"
#include "qunixsandboxprocess_p.h"
#include "qprocutils.h"
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class QUnixProcessBackend
    \brief The QUnixProcessBackend class wraps a QProcess object
    \inmodule QtProcessManager
*/

/*!
    Construct a QUnixProcessBackend with QProcessInfo \a info and optional \a parent
*/

QUnixProcessBackend::QUnixProcessBackend(const QProcessInfo &info, QObject *parent)
    : QProcessBackend(info, parent)
    , m_process(0)
{
}

/*!
  Destroy this process object.
  Any created QProcess is a child of this object, so it will be automatically terminated.
  We have to do some special processing to terminate the process group.
*/

QUnixProcessBackend::~QUnixProcessBackend()
{
    if (m_process && m_process->state() != QProcess::NotRunning)
        QProcUtils::sendSignalToProcess(m_process->pid(), SIGKILL);
}

/*!
    Returns the PID of this process. If the process has not started up yet properly, its PID will be 0.
*/
Q_PID QUnixProcessBackend::pid() const
{
    if (m_process)
        return m_process->pid();
    return QProcessBackend::pid();
}

/*!
    Return the actual process priority (if running)
*/

qint32 QUnixProcessBackend::actualPriority() const
{
    if (m_process) {
        errno = 0;   // getpriority can return -1, so we clear errno
        int result = getpriority(PRIO_PROCESS, m_process->pid());
        if (!errno)
            return result;
    }
    return QProcessBackend::actualPriority();
}

/*!
    Set the process priority to \a priority.  If the process
    is in its own process group, we fix the process priority
    of the entire group.
*/

void QUnixProcessBackend::setDesiredPriority(qint32 priority)
{
    QProcessBackend::setDesiredPriority(priority);
    if (m_process)
        QProcUtils::setPriority(m_process->pid(), priority);
}

#if defined(Q_OS_LINUX)

/*!
    Return the process oomAdjustment
*/

qint32 QUnixProcessBackend::actualOomAdjustment() const
{
    // ### TODO: What if m_process doesn't have a valid PID yet?

    if (m_process) {
        bool ok;
        qint32 result = QProcUtils::oomAdjustment(m_process->pid(), &ok);
        if (ok)
            return result;
        qWarning() << "Unable to read oom adjustment for" << m_process->pid();
    }
    return QProcessBackend::actualOomAdjustment();
}

/*!
    Set the process /proc/<pid>/oom_score_adj to \a oomAdjustment
*/

void QUnixProcessBackend::setDesiredOomAdjustment(qint32 oomAdjustment)
{
    QProcessBackend::setDesiredOomAdjustment(oomAdjustment);
    if (m_process) {
        if (!QProcUtils::setOomAdjustment(m_process->pid(), oomAdjustment))
            qWarning() << "Unable to set oom adjustment for" << m_process->pid();
    }
}

#endif // defined(Q_OS_LINUX)

/*!
    Returns the state of the process.
    The base class always returns NotRunning.
*/
QProcess::ProcessState QUnixProcessBackend::state() const
{
    return m_process ? m_process->state() : QProcess::NotRunning;
}

/*!
    Internal function to create the QProcess.
    Returns true if a process was created.
*/
bool QUnixProcessBackend::createProcess()
{
    if (m_process) {
        qWarning() << "Can't restart process!";
        return false;
    }

    qint64 uid = (m_info.contains(QProcessInfoConstants::Uid) ? m_info.uid() : -1);
    qint64 gid = (m_info.contains(QProcessInfoConstants::Gid) ? m_info.gid() : -1);
    m_process = new QUnixSandboxProcess(uid, gid, m_info.umask(), m_info.dropCapabilities(), this);

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

void QUnixProcessBackend::startProcess()
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

void QUnixProcessBackend::stop(int timeout)
{
    Q_ASSERT(m_process);

    if (m_process->state() != QProcess::NotRunning) {
        if (timeout > 0) {
            QProcUtils::sendSignalToProcess(m_process->pid(), SIGTERM);
            m_killTimer.start(timeout);
        }
        else {
            QProcUtils::sendSignalToProcess(m_process->pid(), SIGKILL);
        }
    }
}

/*!
  Writes at most \a maxSize bytes of data from \a data to the device.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
*/
qint64 QUnixProcessBackend::write(const char *data, qint64 maxSize)
{
    if (m_process)
    return m_process->write(data, maxSize);
    return 0;
}

/*!
    Override this in subclasses.  Make sure you call the parent class.
    Your subclass should emit \sa started()
*/
void QUnixProcessBackend::handleProcessStarted()
{
    if (m_info.contains(QProcessInfoConstants::Priority))
        QProcUtils::setPriority(m_process->pid(), m_info.priority());

    if (m_info.contains(QProcessInfoConstants::OomAdjustment) &&
        !QProcUtils::setOomAdjustment(m_process->pid(), m_info.oomAdjustment()))
        qWarning() << "Failed to set process oom score at startup from " << actualOomAdjustment() <<
            "to" << m_info.oomAdjustment();
}
/*!
    Override this in subclasses.  Make sure you call the parent class with \a error.
    Your subclass should emit \sa error()
*/
void QUnixProcessBackend::handleProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
}

/*!
    Override this in subclasses.  Make sure you call the parent class with \a exitCode
    and \a exitStatus.  Your subclass should emit \sa finished()
*/
void QUnixProcessBackend::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    m_killTimer.stop();
}

/*!
    Override this in subclasses.  Make sure you call the parent class with \a state.
    Your subclass should emit \sa stateChanged()
*/
void QUnixProcessBackend::handleProcessStateChanged(QProcess::ProcessState state)
{
    Q_UNUSED(state);
    m_killTimer.stop();
}

/*!
    \internal
*/
void QUnixProcessBackend::unixProcessStarted()
{
    handleProcessStarted();
}

/*!
    \internal
*/
void QUnixProcessBackend::unixProcessError(QProcess::ProcessError error)
{
    handleProcessError(error);
}

/*!
    \internal
*/
void QUnixProcessBackend::unixProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    handleProcessFinished(exitCode, exitStatus);
}

/*!
    \internal
*/
void QUnixProcessBackend::unixProcessStateChanged(QProcess::ProcessState state)
{
    handleProcessStateChanged(state);
}

/*!
    \internal
*/
void QUnixProcessBackend::killTimeout()
{
    if (m_process && m_process->state() == QProcess::Running)
    m_process->kill();
}

/*!
    \internal
*/
void QUnixProcessBackend::readyReadStandardOutput()
{
    handleStandardOutput(m_process->readAllStandardOutput());
}

/*!
    \internal
*/
void QUnixProcessBackend::readyReadStandardError()
{
    handleStandardError(m_process->readAllStandardError());
}

/*!
    \internal
 */
QString QUnixProcessBackend::errorString() const
{
    return m_process->errorString();
}

#include "moc_qunixprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

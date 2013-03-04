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

#include "processfrontend.h"
#include "processbackend.h"

#include <QDateTime>

/***************************************************************************************/

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class ProcessFrontend
    \brief The ProcessFrontend class is a generalized representation of a process.

    The ProcessFrontend class encapsulates a process whose lifecycle is controlled by the process manager.
    It is a generalized representation - in actuality, subclasses of the ProcessFrontend class are used
    to control processes.  These subclasses are used to accelerate process launching, wrap
    processes with additional arguments and settings, or fake the existence of a process.
    The ProcessFrontend class itself cannot be instantiated.

    A process object always contains a ProcessInfo object, which is the static information
    used to start and execute the process.
*/

/*!
    \property ProcessFrontend::identifier
    \brief the application identifier of the process.
*/

/*!
    \property ProcessFrontend::name
    \brief the application name of the process.
*/

/*!
    \property ProcessFrontend::program
    \brief the filename of the binary executable that is launched to start up the process.
*/

/*!
    \property ProcessFrontend::arguments
    \brief the arguments that will be passed to the program upon process startup
*/

/*!
    \property ProcessFrontend::environment
    \brief a map of the environment variables that will be used by the process
*/

/*!
    \property ProcessFrontend::workingDirectory
    \brief the directory that will be switched to before launching the process
*/

/*!
    \property ProcessFrontend::pid
    \brief the process id (PID) of the process.

    Returns 0 if the process has not been started or if this is a "fake" process.
*/

/*!
    \property ProcessFrontend::startTime
    \brief the start time of the process, measured in milliseconds since the epoch (1st Jan 1970 00:00).

    Returns 0 if process has not been started.
*/

/*!
    \property ProcessFrontend::priority
    \brief The Unix process priority (niceness).

    Returns the current process priority if the process is running.  Otherwise,
    it returns the ProcessFrontend priority setting.  You can only set the priority once the
    process is running.
*/

/*!
    \property ProcessFrontend::oomAdjustment
    \brief The Unix process /proc/<pid>/oom_score_adj (likelihood of being killed)

    Returns the current OOM adjustment score if the process is running.  Otherwise,
    it returns the ProcessFrontend OOM adjustment score setting.  You can only set the OOM adjustment
    score when the process is running.
*/

/*!
    \property ProcessFrontend::errorString
    \brief The human-readable string describing the last error.

    Returns the string describing the last error received.  This is equivalent to
    the string passed by the error() signal.
*/

/*!
    \internal
    Constructs a ProcessFrontend instance with ProcessBackend \a process and optional \a parent
    The ProcessFrontend takes ownership of the ProcessBackend.
*/
ProcessFrontend::ProcessFrontend(ProcessBackend *backend, QObject *parent)
    : QObject(parent)
    , m_startTimeSinceEpoch(0)
    , m_backend(backend)
{
    Q_ASSERT(backend);
    backend->setParent(this);
    connect(backend, SIGNAL(started()), SLOT(handleStarted()));
    connect(backend, SIGNAL(error(QProcess::ProcessError)), SLOT(handleError(QProcess::ProcessError)));
    connect(backend, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(handleFinished(int, QProcess::ExitStatus)));
    connect(backend, SIGNAL(stateChanged(QProcess::ProcessState)),
            SLOT(handleStateChanged(QProcess::ProcessState)));
    connect(backend, SIGNAL(standardOutput(const QByteArray&)),
            SLOT(handleStandardOutput(const QByteArray&)));
    connect(backend, SIGNAL(standardError(const QByteArray&)),
            SLOT(handleStandardError(const QByteArray&)));
}

/*!
  Destroy this process object.
*/

ProcessFrontend::~ProcessFrontend()
{
}

/*!
    Return the process identifier
*/

QString ProcessFrontend::identifier() const
{
    Q_ASSERT(m_backend);
    return m_backend->identifier();
}

/*!
    Return the process name
*/

QString ProcessFrontend::name() const
{
    Q_ASSERT(m_backend);
    return m_backend->name();
}

/*!
    Return the process program
*/

QString ProcessFrontend::program() const
{
    Q_ASSERT(m_backend);
    return m_backend->program();
}

/*!
    Return the process arguments
*/

QStringList ProcessFrontend::arguments() const
{
    Q_ASSERT(m_backend);
    return m_backend->arguments();
}

/*!
    Return the process environment
*/

QVariantMap ProcessFrontend::environment() const
{
    Q_ASSERT(m_backend);
    return m_backend->environment();
}

/*!
    Return the process working directory
*/

QString ProcessFrontend::workingDirectory() const
{
    Q_ASSERT(m_backend);
    return m_backend->workingDirectory();
}

/*!
    Returns the PID of this process. If the process has not started up yet properly, its PID will be 0.
*/
Q_PID ProcessFrontend::pid() const
{
    Q_ASSERT(m_backend);
    return m_backend->pid();
}

/*!
  Return the current process priority.
  This may be different than the desired process priority.
*/

qint32 ProcessFrontend::priority() const
{
    Q_ASSERT(m_backend);
    return m_backend->actualPriority();
}

/*!
    Set the process priority
*/

void ProcessFrontend::setPriority(qint32 priority)
{
    Q_ASSERT(m_backend);
    if (priority != m_backend->desiredPriority()) {
        m_backend->setDesiredPriority(priority);
        emit priorityChanged();
    }
}

/*!
    Return the process actual oomAdjustment
*/

qint32 ProcessFrontend::oomAdjustment() const
{
    Q_ASSERT(m_backend);
    return m_backend->actualOomAdjustment();
}

/*!
    Set the process desired oomAdjustment.
    This may be different than the actual process oomAdjustment
*/

void ProcessFrontend::setOomAdjustment(qint32 oomAdjustment)
{
    Q_ASSERT(m_backend);
    if (oomAdjustment != m_backend->desiredOomAdjustment()) {
        m_backend->setDesiredOomAdjustment(oomAdjustment);
        emit oomAdjustmentChanged();
    }
}

/*!
    Returns the state of the process.
    The base class always returns NotRunning.
*/
QProcess::ProcessState ProcessFrontend::state() const
{
    Q_ASSERT(m_backend);
    return m_backend->state();
}

/*!
    \brief Starts the process.

    After the process is started, the started() signal is emitted.
    If a process fails to start (e.g. because it doesn't give any output until timeout) then
    the (partly) started process is killed and error() is emitted.

    This function must be overridden in subclasses.

    \sa started()
    \sa error()
*/
void ProcessFrontend::start()
{
    if (state() == QProcess::NotRunning) {
        Q_ASSERT(m_backend);
        emit aboutToStart();
        m_startTimeSinceEpoch = QDateTime::currentMSecsSinceEpoch();
        m_backend->start();
    }
}

/*!
    Attempts to stop a process by giving it a \a timeout time to die, measured in milliseconds.

    If the process does not die in the given time limit, it is killed.

    \sa finished()
*/
void ProcessFrontend::stop(int timeout)
{
    Q_ASSERT(m_backend);
    emit aboutToStop();
    m_backend->stop(timeout);
}

/*!
  Writes at most \a maxSize bytes of data from \a data to the process.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
 */

qint64 ProcessFrontend::write(const char *data, qint64 maxSize)
{
    return m_backend->write(data, maxSize);
}

/*!
  Writes \a data from a zero-terminated string of 8-bit characters to the process.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
  This is equivalent to

\code
  ProcessFrontend::write(data, qstrlen(data));
\endcode
 */

qint64 ProcessFrontend::write(const char *data)
{
    return m_backend->write(data, qstrlen(data));
}

/*!
  Writes the content of \a byteArray to the process.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
 */

qint64 ProcessFrontend::write(const QByteArray& byteArray)
{
    return m_backend->write(byteArray.data(), byteArray.length());
}

/*!
    Returns the start time of the process, measured in milliseconds since the epoch (1st Jan 1970 00:00).
*/
qint64 ProcessFrontend::startTime() const
{
    return m_startTimeSinceEpoch;
}

/*!
    Returns the ProcessInfo object as a QVariantMap.

    If you use this method to retrieve the ProcessInfo content from the C++ side, you can
    wrap it around ProcessInfo() object for convenience.
*/
QVariantMap ProcessFrontend::processInfo() const
{
    return m_backend->processInfo().toMap();
}

/*!
  Handle a started() signal from the backend.
  The default implementation emits the started() signal.
 */
void ProcessFrontend::handleStarted()
{
    emit started();
}

/*!
  Handle an error() signal from the backend with \a processError.
  The default implementation emits the error signal.
 */
void ProcessFrontend::handleError(QProcess::ProcessError processError)
{
    emit error(processError);
}

/*!
  Handle a finished() signal from the backend with \a exitCode and \a exitStatus.
  The default implementation emits the finished() signal
 */
void ProcessFrontend::handleFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished(exitCode, exitStatus);
}

/*!
  Handle a stateChange() signal from the backend with \a state.
  The default implementation emits the stateChanged() signal
 */
void ProcessFrontend::handleStateChanged(QProcess::ProcessState state)
{
    emit stateChanged(state);
}

/*!
  Handle stdout \a data received from the process.
 */
void ProcessFrontend::handleStandardOutput(const QByteArray& data)
{
    emit standardOutput(data);
}

/*!
  Handle stderr \a data received from the process.
 */
void ProcessFrontend::handleStandardError(const QByteArray& data)
{
    emit standardError(data);
}

/*!
    Returns the backend object for this process.
*/
ProcessBackend *ProcessFrontend::backend() const
{
    return m_backend;
}


/*!
    \fn void ProcessFrontend::aboutToStart()
    This signal is emitted by start() just before the process starts running.
*/

/*!
    \fn void ProcessFrontend::aboutToStop()
    This signal is emitted by stop() just before the process is stopped.
*/

/*!
    \fn void ProcessFrontend::started()
    This signal is emitted when the proces has started successfully.
*/

/*!
    \fn void ProcessFrontend::error(QProcess::ProcessError error)
    This signal is emitted when the process has failed to start or has another \a error.
*/

/*!
    \fn void ProcessFrontend::finished(int exitCode, QProcess::ExitStatus exitStatus)
    This signal is emitted when the process has stopped and its execution is over.
    The \a exitStatus tells you how the process exited; the \a exitCode is the
    return value of the process.

    Please note:  The \a exitStatus will be QProcess::CrashExit only if the
    ProcessManager stops the process externally.  A normal process that crashes
    will have an \a exitStatus of QProcess::NormalExit with a non-zero \a exitCode.
*/

/*!
    \fn void ProcessFrontend::stateChanged(QProcess::ProcessState newState)
    This signal is emitted whenever the state of QProcess changes.  The \a newState argument
    is the state the internal QProcess changed to.
*/

/*!
    \fn void ProcessFrontend::standardOutput(const QByteArray& data)
    This signal is emitted whenever \a data is received from the stdout of the process.
*/

/*!
    \fn void ProcessFrontend::standardError(const QByteArray& data)
    This signal is emitted whenever \a data is received from the stderr of the process.
*/

/*!
    \fn void ProcessFrontend::priorityChanged()
    This signal is emitted when the process priority has been changed for a running process.
*/

/*!
    \fn void ProcessFrontend::oomAdjustmentChanged()
    This signal is emitted when the process oomAdjustment has been changed for a running process.
    Only applicable under Linux.
*/

/*!
    Returns a human-readable description of the last device error that
    occurred.
 */
QString ProcessFrontend::errorString() const
{
    return m_backend->errorString();
}

#include "moc_processfrontend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

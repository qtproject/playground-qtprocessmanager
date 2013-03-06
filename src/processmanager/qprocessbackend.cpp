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


#include "qprocessbackend.h"

#include <QDateTime>
#include <QUuid>
#include <QDebug>
#include <QFile>
#include <QByteArray>
#include <stdio.h>

/***************************************************************************************/

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class QProcessBackend
    \brief The QProcessBackend class is a generalized representation of a process.
    \inmodule QtProcessManager

    The QProcessBackend class encapsulates a process whose lifecycle is controlled by the process manager.
    It is a generalized representation - in actuality, subclasses of the QProcessBackend class are used
    to control processes.  These subclasses are used to accelerate process launching, wrap
    processes with additional arguments and settings, or fake the existence of a process.
    The QProcessBackend class itself cannot be instantiated.

    A QProcessBackend object always contains a QProcessInfo object, which is the static information
    used to start and execute the process.

    Because QProcessBackends represent actual or fake processes, they can only
    modify a few of the process properties.  Currently the QProcessBackend can
    only modify the process priority (or niceness) and the oomAdjustment score (for Linux
    systems).  All other process properties (such as UID, GID, program name) can
    be considered fixed.

    Furthermore, please note that setting these dynamic values can take time -
    for example, they may require some intra-application communication.  For example,
    if the process manager has delegated starting applications to some kind of
    pipe process, then the process manager itself may not be running with
    sufficient permissions to change the priority or oomAdjustment value to the
    desired setting.  In this case, the process manager must request that the pipe
    process make the changes, which will result in some time delay before the
    changes are real.  In this case, the actualPriority() and desiredPriority()
    functions will return different values until the IPC has completed.
*/

/*!
    \internal
    Constructs a QProcessBackend instance with QProcessInfo \a info and optional \a parent
*/
QProcessBackend::QProcessBackend(const QProcessInfo &info, QObject *parent)
    : QObject(parent)
    , m_info(info)
    , m_echo(QProcessBackend::EchoStdoutStderr)
{
    static int backend_count = 0;
    m_id = ++backend_count;
    createName();
}

/*!
  Destroy this process object.
*/

QProcessBackend::~QProcessBackend()
{
}

/*!
    Return the process name
*/

QString QProcessBackend::name() const
{
    return m_name;
}

/*!
    Return the process identifier
*/

QString QProcessBackend::identifier() const
{
    return m_info.identifier();
}

/*!
    Return the process program
*/

QString QProcessBackend::program() const
{
    return m_info.program();
}

/*!
    Return the process arguments
*/

QStringList QProcessBackend::arguments() const
{
    return m_info.arguments();
}

/*!
    Return the process environment
*/

QVariantMap QProcessBackend::environment() const
{
    return m_info.environment();
}

/*!
    Return the process working directory
*/

QString QProcessBackend::workingDirectory() const
{
    return m_info.workingDirectory();
}

/*!
    Returns a human-readable description of the last device error that
    occurred.
 */
QString QProcessBackend::errorString() const
{
    return QString();
}

/*!
    Returns the PID of this process. If the process has not started up yet properly, its PID will be 0.
*/
Q_PID QProcessBackend::pid() const
{
    return 0;
}

/*!
    Return the desired process priority.  This may not be
    the same as the actual priority
*/

qint32 QProcessBackend::desiredPriority() const
{
    return m_info.priority();
}

/*!
    Return the actual process priority.  This should be
    overridden by a subclass.
*/

qint32 QProcessBackend::actualPriority() const
{
    return 0;
}

/*!
    Set the process priority to \a priority.
    The base class updates the QProcessInfo object
    Subclasses should should override this function.
*/

void QProcessBackend::setDesiredPriority(qint32 priority)
{
    m_info.setPriority(priority);
}

/*!
    Return the desired process oomAdjustment
*/

qint32 QProcessBackend::desiredOomAdjustment() const
{
    return m_info.oomAdjustment();
}

/*!
    Return the actual process oomAdjustment
    Override this in a subclass that actual touches the oomAdjustment
*/

qint32 QProcessBackend::actualOomAdjustment() const
{
    return 0;
}

/*!
    Set the process desired \a oomAdjustment.
    The base class updates the QProcessInfo object
    Subclasses should override this function.

    Please note that there is no guarantee that the actual
    oomAdjustment will match the desired oomAdjustment.
*/

void QProcessBackend::setDesiredOomAdjustment(qint32 oomAdjustment)
{
    m_info.setOomAdjustment(oomAdjustment);
}

/*!
    \fn QProcess::ProcessState QProcessBackend::state() const

    Returns the state of the process.
*/

/*!
    \fn void QProcessBackend::start()
    \brief Starts the process.

    After the process is started, the started() signal is emitted.
    If a process fails to start (e.g. because it doesn't give any output until timeout) then
    the (partly) started process is killed and error(QProcess::FailedToStart) is emitted.

    This function must be overridden in subclasses.

    \sa started()
    \sa error()
*/

/*!
    \fn void QProcessBackend::stop(int timeout)

    Attempts to stop a process by giving it a \a timeout time to die, measured in milliseconds.
    If the process does not die in the given time limit, it is killed.

    \sa finished()
*/

/*!
   Writes at most \a maxSize bytes of data from \a data to the process.
   Returns the number of bytes that were actually written, or -1 if an error occurred.
*/

qint64 QProcessBackend::write(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

/*!
  Writes \a data from a zero-terminated string of 8-bit characters to the process.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
  This is equivalent to

\code
  ProcessFrontend::write(data, qstrlen(data));
\endcode
 */

qint64 QProcessBackend::write(const char *data)
{
    return write(data, qstrlen(data));
}

/*!
  Writes the content of \a byteArray to the process.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
 */

qint64 QProcessBackend::write(const QByteArray& byteArray)
{
    return write(byteArray.data(), byteArray.length());
}

/*!
  \enum QProcessBackend::EchoOutput

  This enum is used to control if data should be copied from an individual
  process stdout/stderr to the global stdout/stderr.  By default, it is
  set to \c QProcessBackend::EchoStdoutStderr, which means that all data
  from the child process will be copied to stdout and stderr.

  \value EchoNone         No data will be displayed
  \value EchoStdoutOnly   Data from the child's stdout will be displayed
  \value EchoStderrOnly   Data from the child's stderr will be displayed
  \value EchoStdoutStderr All data will be displayed.
 */

/*!
  Return the current echo setting.
 */

QProcessBackend::EchoOutput QProcessBackend::echo() const
{
    return m_echo;
}

/*!
  Set the \a echo setting
 */

void QProcessBackend::setEcho( QProcessBackend::EchoOutput echo )
{
    m_echo = echo;
}

/*!
  Return a copy of the current QProcessInfo
 */

QProcessInfo QProcessBackend::processInfo() const
{
    return m_info;
}

/*!
  \internal
 */

static void _writeByteArrayToFd(const QByteArray& data, const QByteArray& prefix, FILE *fh)
{
    QList<QByteArray> lines = data.split('\n');
    if (lines.isEmpty())
        return;

    // If the last item was a separator, there will be an extra blank item at the end
    if (lines.last().isEmpty())
        lines.removeLast();

    if (!lines.isEmpty()) {
        QFile f;
        f.open(fh, QIODevice::WriteOnly);
        foreach (const QByteArray& line, lines) {
            f.write(prefix);
            f.write(line);
            f.write("\n");
        }
        f.close();
    }
}

/*!
    Handler for standard output \a byteArray, read from the running process.

    Reimplement this method to provide handling for standard output.
*/
void QProcessBackend::handleStandardOutput(const QByteArray &byteArray)
{
    if (m_echo == EchoStdoutOnly || m_echo == EchoStdoutStderr) {
        QByteArray prefix = QString::fromLatin1("%1 [%2]: ").arg(m_name).arg(pid()).toLocal8Bit();
        _writeByteArrayToFd( byteArray, prefix, stderr );
    }
    emit standardOutput(byteArray);
}

/*!
    Handler for standard error \a byteArray, read from the running process.

    Reimplement this method to provide handling for standard error output.
*/
void QProcessBackend::handleStandardError(const QByteArray &byteArray)
{
    if (m_echo == EchoStderrOnly || m_echo == EchoStdoutStderr) {
        QByteArray prefix = QString::fromLatin1("%1 [%2] ERR: ").arg(m_name).arg(pid()).toLocal8Bit();
        _writeByteArrayToFd( byteArray, prefix, stderr );
    }
    emit standardError(byteArray);
}

/*!
    This function set the name of the process to be "NAME-ID".
    If you need to change the name of the process (for example, from a prelaunch backend),
    then call this function after changing the name.
 */

void QProcessBackend::createName()
{
    m_name = QString::fromLatin1("%1-%2").arg(m_info.identifier().isEmpty()
                                  ? QString::fromLatin1("process")
                                  : m_info.identifier()).arg(m_id);
}

/*!
    \fn void QProcessBackend::started()
    This signal is emitted when the process has started successfully.
*/

/*!
    \fn void QProcessBackend::error(QProcess::ProcessError error)
    This signal is emitted on a process error.  The \a error argument
    is the error emitted by the internal QProcess.
*/

/*!
    \fn void QProcessBackend::finished(int exitCode, QProcess::ExitStatus exitStatus)
    This signal is emitted when the process has stopped and its execution is over.
    The \a exitStatus tells you how the process exited; the \a exitCode is the
    return value of the process.

    Please note:  The \a exitStatus will be QProcess::CrashExit only if the
    ProcessManager stops the process externally.  A normal process that crashes
    will have an \a exitStatus of QProcess::NormalExit with a non-zero \a exitCode.
*/

/*!
    \fn void QProcessBackend::stateChanged(QProcess::ProcessState newState)
    This signal is emitted whenever the state of QProcess changes.  The \a newState argument
    is the state the internal QProcess changed to.
*/

/*!
    \fn void QProcessBackend::standardOutput(const QByteArray& data)
    This signal is emitted whenever standard output \a data is received from the child.
*/

/*!
    \fn void QProcessBackend::standardError(const QByteArray& data)
    This signal is emitted whenever standard error \a data is received from the child
*/

#include "moc_qprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

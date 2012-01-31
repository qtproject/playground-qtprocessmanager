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

#include "remoteprocessbackend.h"
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

static const QLatin1String kCommand("command");
static const QLatin1String kId("id");
static const QLatin1String kSignal("signal");

/*!
    \class RemoteProcessBackend

    \brief The RemoteProcessBackend class handles a process started by another process.

    The RemoteProcessBackend is used to control a process that was started by
    a separate "controller" process.  The RemoteProcessBackendFactory object handles
    the pipe or socket communication with the remote controller.  Communication is
    carried over serialized JSON messages.
*/

/*!
    Construct a RemoteProcessBackend with ProcessInfo \a info, \a factory,
    \a id, and optional \a parent.
*/

RemoteProcessBackend::RemoteProcessBackend(const ProcessInfo& info,
                                           RemoteProcessBackendFactory *factory,
                                           int id,
                                           QObject *parent)
    : ProcessBackend(info, parent)
    , m_factory(factory)
    , m_state(QProcess::NotRunning)
    , m_pid(-1)
    , m_id(id)
{
    Q_ASSERT(factory);
}

/*!
  Destroy this process object.
*/

RemoteProcessBackend::~RemoteProcessBackend()
{
    if (m_factory)
        m_factory->backendDestroyed(m_id);
}

/*!
    Returns the PID of this process if it is starting or running.
    Return the default value if it is not.
*/

Q_PID RemoteProcessBackend::pid() const
{
    if (m_pid != -1)
        return m_pid;
    return ProcessBackend::pid();
}

/*!
    Return the actual process priority (if running)
*/

qint32 RemoteProcessBackend::actualPriority() const
{
    if (m_pid != -1) {
        errno = 0;   // getpriority can return -1, so we clear errno
        int result = getpriority(PRIO_PROCESS, m_pid);
        if (!errno)
            return result;
    }
    return ProcessBackend::actualPriority();
}

/*!
    Set the process priority to \a priority
*/

void RemoteProcessBackend::setDesiredPriority(qint32 priority)
{
    ProcessBackend::setDesiredPriority(priority);
    if (m_factory && m_pid != -1) {
        QJsonObject object;
        object.insert(kCommand, QLatin1String("set"));
        object.insert(kId, m_id);
        object.insert("key", QLatin1String("priority"));
        object.insert("value", priority);
        m_factory->send(object);
    }
}

#if defined(Q_OS_LINUX)

/*!
    Return the process oomAdjustment
*/

qint32 RemoteProcessBackend::actualOomAdjustment() const
{
    if (m_pid != -1) {
        // ### TODO: Read correctly from /proc/<pid>/oom_score_adj
    }
    return ProcessBackend::actualOomAdjustment();
}

/*!
    Set the process /proc/<pid>/oom_score_adj to \a oomAdjustment
*/

void RemoteProcessBackend::setDesiredOomAdjustment(qint32 oomAdjustment)
{
    ProcessBackend::setDesiredOomAdjustment(oomAdjustment);
    if (m_factory && m_pid != -1) {
        QJsonObject object;
        object.insert(kCommand, QLatin1String("set"));
        object.insert(kId, m_id);
        object.insert("key", QLatin1String("oomAdjustment"));
        object.insert("value", oomAdjustment);
        m_factory->send(object);
    }
}

#endif // defined(Q_OS_LINUX)


/*!
    Returns the state of the process.
*/
QProcess::ProcessState RemoteProcessBackend::state() const
{
    return m_state;
}

/*!
    Start the process running
*/

void RemoteProcessBackend::start()
{
    if (m_factory) {
        QJsonObject object;
        object.insert(kCommand, QLatin1String("start"));
        object.insert(kId, m_id);
        object.insert(QLatin1String("info"), QJsonValue::fromVariant(m_info.toMap()));
        m_factory->send(object);
    }
}

/*!
    Attempts to stop a process by giving it a \a timeout time to die, measured in milliseconds.
    If the process does not die in the given time limit, it is killed.
    \sa finished()
*/

void RemoteProcessBackend::stop(int timeout)
{
    if (m_factory) {
        QJsonObject object;
        object.insert(kCommand, QLatin1String("stop"));
        object.insert(kId, m_id);
        object.insert(QLatin1String("timeout"), timeout);
    }
}

/*!
  Writes at most \a maxSize bytes of data from \a data to the device.
  Returns the number of bytes that were actually written, or -1 if an error occurred.

  This function isn't quite correct, because we have a JSON encoding problem.
  For the moment we'll pretend that any data sent is actually something in
  standard ASCII.
*/
qint64 RemoteProcessBackend::write(const char *data, qint64 maxSize)
{
    if (m_factory) {
        QJsonObject object;
        object.insert(kCommand, QLatin1String("write"));
        object.insert(kId, m_id);
        object.insert(QLatin1String("data"), QString::fromLocal8Bit(data, maxSize));
        if (m_factory->send(object))
            return maxSize;
    }
    return -1;
}


/*!
  Message received from the remote.
  The \a message is encoded in JSON format.
*/

void RemoteProcessBackend::receive(const QJsonObject& message)
{
    QString event = message.value("event").toString();
    qDebug() << Q_FUNC_INFO << message;
    if (event == "started") {
        m_pid = message.value("pid").toDouble();
        emit started();
    }
    else if (event == "error") {
        emit error(static_cast<QProcess::ProcessError>(message.value("error").toDouble()));
    }
    else if (event == "finished") {
        emit finished(message.value("exitCode").toDouble(),
                      static_cast<QProcess::ExitStatus>(message.value("exitStatus").toDouble()));
    }
    else if (event == "stateChanged") {
        m_state = static_cast<QProcess::ProcessState>(message.value("stateChanged").toDouble());
        emit stateChanged(m_state);
    }
    else if (event == "output") {
        if (message.contains("stdout")) {
            handleStandardOutput(message.value("stdout").toString().toLocal8Bit());
        }
        if (message.contains("stderr")) {
            handleStandardError(message.value("stderr").toString().toLocal8Bit());
        }
    }
    else
        qDebug() << Q_FUNC_INFO << "unrecognized message";
}

/*!
    \internal
*/

void RemoteProcessBackend::killTimeout()
{
    if (m_factory) {
        QJsonObject object;
        object.insert(kCommand, kSignal);
        object.insert(kId, m_id);
        object.insert(kSignal, SIGKILL);
        m_factory->send(object);
    }
}

/*!
    \internal
*/

void RemoteProcessBackend::factoryDestroyed()
{
    m_factory = NULL;
    if (m_state != QProcess::NotRunning) {
        m_state = QProcess::NotRunning;
        emit error(QProcess::UnknownError);
    }
}

#include "moc_remoteprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

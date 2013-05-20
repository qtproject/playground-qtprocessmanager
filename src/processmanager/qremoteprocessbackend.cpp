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

#include "qremoteprocessbackend.h"
#include "qremoteprotocol.h"
#include "qprocutils.h"
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class QRemoteProcessBackend

    \brief The QRemoteProcessBackend class handles a process started by another process.
    \inmodule QtProcessManager

    The QRemoteProcessBackend is used to control a process that was started by
    a separate "controller" process.  The QRemoteProcessBackendFactory object handles
    the pipe or socket communication with the remote controller.  Communication is
    carried over serialized JSON messages.
*/

/*!
    Construct a QRemoteProcessBackend with QProcessInfo \a info, \a factory,
    \a id, and optional \a parent.
*/

QRemoteProcessBackend::QRemoteProcessBackend(const QProcessInfo& info,
                                           QRemoteProcessBackendFactory *factory,
                                           int id,
                                           QObject *parent)
    : QProcessBackend(info, parent)
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

QRemoteProcessBackend::~QRemoteProcessBackend()
{
    if (m_factory)
        m_factory->backendDestroyed(m_id);
}

/*!
    Returns the PID of this process if it is starting or running.
    Return the default value if it is not.
*/

Q_PID QRemoteProcessBackend::pid() const
{
    if (m_pid != -1)
        return m_pid;
    return QProcessBackend::pid();
}

/*!
    Return the actual process priority (if running)
*/

qint32 QRemoteProcessBackend::actualPriority() const
{
    if (m_pid != -1) {
        errno = 0;   // getpriority can return -1, so we clear errno
        int result = getpriority(PRIO_PROCESS, m_pid);
        if (!errno)
            return result;
    }
    return QProcessBackend::actualPriority();
}

/*!
    Set the process priority to \a priority
*/

void QRemoteProcessBackend::setDesiredPriority(qint32 priority)
{
    QProcessBackend::setDesiredPriority(priority);
    if (m_factory && m_pid != -1) {
        QJsonObject object;
        object.insert(QRemoteProtocol::command(), QRemoteProtocol::set());
        object.insert(QRemoteProtocol::id(), m_id);
        object.insert(QRemoteProtocol::key(), QRemoteProtocol::priority());
        object.insert(QRemoteProtocol::value(), priority);
        m_factory->send(object);
    }
}

#if defined(Q_OS_LINUX)

/*!
    Return the process oomAdjustment
*/

qint32 QRemoteProcessBackend::actualOomAdjustment() const
{
    if (m_pid != -1) {
        bool ok;
        qint32 result = QProcUtils::oomAdjustment(m_pid, &ok);
        if (ok)
            return result;
        qWarning() << "Unable to read oom adjustment for" << m_pid;
    }
    return QProcessBackend::actualOomAdjustment();
}

/*!
    Set the process /proc/<pid>/oom_score_adj to \a oomAdjustment
*/

void QRemoteProcessBackend::setDesiredOomAdjustment(qint32 oomAdjustment)
{
    QProcessBackend::setDesiredOomAdjustment(oomAdjustment);
    if (m_factory && m_pid != -1) {
        QJsonObject object;
        object.insert(QRemoteProtocol::command(), QRemoteProtocol::set());
        object.insert(QRemoteProtocol::id(), m_id);
        object.insert(QRemoteProtocol::key(), QRemoteProtocol::oomAdjustment());
        object.insert(QRemoteProtocol::value(), oomAdjustment);
        m_factory->send(object);
    }
}

#endif // defined(Q_OS_LINUX)


/*!
    Returns the state of the process.
*/
QProcess::ProcessState QRemoteProcessBackend::state() const
{
    return m_state;
}

/*!
    Start the process running
*/

void QRemoteProcessBackend::start()
{
    if (m_factory) {
        QJsonObject object;
        object.insert(QRemoteProtocol::command(), QRemoteProtocol::start());
        object.insert(QRemoteProtocol::id(), m_id);
        object.insert(QRemoteProtocol::info(), QJsonValue::fromVariant(m_info.toMap()));
        m_factory->send(object);
    }
}

/*!
    Attempts to stop a process by giving it a \a timeout time to die, measured in milliseconds.
    If the process does not die in the given time limit, it is killed.
    \sa finished()
*/

void QRemoteProcessBackend::stop(int timeout)
{
    if (m_factory) {
        QJsonObject object;
        object.insert(QRemoteProtocol::command(), QRemoteProtocol::stop());
        object.insert(QRemoteProtocol::id(), m_id);
        object.insert(QRemoteProtocol::timeout(), timeout);
        m_factory->send(object);
    }
}

/*!
  Writes at most \a maxSize bytes of data from \a data to the device.
  Returns the number of bytes that were actually written, or -1 if an error occurred.
*/
qint64 QRemoteProcessBackend::write(const char *data, qint64 maxSize)
{
    if (m_factory) {
        QJsonObject object;
        object.insert(QRemoteProtocol::command(), QRemoteProtocol::write());
        object.insert(QRemoteProtocol::id(), m_id);
        object.insert(QRemoteProtocol::data(), QString::fromLatin1(QByteArray(data, maxSize).toBase64()));
        if (m_factory->send(object))
            return maxSize;
    }
    return -1;
}

/*!
   Returns the most recent error string
*/
QString QRemoteProcessBackend::errorString() const
{
    return m_errorString;
}


/*!
  Message received from the remote.
  The \a message is encoded in JSON format.
*/

void QRemoteProcessBackend::receive(const QJsonObject& message)
{
    QString event = message.value(QRemoteProtocol::event()).toString();
    if (event == QRemoteProtocol::started()) {
        m_pid = message.value(QRemoteProtocol::pid()).toDouble();
        emit started();
    }
    else if (event == QRemoteProtocol::error()) {
        m_errorString = message.value(QRemoteProtocol::errorString()).toString();
        emit error(static_cast<QProcess::ProcessError>(message.value(QRemoteProtocol::error()).toDouble()));
    }
    else if (event == QRemoteProtocol::finished()) {
        emit finished(message.value(QRemoteProtocol::exitCode()).toDouble(),
                      static_cast<QProcess::ExitStatus>(message.value(QRemoteProtocol::exitStatus()).toDouble()));
    }
    else if (event == QRemoteProtocol::stateChanged()) {
        m_state = static_cast<QProcess::ProcessState>(message.value(QRemoteProtocol::stateChanged()).toDouble());
        emit stateChanged(m_state);
    }
    else if (event == QRemoteProtocol::output()) {
        if (message.contains(QRemoteProtocol::standardout())) {
            handleStandardOutput(message.value(QRemoteProtocol::standardout()).toString().toLocal8Bit());
        }
        if (message.contains(QRemoteProtocol::standarderror())) {
            handleStandardError(message.value(QRemoteProtocol::standarderror()).toString().toLocal8Bit());
        }
    }
    else
        qDebug() << Q_FUNC_INFO << "unrecognized message" << message;
}

/*!
    \internal
*/

void QRemoteProcessBackend::killTimeout()
{
    if (m_factory) {
        QJsonObject object;
        object.insert(QRemoteProtocol::command(), QRemoteProtocol::signal());
        object.insert(QRemoteProtocol::id(), m_id);
        object.insert(QRemoteProtocol::signal(), SIGKILL);
        m_factory->send(object);
    }
}

/*!
    \internal
*/

void QRemoteProcessBackend::factoryDestroyed()
{
    m_factory = NULL;
    if (m_state != QProcess::NotRunning) {
        m_state = QProcess::NotRunning;
        emit error(QProcess::UnknownError);
    }
}

#include "moc_qremoteprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

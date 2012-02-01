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

#include "prelaunchprocessbackend.h"

#include <qjsondocument.h>
#include <QUuid>
#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class PrelaunchProcessBackend
  \brief The PrelaunchProcessBackend class encapsulates a process that is started and later told what to run.

  The PrelaunchProcessBackend class contains an internal QProcess object that is
  "prelaunched" by the PrelaunchProcessBackendFactory object.  When the factory
  is asked to create a new process, the prelaunched backend is returned.  When start() is
  called, the prelaunched process is passed a ProcessInfo record encoded in QBinaryJson
  document format (a serialized JSON object).  This record should be used by the
  prelaunched process to transform itself into the correctly running application.
 */

/*!
  Construct a PrelaunchProcessBackend with ProcessInfo \a info and optional \a parent.
  The \a info ProcessInfo is used to start the internal QProcess.  This is different
  from the final ProcessInfo which will be directly passed to the running QProcess.
 */

PrelaunchProcessBackend::PrelaunchProcessBackend(const ProcessInfo &info, QObject *parent)
    : UnixProcessBackend(info, parent)
    , m_started(false)
{
}

/*!
  Destroy this and child objects
*/

PrelaunchProcessBackend::~PrelaunchProcessBackend()
{
}

/*!
  Starts the internal QProcess with the prelaunch information.
 */

void PrelaunchProcessBackend::prestart()
{
    if (createProcess())
        startProcess();
}

/*!
  Switch the stored ProcessInfo to the final \a info object.
  This function also updates the identifier of the process.
 */

void PrelaunchProcessBackend::setInfo(const ProcessInfo& info)
{
    m_info = info;
    setDesiredPriority(m_info.priority());
    setDesiredOomAdjustment(m_info.oomAdjustment());
    createName();
}

/*!
  Check to see if this prelaunched process is ready to be used
 */

bool PrelaunchProcessBackend::isReady() const
{
    return m_process && m_process->state() == QProcess::Running;
}

/*!
  Pretend to start the prelaunched process.
  The stored ProcessInfo record is written to the internal QProcess as a serialized
  JSON object (the internal QProcess receives it from stdin).
  Then emit all queued signals that were captured from the QProcess.
 */

void PrelaunchProcessBackend::start()
{
    if (m_started) {
        qWarning() << "Can't restart prelaunched process";
        return;
    }

    m_started = true;
    // Pass the actual process info to the child process
    QByteArray byteArray = QJsonDocument::fromVariant(m_info.toMap()).toBinaryData();
    write(byteArray.data(), byteArray.size());

    while (m_queue.size()) {
        QueuedSignal s = m_queue.takeFirst();
        switch (s.name) {
        case QueuedSignal::StateChanged:
            emit stateChanged(s.n.state);
            break;
        case QueuedSignal::Started:
            emit started();
            break;
        case QueuedSignal::Error:
            emit error(s.n.error);
            break;
        case QueuedSignal::Finished:
            emit finished(s.n.f.exitCode, s.n.f.exitStatus);
            break;
        }
    }
}

/*!
  Return the current process state.  Until the process has been
  officially "started", the state has to be NotRunning.
 */

QProcess::ProcessState PrelaunchProcessBackend::state() const
{
    if (m_started)
        return UnixProcessBackend::state();
    else
        return QProcess::NotRunning;
}

/*!
    \internal
*/
void PrelaunchProcessBackend::handleProcessStarted()
{
    UnixProcessBackend::handleProcessStarted();
    if (m_started)
        emit started();
    else {
        QueuedSignal s;
        s.name = QueuedSignal::Started;
        m_queue << s;
    }
}

/*!
    \internal
*/
void PrelaunchProcessBackend::handleProcessError(QProcess::ProcessError processError)
{
    UnixProcessBackend::handleProcessError(processError);
    if (m_started)
        emit error(processError);
    else {
        QueuedSignal s;
        s.name = QueuedSignal::Error;
        s.n.error = processError;
        m_queue << s;
    }
}

/*!
    \internal
*/
void PrelaunchProcessBackend::handleProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    UnixProcessBackend::handleProcessFinished(exitCode, status);
    if (m_started)
        emit finished(exitCode, status);
    else {
        QueuedSignal s;
        s.name = QueuedSignal::Finished;
        s.n.f.exitCode = exitCode;
        s.n.f.exitStatus = status;
        m_queue << s;
    }
}

/*!
    \internal
*/
void PrelaunchProcessBackend::handleProcessStateChanged(QProcess::ProcessState state)
{
    UnixProcessBackend::handleProcessStateChanged(state);
    if (m_started)
        emit stateChanged(state);
    else {
        QueuedSignal s;
        s.name = QueuedSignal::StateChanged;
        s.n.state = state;
        m_queue << s;
    }
}

/*!
  \class QueuedSignal
  \internal
 */

/*!
  \enum QueuedSignal::SignalName
  \internal
 */

#include "moc_prelaunchprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

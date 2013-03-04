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

#include "processmanager.h"
#include "processfrontend.h"
#include "processbackendfactory.h"
#include "processbackendmanager.h"
#include "processbackend.h"

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class ProcessManager
  \brief The ProcessManager class encapsulates ways of creating and tracking processes.
*/

/*!
    \property ProcessManager::memoryRestricted
    \brief the current setting of memory restriction
*/

/*!
    \property ProcessManager::idleDelegate
    \brief The IdleDelegate object assigned to this factory.
*/

/*!
  Construct a ProcessManager with an optional \a parent
*/

ProcessManager::ProcessManager(QObject *parent)
    : QObject(parent)
{
    m_backend = new ProcessBackendManager(this);
    connect(m_backend, SIGNAL(internalProcessesChanged()), SIGNAL(internalProcessesChanged()));
    connect(m_backend, SIGNAL(internalProcessError(QProcess::ProcessError)),
            SIGNAL(internalProcessError(QProcess::ProcessError)));
}

/*!
  Delete the ProcessManager and terminate all processes
  The ProcessManager is the parent of all factories and process frontends.
*/

ProcessManager::~ProcessManager()
{
}

/*!
  Create a new ProcessFrontend based on ProcessInfo \a info
  The parent of the ProcessFrontend is the ProcessManager.
*/

ProcessFrontend *ProcessManager::create(const ProcessInfo& info)
{
    ProcessBackend *backend = m_backend->create(info);
    if (!backend)
        return NULL;
    ProcessFrontend *frontend = createFrontend(backend);
    frontend->setParent(this);
    m_processlist.append(frontend);
    connect(frontend, SIGNAL(aboutToStart()), SLOT(processFrontendAboutToStart()));
    connect(frontend, SIGNAL(aboutToStop()), SLOT(processFrontendAboutToStop()));
    connect(frontend, SIGNAL(started()), SLOT(processFrontendStarted()));
    connect(frontend, SIGNAL(error(QProcess::ProcessError)), SLOT(processFrontendError(QProcess::ProcessError)));
    connect(frontend, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(processFrontendFinished(int, QProcess::ExitStatus)));
    connect(frontend, SIGNAL(stateChanged(QProcess::ProcessState)),
            SLOT(processFrontendStateChanged(QProcess::ProcessState)));
    connect(frontend, SIGNAL(destroyed()), SLOT(processFrontendDestroyed()));
    return frontend;
}

/*!
  Create a new ProcessFrontend based on QVariantMap \a info
  (which should look a lot like ProcessInfo).
*/

ProcessFrontend *ProcessManager::create(const QVariantMap& info)
{
    return create(ProcessInfo(info));
}

/*!
  Create a new ProcessFrontend for a \a backend.
  Override this function if you need to subclass ProcessFrontend to
  store additional process information or functions.
*/

ProcessFrontend *ProcessManager::createFrontend(ProcessBackend *backend)
{
    return new ProcessFrontend(backend);
}

/*!
    Returns a process that matches the name \a name.

    Each process started with the ProcessManager has uniqut application name.
    If the method returns 0,
    the name is either invalid or the process has been killed.
*/
ProcessFrontend *ProcessManager::processForName(const QString &name) const
{
    foreach (ProcessFrontend *frontend, m_processlist)
    if (frontend->name() == name)
        return frontend;
    return NULL;
}

/*!
    Returns a process that matches the process id \a pid. Returns NULL if no such process was found.
    Returns NULL if \a pid is 0.
    If more than one process matches the same ID, this method will return the first one find.
*/

ProcessFrontend *ProcessManager::processForPID(qint64 pid) const
{
    if (!pid)
        return NULL;

    foreach (ProcessFrontend *frontend, m_processlist)
    if (frontend->pid() == pid)
        return frontend;
    return NULL;
}

/*!
    Returns the number of processes.
*/

int ProcessManager::size() const
{
    return m_processlist.size();
}

/*!
  Add a ProcessBackendFactory \a factory to the end of the Factory list.
*/

void ProcessManager::addBackendFactory(ProcessBackendFactory *factory)
{
    m_backend->addFactory(factory);
}

/*!
  Return a list of current process names
*/

QStringList ProcessManager::names() const
{
    QStringList result;
    foreach (ProcessFrontend *frontend, m_processlist)
        result << frontend->name();
    return result;
}

/*!
  Return a list of all internal processes being used by factories
*/

PidList ProcessManager::internalProcesses() const
{
    return m_backend->internalProcesses();
}

/*!
  Set memory restrictions.  If \a memoryRestricted is true
  all factories are requested to minimize memory use.
*/

void ProcessManager::setMemoryRestricted(bool memoryRestricted)
{
    m_backend->setMemoryRestricted(memoryRestricted);
    emit memoryRestrictedChanged();
}

/*!
  Return current memory restriction setting
*/

bool ProcessManager::memoryRestricted() const
{
    return m_backend->memoryRestricted();
}

/*!
  Set the backend idle delegate.
*/

void ProcessManager::setIdleDelegate(IdleDelegate *idleDelegate)
{
    m_backend->setIdleDelegate(idleDelegate);
    emit idleDelegateChanged();
}

/*!
  Return current idle restriction setting
*/

IdleDelegate * ProcessManager::idleDelegate() const
{
    return m_backend->idleDelegate();
}

/*!
  Raise the processAboutToStart() signal.
*/

void ProcessManager::processFrontendAboutToStart()
{
}

/*!
  Raise the processAboutToStop() signal.
*/

void ProcessManager::processFrontendAboutToStop()
{
}

/*!
  Raise the processStarted() signal.
*/

void ProcessManager::processFrontendStarted()
{
}

/*!
  Raise the processError() signal.
  Pass through the \a error value.
*/

void ProcessManager::processFrontendError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
}

/*!
  Raise the processFinished() signal.  Pass through the
  \a exitCode and \a exitStatus values.
*/

void ProcessManager::processFrontendFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}

/*!
  Raise the processStateChanged() signal.  Pass through the
  \a state value.
*/

void ProcessManager::processFrontendStateChanged(QProcess::ProcessState state)
{
    Q_UNUSED(state);
}

/*!
  Track a process being destroyed.  Remote this process from our
  list of valid process.  Also emit the processDestroyed() signal.
  If you override this function in a subclass, be sure to call
  the parent function.
*/

void ProcessManager::processFrontendDestroyed()
{
    ProcessFrontend *frontend = static_cast<ProcessFrontend *>(sender());
    if (frontend)
        m_processlist.removeAll(frontend);
}

/*!
    \fn void ProcessManager::memoryRestrictedChanged()
    This signal is emitted when the memory restriction is changed
*/

/*!
    \fn void ProcessManager::idleDelegateChanged()
    This signal is emitted when the idle delegate is changed
*/

/*!
  \fn void ProcessManager::internalProcessesChanged()
  This signal is emitted when the list of internal processes changes.
*/

/*!
  \fn void ProcessManager::internalProcessError(QProcess::ProcessError error)
  This signal is emitted when an internal process has an \a error.
*/


#include "moc_processmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

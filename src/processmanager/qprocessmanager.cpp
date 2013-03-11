/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qprocessmanager.h"
#include "qprocessfrontend.h"
#include "qprocessbackendfactory.h"
#include "qprocessbackendmanager.h"
#include "qprocessbackend.h"

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QProcessManager
  \brief The QProcessManager class encapsulates ways of creating and tracking processes.
  \inmodule QtProcessManager
*/

/*!
    \property QProcessManager::memoryRestricted
    \brief the current setting of memory restriction
*/

/*!
    \property QProcessManager::idleDelegate
    \brief The IdleDelegate object assigned to this factory.
*/

/*!
  Construct a QProcessManager with an optional \a parent
*/

QProcessManager::QProcessManager(QObject *parent)
    : QObject(parent)
{
    m_backend = new QProcessBackendManager(this);
    connect(m_backend, SIGNAL(internalProcessesChanged()), SIGNAL(internalProcessesChanged()));
    connect(m_backend, SIGNAL(internalProcessError(QProcess::ProcessError)),
            SIGNAL(internalProcessError(QProcess::ProcessError)));
}

/*!
  Delete the QProcessManager and terminate all processes
  The QProcessManager is the parent of all factories and process frontends.
*/

QProcessManager::~QProcessManager()
{
}

/*!
  Create a new QProcessFrontend based on QProcessInfo \a info
  The parent of the QProcessFrontend is the QProcessManager.
*/

QProcessFrontend *QProcessManager::create(const QProcessInfo& info)
{
    QProcessBackend *backend = m_backend->create(info);
    if (!backend)
        return NULL;
    QProcessFrontend *frontend = createFrontend(backend);
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
  Create a new QProcessFrontend based on QVariantMap \a info
  (which should look a lot like QProcessInfo).
*/

QProcessFrontend *QProcessManager::create(const QVariantMap& info)
{
    return create(QProcessInfo(info));
}

/*!
  Create a new QProcessFrontend for a \a backend.
  Override this function if you need to subclass QProcessFrontend to
  store additional process information or functions.
*/

QProcessFrontend *QProcessManager::createFrontend(QProcessBackend *backend)
{
    return new QProcessFrontend(backend);
}

/*!
    Returns a process that matches the name \a name.

    Each process started with the QProcessManager has uniqut application name.
    If the method returns 0,
    the name is either invalid or the process has been killed.
*/
QProcessFrontend *QProcessManager::processForName(const QString &name) const
{
    foreach (QProcessFrontend *frontend, m_processlist)
    if (frontend->name() == name)
        return frontend;
    return NULL;
}

/*!
    Returns a process that matches the process id \a pid. Returns NULL if no such process was found.
    Returns NULL if \a pid is 0.
    If more than one process matches the same ID, this method will return the first one find.
*/

QProcessFrontend *QProcessManager::processForPID(qint64 pid) const
{
    if (!pid)
        return NULL;

    foreach (QProcessFrontend *frontend, m_processlist)
    if (frontend->pid() == pid)
        return frontend;
    return NULL;
}

/*!
    Returns the number of processes.
*/

int QProcessManager::size() const
{
    return m_processlist.size();
}

/*!
  Add a QProcessBackendFactory \a factory to the end of the Factory list.
*/

void QProcessManager::addBackendFactory(QProcessBackendFactory *factory)
{
    m_backend->addFactory(factory);
}

/*!
  Return a list of current process names
*/

QStringList QProcessManager::names() const
{
    QStringList result;
    foreach (QProcessFrontend *frontend, m_processlist)
        result << frontend->name();
    return result;
}

/*!
  Return a list of all internal processes being used by factories
*/

QPidList QProcessManager::internalProcesses() const
{
    return m_backend->internalProcesses();
}

/*!
  Set memory restrictions.  If \a memoryRestricted is true
  all factories are requested to minimize memory use.
*/

void QProcessManager::setMemoryRestricted(bool memoryRestricted)
{
    m_backend->setMemoryRestricted(memoryRestricted);
    emit memoryRestrictedChanged();
}

/*!
  Return current memory restriction setting
*/

bool QProcessManager::memoryRestricted() const
{
    return m_backend->memoryRestricted();
}

/*!
  Set the backend idle delegate.
*/

void QProcessManager::setIdleDelegate(QIdleDelegate *idleDelegate)
{
    m_backend->setIdleDelegate(idleDelegate);
    emit idleDelegateChanged();
}

/*!
  Return current idle restriction setting
*/

QIdleDelegate * QProcessManager::idleDelegate() const
{
    return m_backend->idleDelegate();
}

/*!
  Raise the processAboutToStart() signal.
*/

void QProcessManager::processFrontendAboutToStart()
{
}

/*!
  Raise the processAboutToStop() signal.
*/

void QProcessManager::processFrontendAboutToStop()
{
}

/*!
  Raise the processStarted() signal.
*/

void QProcessManager::processFrontendStarted()
{
}

/*!
  Raise the processError() signal.
  Pass through the \a error value.
*/

void QProcessManager::processFrontendError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
}

/*!
  Raise the processFinished() signal.  Pass through the
  \a exitCode and \a exitStatus values.
*/

void QProcessManager::processFrontendFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}

/*!
  Raise the processStateChanged() signal.  Pass through the
  \a state value.
*/

void QProcessManager::processFrontendStateChanged(QProcess::ProcessState state)
{
    Q_UNUSED(state);
}

/*!
  Track a process being destroyed.  Remote this process from our
  list of valid process.  Also emit the processDestroyed() signal.
  If you override this function in a subclass, be sure to call
  the parent function.
*/

void QProcessManager::processFrontendDestroyed()
{
    QProcessFrontend *frontend = static_cast<QProcessFrontend *>(sender());
    if (frontend)
        m_processlist.removeAll(frontend);
}

/*!
    \fn void QProcessManager::memoryRestrictedChanged()
    This signal is emitted when the memory restriction is changed
*/

/*!
    \fn void QProcessManager::idleDelegateChanged()
    This signal is emitted when the idle delegate is changed
*/

/*!
  \fn void QProcessManager::internalProcessesChanged()
  This signal is emitted when the list of internal processes changes.
*/

/*!
  \fn void QProcessManager::internalProcessError(QProcess::ProcessError error)
  This signal is emitted when an internal process has an \a error.
*/


#include "moc_qprocessmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

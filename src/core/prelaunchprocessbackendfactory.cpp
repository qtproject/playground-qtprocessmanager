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

#include <QDebug>

#include "prelaunchprocessbackendfactory.h"
#include "prelaunchprocessbackend.h"
#include "processinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class PrelaunchProcessBackendFactory
  \brief The PrelaunchProcessBackendFactory class creates PrelaunchProcessBackend objects

  The PrelaunchProcessBackendFactory starts up a PrelaunchProcessBackend using
  information passed in the constructor.
*/

/*!
  \property PrelaunchProcessBackendFactory::processInfo
  \brief ProcessInfo record used to create the prelaunched process
 */

/*!
  \property PrelaunchProcessBackendFactory::prelaunchEnabled
  \brief Controls whether or not a prelaunched process is created.

  If this property is true, a prelaunched process will be created
  after a suitable time interval.  If it is set to false and a
  prelaunched process exists, that process will be terminated.
 */

/*!
  Construct a PrelaunchProcessBackendFactory with optional \a parent.
  To be able to use the PrelaunchProcessBackendFactory, you also need to set
  a ProcessInfo object to it that specifies which process is prelaunched.
*/
PrelaunchProcessBackendFactory::PrelaunchProcessBackendFactory(QObject *parent)
    : ProcessBackendFactory(parent)
    , m_prelaunch(NULL)
    , m_info(NULL)
    , m_prelaunchEnabled(true)
{
}

/*!
   Destroy this and child objects.
*/

PrelaunchProcessBackendFactory::~PrelaunchProcessBackendFactory()
{
}

/*!
  Return true if the PrelaunchProcessBackendFactory can create
  a process that matches \a info.  The default implementation only checks that
  a valid info object has been previously set in the factory, and then
  passes the decision on to the default implementation, which should probably
  have some kind of MatchDelegate installed.
*/

bool PrelaunchProcessBackendFactory::canCreate(const ProcessInfo &info) const
{
    if (!m_info)
        return false;

    return ProcessBackendFactory::canCreate(info);
}

/*!
  Construct a PrelaunchProcessBackend from a ProcessInfo \a info record with \a parent.
*/

ProcessBackend * PrelaunchProcessBackendFactory::create(const ProcessInfo &info, QObject *parent)
{
    Q_ASSERT(m_info);
    PrelaunchProcessBackend *prelaunch = m_prelaunch;

    if (hasPrelaunchedProcess()) {
        // qDebug() << "Using existing prelaunch";
        m_prelaunch = NULL;
        prelaunch->setInfo(info);
        prelaunch->setParent(parent);
        prelaunch->disconnect(this);
        updateState();
    } else {
        // qDebug() << "Creating prelaunch from scratch";
        prelaunch = new PrelaunchProcessBackend(*m_info, parent);
        prelaunch->prestart();
        prelaunch->setInfo(info);
    }
    return prelaunch;
}

/*!
  Return the prelaunched process information
 */

ProcessInfo *PrelaunchProcessBackendFactory::processInfo() const
{
    return m_info;
}

/*!
    Returns whether prelaunching is enabled for this factory.
*/
bool PrelaunchProcessBackendFactory::prelaunchEnabled() const
{
    return m_prelaunchEnabled;
}

void PrelaunchProcessBackendFactory::setPrelaunchEnabled(bool value)
{
    if (m_prelaunchEnabled != value) {
        m_prelaunchEnabled = value;
        if (!m_prelaunchEnabled) {
            if (m_prelaunch) {
                m_prelaunch->deleteLater();
                m_prelaunch = NULL;
            }
        }
        updateState();
        emit prelaunchEnabledChanged();
    }
}

/*!
    Returns whether there is a prelaunched process which is ready to be consumed.
*/
bool PrelaunchProcessBackendFactory::hasPrelaunchedProcess() const
{
    return (m_prelaunch && m_prelaunch->isReady());
}

/*!
    Under memory restriction, terminate the prelaunch process.
*/
void PrelaunchProcessBackendFactory::handleMemoryRestrictionChange()
{
    if (m_memoryRestricted) {
        if (m_prelaunch) {
            delete m_prelaunch;   // This will kill the child process as well
            m_prelaunch = NULL;
        }
    }
    updateState();
}

/*!
    Returns the prelaunched process backend, or null if none is created.
 */
PrelaunchProcessBackend *PrelaunchProcessBackendFactory::prelaunchProcessBackend() const
{
    return m_prelaunch;
}

/*!
  Launch a new prelaunched process backend.
 */

void PrelaunchProcessBackendFactory::idleCpuAvailable()
{
    // qDebug() << Q_FUNC_INFO;
    if (m_prelaunchEnabled && !m_prelaunch && !m_memoryRestricted && m_info) {
        // qDebug() << Q_FUNC_INFO << "...launching";
        m_prelaunch = new PrelaunchProcessBackend(*m_info, this);
        connect(m_prelaunch, SIGNAL(finished(int,QProcess::ExitStatus)),
                SLOT(prelaunchFinished(int,QProcess::ExitStatus)));
        connect(m_prelaunch, SIGNAL(error(QProcess::ProcessError)),
                SLOT(prelaunchError(QProcess::ProcessError)));
        connect(m_prelaunch, SIGNAL(stateChanged(QProcess::ProcessState)),
                SLOT(updateState()));
        m_prelaunch->prestart();
        emit processPrelaunched();
    }
    updateState();
}

/*!
  Handle a surprise termination condition - the prelaunched process died
  unexpectedly.
 */

void PrelaunchProcessBackendFactory::prelaunchFinished(int exitCode, QProcess::ExitStatus status)
{
    qWarning() << Q_FUNC_INFO << "died unexpectedly" << exitCode << status;
    if (m_prelaunch) {
        m_prelaunch->deleteLater();
        m_prelaunch = NULL;
    }
    updateState();
}

/*!
  Handle surprise error conditions on the prelaunched process.
 */

void PrelaunchProcessBackendFactory::prelaunchError(QProcess::ProcessError err)
{
    qWarning() << Q_FUNC_INFO << "unexpected error" << err;
    if (m_prelaunch) {
        m_prelaunch->deleteLater();
        m_prelaunch = NULL;
    }

    if (err == QProcess::FailedToStart) {
        qWarning() << Q_FUNC_INFO << "disabling prelaunch because of process errors";
        m_prelaunchEnabled = false;
    }

    updateState();
}

/*!
   Update our presented state, consisting of whether or not we need
   idle CPU resources and how many internal processes we are running.
*/
void PrelaunchProcessBackendFactory::updateState()
{
    Q_ASSERT(!m_prelaunch || m_prelaunchEnabled);  // If prelaunch is not enabled, we must not have a prelaunch process
    Q_ASSERT(!m_prelaunch || !m_memoryRestricted);  // If memory is restricted, we must not have a prelaunch process

    setIdleCpuRequest(!m_memoryRestricted && m_prelaunchEnabled && !m_prelaunch && m_info);

    PidList list;
    if (m_prelaunch && m_prelaunch->isReady())
        list << m_prelaunch->pid();
    setInternalProcesses(list);
}

/*!
    Sets the ProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
    An internal copy is made of the \a processInfo object.
 */
void PrelaunchProcessBackendFactory::setProcessInfo(ProcessInfo *processInfo)
{
    if (m_info != processInfo) {
        if (m_info) {
            delete m_info;
            m_info = NULL;
        }

        if (processInfo) {
            m_info = new ProcessInfo(*processInfo);
            m_info->setParent(this);
            if (m_prelaunch) {
                m_prelaunch->deleteLater();
                m_prelaunch = NULL;
            }
        }
        updateState();
        emit processInfoChanged();
    }
}

/*!
    Sets the ProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
 */
void PrelaunchProcessBackendFactory::setProcessInfo(ProcessInfo& processInfo)
{
    setProcessInfo(&processInfo);
}


/*!
  \fn void PrelaunchProcessBackendFactory::processInfoChanged()
  This signal is emitted when the internal ProcessInfo record is
  changed.
 */
/*!
  \fn void PrelaunchProcessBackendFactory::prelaunchEnabledChanged()
  This signal is emitted when the prelaunchEnabled property is changed.
 */
/*!
  \fn void PrelaunchProcessBackendFactory::processPrelaunched()
  This signal is emitted when the prelaunched process is first created.
 */

#include "moc_prelaunchprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

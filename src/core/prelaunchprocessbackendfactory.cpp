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

const int kPrelaunchTimerInterval = 1000;

/*!
  \class PrelaunchProcessBackendFactory
  \brief The PrelaunchProcessBackendFactory class creates PrelaunchProcessBackend objects

  The PrelaunchProcessBackendFactory starts up a PrelaunchProcessBackend using
  information passed in the constructor.
*/

/*!
  \property PrelaunchProcessBackendFactory::launchInterval
  \brief Time in milliseconds before a new prelaunch backend will be started
 */

/*!
  Construct a PrelaunchProcessBackendFactory with optional \a parent.
  To be able to use the PrelaunchProcessBackendFactory, you also need to set
  a ProcessInfo object to it that specifies which process is prelaunched.
*/
PrelaunchProcessBackendFactory::PrelaunchProcessBackendFactory(QObject *parent)
    : ProcessBackendFactory(parent)
    , m_prelaunch(NULL)
    , m_info(0)
    , m_prelaunchEnabled(true)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    m_timer.setSingleShot(true);
    m_timer.setInterval(kPrelaunchTimerInterval);
}

/*!
   Destroy this and child objects.
*/

PrelaunchProcessBackendFactory::~PrelaunchProcessBackendFactory()
{
}

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
        startPrelaunchTimer();
        prelaunch->setInfo(info);
        prelaunch->setParent(parent);
        prelaunch->disconnect(this);
    } else {
        // qDebug() << "Creating prelaunch from scratch";
        prelaunch = new PrelaunchProcessBackend(*m_info, parent);
        prelaunch->prestart();
        prelaunch->setInfo(info);
    }
    return prelaunch;
}

/*!
  If there is a prelaunched process running, it will be return here.
 */

QList<Q_PID> PrelaunchProcessBackendFactory::internalProcesses()
{
    QList<Q_PID> list;
    if (m_prelaunch && m_prelaunch->isReady())
        list << m_prelaunch->pid();
    return list;
}

ProcessInfo *PrelaunchProcessBackendFactory::processInfo() const
{
    return m_info;
}

/*!
  Return the current launch interval in milliseconds
 */

int PrelaunchProcessBackendFactory::launchInterval() const
{
    return m_timer.interval();
}

/*!
  Set the current launch interval to \a interval milliseconds
*/

void PrelaunchProcessBackendFactory::setLaunchInterval(int interval)
{
    if (m_timer.interval() != interval) {
        bool active = m_timer.isActive();
        m_timer.stop();
        m_timer.setInterval(interval);
        if (active)
            startPrelaunchTimer();
        emit launchIntervalChanged();
    }
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
            m_timer.stop();
        } else {
            startPrelaunchTimer();
        }
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
        m_timer.stop();
        if (m_prelaunch) {
            delete m_prelaunch;   // This will kill the child process as well
            m_prelaunch = NULL;
        }
    } else {
        Q_ASSERT(m_prelaunch == NULL);
        startPrelaunchTimer();
    }
}

/*!
  Launch a new prelaunched process backend.
  In the future, it would be useful if the launch didn't require a timeout.
 */

void PrelaunchProcessBackendFactory::timeout()
{
    Q_ASSERT(m_prelaunch == NULL);
    Q_ASSERT(!m_memoryRestricted);
    Q_ASSERT(m_info);

    m_prelaunch = new PrelaunchProcessBackend(*m_info, this);
    connect(m_prelaunch, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(prelaunchFinished(int,QProcess::ExitStatus)));
    m_prelaunch->prestart();
}

/*!
  Handle a surprise termination condition - the prelaunched process died
  unexpectedly.
 */

void PrelaunchProcessBackendFactory::prelaunchFinished(int exitCode, QProcess::ExitStatus status)
{
    qWarning() << Q_FUNC_INFO << "died unexpectedly" << exitCode << status;
    if (m_prelaunch) {
        delete m_prelaunch;
        m_prelaunch = NULL;
    }
    startPrelaunchTimer();
}

/*!
    Starts the prelaunch timer only if prelaunching is enabled.
*/
void PrelaunchProcessBackendFactory::startPrelaunchTimer()
{
    if (m_prelaunchEnabled)
        m_timer.start();
}

/*!
    Sets the ProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
    Takes ownership of the object. If the ProcessInfo object is changed, the old object is deleted.
 */
void PrelaunchProcessBackendFactory::setProcessInfo(ProcessInfo *processInfo)
{
    if (m_info != processInfo) {
        delete m_info;
        m_info = processInfo;

        if (m_info) {
            m_info->setParent(this);
            startPrelaunchTimer();
        } else {
            m_timer.stop();
        }
        emit processInfoChanged();
    }
}

/*!
  \fn void PrelaunchProcessBackendFactory::launchIntervalChanged()
  This signal is emitted when the launchInterval is changed.
 */

#include "moc_prelaunchprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

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
#include "cpuload.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kPrelaunchCpuPollingInterval = 1000;
const int kPrelaunchDefaultTimeout     = 20000;
const int kPrelaunchCpuThreshold       = 50;
const int kPrelaunchStablilityInterval = 2;

#ifdef Q_OS_LINUX
    const bool kCpuPollingEnabled = true;
#else
    const bool kCpuPollingEnabled = false;
#endif


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
    , m_info(NULL)
    , m_prelaunchEnabled(true)
    , m_pollingCpu(false)
    , m_prelaunchDelay(kPrelaunchDefaultTimeout)
#ifdef Q_OS_LINUX
    , m_cpuLoad(new CPULoad)
#else
    , m_cpuLoad(NULL)
#endif
{
}

/*!
 *  Destroy this and child objects.
 */

PrelaunchProcessBackendFactory::~PrelaunchProcessBackendFactory()
{
    delete m_cpuLoad;
}

/*!
 * Returns true if corresponding prelaunch can be created
 */

bool PrelaunchProcessBackendFactory::canCreate(const ProcessInfo &info) const
{
    if (!m_info)
        return false;

    return ProcessBackendFactory::canCreate(info);
}

/*!
 * Construct a PrelaunchProcessBackend from a ProcessInfo \a info record with \a parent.
 */

ProcessBackend * PrelaunchProcessBackendFactory::create(const ProcessInfo &info, QObject *parent)
{
    Q_ASSERT(m_info);

    PrelaunchProcessBackend *prelaunch = m_prelaunch;

    if (hasPrelaunchedProcess()) {
        // qDebug() << "Using existing prelaunch";
        m_prelaunch = NULL;
        prelaunchWhenPossible();
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
 * If there is a prelaunched process running, it will be return here.
 */

QList<Q_PID> PrelaunchProcessBackendFactory::internalProcesses()
{
    QList<Q_PID> list;
    if (m_prelaunch && m_prelaunch->isReady())
        list << m_prelaunch->pid();
    return list;
}

/*!
 * Returns ProcessInfo object
 */

ProcessInfo *PrelaunchProcessBackendFactory::processInfo() const
{
    return m_info;
}

/*!
 * Return the current launch interval in milliseconds.
 */

int PrelaunchProcessBackendFactory::launchInterval() const
{
    return m_prelaunchDelay;
}

/*!
 * Set the current launch interval to \a interval milliseconds.
 * If the timer is ticking, the new value will affect the next round.
 */

void PrelaunchProcessBackendFactory::setLaunchInterval(int interval)
{
    if (m_prelaunchDelay != interval) {
        m_prelaunchDelay = interval;
        emit launchIntervalChanged();
    }
}

/*!
 *  Returns whether prelaunching is enabled for this factory.
 */

bool PrelaunchProcessBackendFactory::prelaunchEnabled() const
{
    return m_prelaunchEnabled;
}

/*!
 * Enables and disables prelaunch
 */

void PrelaunchProcessBackendFactory::setPrelaunchEnabled(bool value)
{
    if (m_prelaunchEnabled != value) {
        m_prelaunchEnabled = value;

        if (m_prelaunchEnabled) {
            prelaunchWhenPossible();
        } else {
            disablePrelaunch();
        }
        emit prelaunchEnabledChanged();
    }
}

/*!
 *   Returns whether there is a prelaunched process which is ready to be consumed.
 */

bool PrelaunchProcessBackendFactory::hasPrelaunchedProcess() const
{
    return (m_prelaunch && m_prelaunch->isReady());
}

/*!
 *  Under memory restriction, terminate the prelaunch process.
 */

void PrelaunchProcessBackendFactory::handleMemoryRestrictionChange()
{
    if (m_memoryRestricted) {
        setPrelaunchEnabled(false);
        if (m_prelaunch) {
            delete m_prelaunch;   // This will kill the child process as well
            m_prelaunch = NULL;
        }
    } else {
        Q_ASSERT(m_prelaunch == NULL);
        setPrelaunchEnabled(true);
    }
}

/*!
 *  Returns the prelaunched process backend, or null if none is created.
 */

PrelaunchProcessBackend *PrelaunchProcessBackendFactory::prelaunchProcessBackend() const
{
    return m_prelaunch;
}

/*!
 * Launch a new prelaunched process backend.
 * In the future, it would be useful if the launch didn't require a timeout.
 */

void PrelaunchProcessBackendFactory::timeout()
{
    Q_ASSERT(m_prelaunch == NULL);
    Q_ASSERT(!m_memoryRestricted);
    Q_ASSERT(m_info);

    m_prelaunch = new PrelaunchProcessBackend(*m_info, this);
    connect(m_prelaunch, SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(prelaunchFinished(int,QProcess::ExitStatus)));
    connect(m_prelaunch, SIGNAL(error(QProcess::ProcessError)),
            SLOT(prelaunchError(QProcess::ProcessError)));
    m_prelaunch->prestart();
    emit processPrelaunched();
}

/*!
 * Handle a surprise termination condition - the prelaunched process died
 * unexpectedly.
 */

void PrelaunchProcessBackendFactory::prelaunchFinished(int exitCode, QProcess::ExitStatus status)
{
    qWarning() << Q_FUNC_INFO << "died unexpectedly" << exitCode << status;
    if (m_prelaunch) {
        m_prelaunch->deleteLater();
        m_prelaunch = NULL;
    }
    prelaunchWhenPossible();
}

/*!
 * Handle surprise error conditions on the prelaunched process.
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
        setPrelaunchEnabled(false);
    }
    else {
        // ### TODO: This isn't optimal
        prelaunchWhenPossible();
    }
}

/*!
 *  Sets the ProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
 *  An internal copy is made of the \a processInfo object.
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
            prelaunchWhenPossible();
        } else {
            disablePrelaunch();
        }
        emit processInfoChanged();
    }
}

/*!
 *  Sets the ProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
 */

void PrelaunchProcessBackendFactory::setProcessInfo(ProcessInfo& processInfo)
{
    setProcessInfo(&processInfo);
}

/*!
 * Prelaunch runtime when cpu load is low or by timer
 */

void PrelaunchProcessBackendFactory::prelaunchWhenPossible()
{
    if (m_prelaunchEnabled) {
        if (kCpuPollingEnabled) {
            m_accu = 0;
            m_waitTime = 0;
            enableCPULoadPolling(true);
            // qDebug() << "(cpu) start cpu polling, max waiting time " << (m_prelaunchDelay/1000) << " seconds";
        } else {
            m_timer.singleShot(m_prelaunchDelay, this, SLOT(timeout()));
        }
    }
}

/*!
 * disable prelaunch process and stops timers
 */

void PrelaunchProcessBackendFactory::disablePrelaunch()
{
    if (kCpuPollingEnabled) {
        enableCPULoadPolling(false);
    } else {
        m_timer.stop();
    }
}

/*!
 * Update cpu load information, triggers prelaunch start
 */

void PrelaunchProcessBackendFactory::checkCPULoadUpdated()
{
    Q_ASSERT(kCpuPollingEnabled);

    m_cpuLoad->update();
    m_waitTime += kPrelaunchCpuPollingInterval;
    int curLoad = m_cpuLoad->cpuLoad();

    if (m_prelaunchDelay != 0 && m_waitTime >= (m_prelaunchDelay)) {
        // qDebug() << "(cpu) can't wait low cpu load any longer";
        m_accu = kPrelaunchStablilityInterval;
    } else {
        if (curLoad < kPrelaunchCpuThreshold)
            m_accu++;
        else
            m_accu = 0;
    }

    // qDebug() << "(cpu) load is " << curLoad << " wait time " << m_waitTime/1000 << " seconds";

    if (m_accu >= kPrelaunchStablilityInterval) {
        // qDebug() << "(cpu) stop cpu polling, start runtimes prelaunching";
        enableCPULoadPolling(false);
        timeout();
    }
}

/*!
 * Enables or disables cpu load polling
 */

void PrelaunchProcessBackendFactory::enableCPULoadPolling(bool enable)
{
    Q_ASSERT(kCpuPollingEnabled);

    if (enable) {
        if (m_pollingCpu)
            return;

        connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkCPULoadUpdated()));
        m_cpuLoad->init();
        m_timer.setInterval(kPrelaunchCpuPollingInterval);
        m_timer.start();
        m_pollingCpu = true;

    } else {
        disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(checkCPULoadUpdated()));
        m_timer.stop();
        m_pollingCpu = false;
    }
}

/*!
 * \fn void PrelaunchProcessBackendFactory::launchIntervalChanged()
 * This signal is emitted when the launchInterval is changed.
 */

#include "moc_prelaunchprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

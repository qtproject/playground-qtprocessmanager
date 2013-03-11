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

#include <QFile>

#ifdef Q_OS_MAC
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#endif

#if defined(Q_OS_LINUX_ANDROID)
#include <ctype.h>
#endif

#include "qcpuidledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int    kIdleTimerInterval = 1000;
const double kDefaultLoadThreshold = 0.4;

/*!
  \class QCpuIdleDelegate
  \brief The QCpuIdleDelegate class generates a periodic cpu.
  \inmodule QtProcessManager

  The QCpuIdleDelegate class generates a periodic cpu for
  creation of idle resources.  When idle CPU resources are requested
  it checks the system load level approximately once per second.  When
  the load level is below the threshold set by \l{loadThreshold}, the
  idleCpuAvailable() signal will be omitted.
*/

/*!
  \property QCpuIdleDelegate::idleInterval
  \brief Time in milliseconds before a new idle CPU request will be fulfilled
 */

/*!
  \property QCpuIdleDelegate::loadThreshold
  \brief Load level we need to be under to generate idle CPU requests.

  This value is a double that ranges from 0.0 to 1.0.  A value greater
  than or equal to 1.0 guarantees that \l{idleCpuAvailable()} signals
  will always be emitted.  A value less than 0.0 blocks all \l{idleCpuAvailable()}
  signals.
 */


/*!
    Construct a QCpuIdleDelegate with an optional \a parent.
*/

QCpuIdleDelegate::QCpuIdleDelegate(QObject *parent)
    : QIdleDelegate(parent)
    , m_load(1.0)
    , m_loadThreshold(kDefaultLoadThreshold)
    , m_total(0)
    , m_idle(0)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    m_timer.setInterval(kIdleTimerInterval);
}

/*!
    Turn on or off idle requests based on \a state.
*/

void QCpuIdleDelegate::handleStateChange(bool state)
{
    if (state) {
        updateStats(true);
        m_timer.start();
    }
    else
        m_timer.stop();
}

/*!
  \internal
 */

double QCpuIdleDelegate::updateStats(bool reset)
{
    int idle = 0;
    int total = 0;

    m_load = 1.0;  // Always reset to max in case of error

#if defined(Q_OS_LINUX)
    QList<QByteArray> stats;
    QFile file(QStringLiteral("/proc/stat"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray line = file.readLine();
        int offset = 0;
        while (offset < line.size() && !isdigit(line.at(offset)))
            offset++;
        if (offset < line.size())
            stats = line.mid(offset).split(' ');
    }
    if (stats.size() < 5)
        qFatal("Unable to read /proc/stat file");

    idle = stats.at(3).toInt();
    for (int i = 1 ; i < stats.size() ; i++)
        total += stats.at(i).toInt();

#elif defined(Q_OS_MAC)
    natural_t n_cpus;
    processor_info_t pinfo;
    mach_msg_type_number_t msg_count;
    kern_return_t status = host_processor_info(mach_host_self(),
                                               PROCESSOR_CPU_LOAD_INFO,
                                               &n_cpus,
                                               (processor_info_array_t *)&pinfo,
                                               &msg_count);
    if (status != KERN_SUCCESS)
        qFatal("Unable to read host processor info");

    for (unsigned int cpu = 0 ; cpu < n_cpus ; cpu++) {
        processor_info_t p = pinfo + (CPU_STATE_MAX * cpu);
        total += p[CPU_STATE_USER] + p[CPU_STATE_SYSTEM]
            + p[CPU_STATE_NICE] + p[CPU_STATE_IDLE];
        idle += p[CPU_STATE_IDLE];
    }
    vm_deallocate(mach_task_self(),
                  (vm_address_t)pinfo,
                  (vm_size_t)sizeof(*pinfo) * msg_count);
#endif
    if (!reset && total > m_total)
        m_load = 1.0 - ((float) (idle - m_idle)) / (total - m_total);

    m_total = total;
    m_idle = idle;
    return m_load;
}

/*!
  \internal
 */

void QCpuIdleDelegate::timeout()
{
    if (updateStats(false) <= m_loadThreshold)
        emit idleCpuAvailable();
    emit loadUpdate(m_load);
}

/*!
  Return the current launch interval in milliseconds
 */

int QCpuIdleDelegate::idleInterval() const
{
    return m_timer.interval();
}

/*!
  Set the current idle interval to \a interval milliseconds
*/

void QCpuIdleDelegate::setIdleInterval(int interval)
{
    if (m_timer.interval() != interval) {
        m_timer.stop();
        m_timer.setInterval(interval);
        if (enabled() && requested())
            m_timer.start();
        emit idleIntervalChanged();
    }
}

/*!
  Return the current load threshold as a number from 0.0 to 1.0
 */

double QCpuIdleDelegate::loadThreshold() const
{
    return m_loadThreshold;
}

/*!
  Set the current load threshold to \a threshold milliseconds
*/

void QCpuIdleDelegate::setLoadThreshold(double threshold)
{
    if (m_loadThreshold != threshold) {
        m_loadThreshold = threshold;
        emit loadThresholdChanged();
    }
}

/*!
  \fn void QCpuIdleDelegate::idleIntervalChanged()
  This signal is emitted when the idleInterval is changed.
 */

/*!
  \fn void QCpuIdleDelegate::loadThresholdChanged()
  This signal is emitted when the loadThreshold is changed.
 */

/*!
  \fn void QCpuIdleDelegate::loadUpdate(double load)
  This signal is emitted when the load is read.  It mainly
  serves as a debugging signal.  The \a load value will
  range between 0.0 and 1.0
 */


#include "moc_qcpuidledelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

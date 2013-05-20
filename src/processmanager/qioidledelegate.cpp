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

#include <QFile>
#include <QDebug>
#include <QStringList>

#ifdef Q_OS_MAC
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#endif

#include "qioidledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int    kIdleTimerInterval = 1000;
const double kDefaultLoadThreshold = 0.4;

/*!
  \class QIoIdleDelegate
  \brief The QIoIdleDelegate class generates \l{idleCpuAvailable()} signals.
  \inmodule QtProcessManager

  The QIoIdleDelegate determintes available CPU resources by watching
  the IO activity level on your system.  This delegate is appropriate
  when your system is primarily IO bound and not CPU bound.  When idle
  CPU resources are requested it checks the IO load level
  approximately once per second.  The calculated IO load level is
  measured by looking at the percentage of time spent in IO requests
  over the last time interval.  When the load level is below the
  threshold set by \l{loadThreshold}, the idleCpuAvailable() signal
  will be emitted.

  Under Linux, the \c{/proc/diskstats} file system object provides
  basic IO statistics use across all storage devices.  The
  QIoIdleDelegate reads statistics for exactly one of these devices,
  from the \c{/sys/block/DEVICE/stat} file system entry.  To use the
  QIoIdleDelegate under Linux, you must set the device property to be
  the name of the disk object.

  For example, on a standard Linux laptop, the hard disk usually shows
  up as \c{/dev/sda}, with appropriate file system partitions [on my
  laptop \c{/dev/sda1} is the main partition and \c{/dev/sda5} is the
  swap partition].  If I want to monitor io use on the main device, I
  could write:

  \code
    QIoIdleDelegate *delegate = new QIoIdleDelegate;
    delegate->setDevice("sda1");
  \endcode

  If I want to monitor disk use in general:

  \code
    delegate->setDevice("sda");
  \endcode

*/

/*!
  \property QIoIdleDelegate::idleInterval
  \brief Time in milliseconds before a new idle CPU request will be fulfilled
 */

/*!
  \property QIoIdleDelegate::device
  \brief Unix device name for the disk that you want to measure
 */

/*!
  \property QIoIdleDelegate::loadThreshold
  \brief Load level we need to be under to generate idle CPU requests.

  This value is a double that ranges from 0.0 to 1.0.  A value greater
  than or equal to 1.0 guarantees that \l{idleCpuAvailable()} signals
  will always be emitted.  A value less than 0.0 blocks all \l{idleCpuAvailable()}
  signals.
 */


/*!
    Construct a QIoIdleDelegate with an optional \a parent.
*/

QIoIdleDelegate::QIoIdleDelegate(QObject *parent)
    : QIdleDelegate(parent)
    , m_load(1.0)
    , m_loadThreshold(kDefaultLoadThreshold)
    , m_last_msecs(0)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    m_timer.setInterval(kIdleTimerInterval);
}

/*!
    Turn on or off idle requests based on \a state.
*/

void QIoIdleDelegate::handleStateChange(bool state)
{
    if (state) {
        updateStats(true);
        m_elapsed.start();
        m_timer.start();
    }
    else {
        m_elapsed.invalidate();
        m_timer.stop();
    }
}

/*!
  \internal
 */

double QIoIdleDelegate::updateStats(bool reset)
{
    int msecs = 0;
    m_load = 1.0;  // Always reset to max in case of error

#if defined(Q_OS_LINUX)
    QStringList stats;
    QFile file(QString::fromLatin1("/sys/block/%1/stat").arg(m_device));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString line = QString::fromLocal8Bit(file.readLine());
        stats = line.split(' ', QString::SkipEmptyParts);
    }
    if (stats.size() < 11)
        qFatal("Unable to read /sys/block stat file");

    msecs = stats.at(9).toInt();
#elif defined(Q_OS_MAC)
    // ### TODO
#endif
    if (!reset && m_last_msecs > 0 && m_elapsed.isValid()) {
        qint64 elapsed = m_elapsed.restart();
        if (elapsed > 0)
            m_load = (msecs - m_last_msecs) / (double) elapsed;
    }
    m_last_msecs = msecs;
    return m_load;
}

/*!
  \internal
 */

void QIoIdleDelegate::timeout()
{
    if (updateStats(false) <= m_loadThreshold)
        emit idleCpuAvailable();
    emit loadUpdate(m_load);
}

/*!
  Return the current launch interval in milliseconds
 */

int QIoIdleDelegate::idleInterval() const
{
    return m_timer.interval();
}

/*!
  Set the current idle interval to \a interval milliseconds
*/

void QIoIdleDelegate::setIdleInterval(int interval)
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

double QIoIdleDelegate::loadThreshold() const
{
    return m_loadThreshold;
}

/*!
  Set the current load threshold to \a threshold milliseconds
*/

void QIoIdleDelegate::setLoadThreshold(double threshold)
{
    if (m_loadThreshold != threshold) {
        m_loadThreshold = threshold;
        emit loadThresholdChanged();
    }
}

/*!
  Return the current IO device
 */

QString QIoIdleDelegate::device() const
{
    return m_device;
}

/*!
  Set the current IO device
*/

void QIoIdleDelegate::setDevice(const QString& device)
{
    if (m_device != device) {
        m_device = device;
        m_last_msecs = 0;
        emit deviceChanged();
    }
}

/*!
  \fn void QIoIdleDelegate::idleIntervalChanged()
  This signal is emitted when the idleInterval is changed.
 */

/*!
  \fn void QIoIdleDelegate::loadThresholdChanged()
  This signal is emitted when the loadThreshold is changed.
 */

/*!
  \fn void QIoIdleDelegate::deviceChanged()
  This signal is emitted when the device is changed.
 */

/*!
  \fn void QIoIdleDelegate::loadUpdate(double load)
  This signal is emitted when the load is read.  It mainly
  serves as a debugging signal.  The \a load value will
  range between 0.0 and 1.0
 */


#include "moc_qioidledelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

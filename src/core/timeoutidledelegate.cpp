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

#include "timeoutidledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const int kIdleTimerInterval = 1000;

/*!
  \class TimeoutIdleDelegate
  \brief The TimeoutIdleDelegate class generates a periodic timeout.

  The TimeoutIdleDelegate class generates a periodic timeout for
  creation of idle resources.  This is not an intelligent class - it
  doesn't matter what your CPU is doing, it will still generate the
  periodic timeout.
*/

/*!
  \property TimeoutIdleDelegate::idleInterval
  \brief Time in milliseconds before a new idle CPU request will be fulfilled
 */


/*!
    Construct a TimeoutIdleDelegate with an optional \a parent.
*/

TimeoutIdleDelegate::TimeoutIdleDelegate(QObject *parent)
    : IdleDelegate(parent)
{
    connect(&m_timer, SIGNAL(timeout()), SIGNAL(idleCpuAvailable()));
    m_timer.setInterval(kIdleTimerInterval);
}

/*!
    \fn void TimeoutIdleDelegate::requestIdleCpu(bool request)

    Turn on or off idle requests based on \a request
    You must override this function in a subclass.
*/

void TimeoutIdleDelegate::requestIdleCpu(bool request)
{
    if (request != m_timer.isActive()) {
        if (request)
            m_timer.start();
        else
            m_timer.stop();
    }
}

/*!
  Return the current launch interval in milliseconds
 */

int TimeoutIdleDelegate::idleInterval() const
{
    return m_timer.interval();
}

/*!
  Set the current idle interval to \a interval milliseconds
*/

void TimeoutIdleDelegate::setIdleInterval(int interval)
{
    if (m_timer.interval() != interval) {
        bool active = m_timer.isActive();
        m_timer.stop();
        m_timer.setInterval(interval);
        if (active)
            m_timer.start();
        emit idleIntervalChanged();
    }
}

/*!
  \fn void TimeoutIdleDelegate::idleIntervalChanged()
  This signal is emitted when the idleInterval is changed.
 */


#include "moc_timeoutidledelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

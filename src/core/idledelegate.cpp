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

#include "idledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class IdleDelegate
  \brief The IdleDelegate class is a virtual class for gathering idle CPU cycles

  You must subclass this class to do anything useful.

  The IdleDelegate is turned on and off by the \l{requestIdleCpu()} function.
  The IdleDelegate should emit the \l{idleCpuAvailable()} signal approximately
  once per second when it is turned on.

  Subclasses should respect the \l{enabled} property by not generating signals
  if the IdleDelegate is not enabled.
*/

/*!
  \property IdleDelegate::enabled
  \brief Boolean value of whether or not this IdleDelegate should generate signals.
 */

/*!
    Construct a IdleDelegate with an optional \a parent.
*/

IdleDelegate::IdleDelegate(QObject *parent)
    : QObject(parent)
    , m_enabled(true)
    , m_requested(false)
{
}

/*!
  Set the \a enabled property.
 */

void IdleDelegate::setEnabled(bool value)
{
    if (m_enabled != value) {
        m_enabled = value;
        emit enabledChanged();
        if (m_requested)
            handleStateChange(m_requested && m_enabled);
    }
}

/*!
    \fn void IdleDelegate::requestIdleCpu(bool request)
    Turn on or off idle requests based on \a request.
*/

void IdleDelegate::requestIdleCpu(bool request)
{
    if (m_requested != request) {
        m_requested = request;
        if (m_enabled)
            handleStateChange(m_requested && m_enabled);
    }
}

/*!
    \fn void IdleDelegate::handleStateChange(bool state)
    Override this in subclasses to turn on and off your Idle delegate
    based on \a state (which is just the logical OR of \l{requested()} and \l{enabled()}
*/

/*!
    \fn bool IdleDelegate::enabled() const
    Return true if this delegate is enabled
*/

/*!
    \fn bool IdleDelegate::requested() const
    Return true if this delegate has been requested
*/

/*!
    \fn void IdleDelegate::idleCpuAvailable()
    Signal emitted periodically when idle CPU resources are available.
*/

/*!
    \fn void IdleDelegate::enabledChanged()
    Signal emitted when enabled value is changed
*/

#include "moc_idledelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

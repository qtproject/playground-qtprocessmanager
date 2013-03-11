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

#include "qidledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QIdleDelegate
  \brief The QIdleDelegate class is a virtual class for gathering idle CPU cycles
  \inmodule QtProcessManager

  You must subclass this class to do anything useful.

  The QIdleDelegate is turned on and off by the \l{requestIdleCpu()} function.
  The QIdleDelegate should emit the \l{idleCpuAvailable()} signal approximately
  once per second when it is turned on.

  Subclasses should respect the \l{enabled} property by not generating signals
  if the QIdleDelegate is not enabled.
*/

/*!
  \property QIdleDelegate::enabled
  \brief Boolean value of whether or not this QIdleDelegate should generate signals.
 */

/*!
    Construct a QIdleDelegate with an optional \a parent.
*/

QIdleDelegate::QIdleDelegate(QObject *parent)
    : QObject(parent)
    , m_enabled(true)
    , m_requested(false)
{
}

/*!
  Set the \a enabled property.
 */

void QIdleDelegate::setEnabled(bool value)
{
    if (m_enabled != value) {
        m_enabled = value;
        emit enabledChanged();
        if (m_requested)
            handleStateChange(m_requested && m_enabled);
    }
}

/*!
    \fn void QIdleDelegate::requestIdleCpu(bool request)
    Turn on or off idle requests based on \a request.
*/

void QIdleDelegate::requestIdleCpu(bool request)
{
    if (m_requested != request) {
        m_requested = request;
        if (m_enabled)
            handleStateChange(m_requested && m_enabled);
    }
}

/*!
    \fn void QIdleDelegate::handleStateChange(bool state)
    Override this in subclasses to turn on and off your Idle delegate
    based on \a state (which is just the logical OR of \l{requested()} and \l{enabled()}
*/

/*!
    \fn bool QIdleDelegate::enabled() const
    Return true if this delegate is enabled
*/

/*!
    \fn bool QIdleDelegate::requested() const
    Return true if this delegate has been requested
*/

/*!
    \fn void QIdleDelegate::idleCpuAvailable()
    Signal emitted periodically when idle CPU resources are available.
*/

/*!
    \fn void QIdleDelegate::enabledChanged()
    Signal emitted when enabled value is changed
*/

#include "moc_qidledelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

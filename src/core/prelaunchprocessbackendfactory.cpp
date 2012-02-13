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
  The \a info ProcessInfo is used to start the prelaunched process.  This is
  different from the final ProcessInfo which will be passed to the prelaunched
  process as a QBinaryJson document.
*/

PrelaunchProcessBackendFactory::PrelaunchProcessBackendFactory(const ProcessInfo& info, QObject *parent)
    : ProcessBackendFactory(parent)
    , m_prelaunch(NULL)
    , m_info(info)
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(timeout()));
    m_timer.setSingleShot(true);
    m_timer.setInterval(kPrelaunchTimerInterval);
    m_timer.start();
}

/*!
   Destroy this and child objects.
*/

PrelaunchProcessBackendFactory::~PrelaunchProcessBackendFactory()
{
}

/*!
  Construct a PrelaunchProcessBackend from a ProcessInfo \a info record with \a parent.
*/

ProcessBackend * PrelaunchProcessBackendFactory::create(const ProcessInfo& info, QObject *parent)
{
    PrelaunchProcessBackend *prelaunch = m_prelaunch;

    if ( prelaunch && prelaunch->isReady() ) {
        // qDebug() << "Using existing prelaunch";
        m_prelaunch = NULL;
        m_timer.start();
        prelaunch->setInfo(info);
        prelaunch->setParent(parent);
        prelaunch->disconnect(this);
    }
    else {
        // qDebug() << "Creating prelaunch from scratch";
        prelaunch = new PrelaunchProcessBackend(m_info, parent);
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
            m_timer.start();
        emit launchIntervalChanged();
    }
}

/*!
  Under memory restriction, terminate the prelaunch process.
 */

void PrelaunchProcessBackendFactory::handleMemoryRestrictionChange()
{
    if ( m_memoryRestricted ) {
        m_timer.stop();
        if (m_prelaunch) {
            delete m_prelaunch;   // This will kill the child process as well
            m_prelaunch = NULL;
        }
    } else {
        Q_ASSERT(m_prelaunch == NULL);
        m_timer.start();
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

    m_prelaunch = new PrelaunchProcessBackend(m_info, this);
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
    m_timer.start();
}

/*!
  \fn void PrelaunchProcessBackendFactory::launchIntervalChanged()
  This signal is emitted when the launchInterval is changed.
 */

#include "moc_prelaunchprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

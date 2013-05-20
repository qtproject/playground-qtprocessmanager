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

#include <QDebug>

#include "qprelaunchprocessbackendfactory.h"
#include "qprelaunchprocessbackend.h"
#include "qprocessinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QPrelaunchProcessBackendFactory
  \brief The QPrelaunchProcessBackendFactory class creates QPrelaunchProcessBackend objects
  \inmodule QtProcessManager

  The QPrelaunchProcessBackendFactory starts up a QPrelaunchProcessBackend using
  information passed in the constructor.
*/

/*!
  \property QPrelaunchProcessBackendFactory::processInfo
  \brief QProcessInfo record used to create the prelaunched process
 */

/*!
  \property QPrelaunchProcessBackendFactory::prelaunchEnabled
  \brief Controls whether or not a prelaunched process is created.

  If this property is true, a prelaunched process will be created
  after a suitable time interval.  If it is set to false and a
  prelaunched process exists, that process will be terminated.
 */

/*!
  Construct a QPrelaunchProcessBackendFactory with optional \a parent.
  To be able to use the QPrelaunchProcessBackendFactory, you also need to set
  a QProcessInfo object to it that specifies which process is prelaunched.
*/
QPrelaunchProcessBackendFactory::QPrelaunchProcessBackendFactory(QObject *parent)
    : QProcessBackendFactory(parent)
    , m_prelaunch(NULL)
    , m_info(NULL)
    , m_prelaunchEnabled(true)
{
}

/*!
   Destroy this and child objects.
*/

QPrelaunchProcessBackendFactory::~QPrelaunchProcessBackendFactory()
{
}

/*!
  Return true if the QPrelaunchProcessBackendFactory can create
  a process that matches \a info.  The default implementation only checks that
  a valid info object has been previously set in the factory, and then
  passes the decision on to the default implementation, which should probably
  have some kind of QMatchDelegate installed.
*/

bool QPrelaunchProcessBackendFactory::canCreate(const QProcessInfo &info) const
{
    if (!m_info)
        return false;

    return QProcessBackendFactory::canCreate(info);
}

/*!
  Construct a PrelaunchProcessBackend from a QProcessInfo \a info record with \a parent.
*/

QProcessBackend * QPrelaunchProcessBackendFactory::create(const QProcessInfo &info, QObject *parent)
{
    Q_ASSERT(m_info);
    QPrelaunchProcessBackend *prelaunch = m_prelaunch;

    if (hasPrelaunchedProcess()) {
        // qDebug() << "Using existing prelaunch";
        m_prelaunch = NULL;
        prelaunch->setInfo(info);
        prelaunch->setParent(parent);
        prelaunch->disconnect(this);
        updateState();
    } else {
        // qDebug() << "Creating prelaunch from scratch";
        prelaunch = new QPrelaunchProcessBackend(*m_info, parent);
        prelaunch->prestart();
        prelaunch->setInfo(info);
    }
    return prelaunch;
}

/*!
  Return the prelaunched process information
 */

QProcessInfo *QPrelaunchProcessBackendFactory::processInfo() const
{
    return m_info;
}

/*!
    Returns whether prelaunching is enabled for this factory.
*/
bool QPrelaunchProcessBackendFactory::prelaunchEnabled() const
{
    return m_prelaunchEnabled;
}

void QPrelaunchProcessBackendFactory::setPrelaunchEnabled(bool value)
{
    if (m_prelaunchEnabled != value) {
        m_prelaunchEnabled = value;
        updateState();
        emit prelaunchEnabledChanged();
    }
}

/*!
    Returns whether there is a prelaunched process which is ready to be consumed.
*/
bool QPrelaunchProcessBackendFactory::hasPrelaunchedProcess() const
{
    return (m_prelaunch && m_prelaunch->isReady());
}

/*!
    Under memory restriction, terminate the prelaunch process.
*/
void QPrelaunchProcessBackendFactory::handleMemoryRestrictionChange()
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
QPrelaunchProcessBackend *QPrelaunchProcessBackendFactory::prelaunchProcessBackend() const
{
    return m_prelaunch;
}

/*!
  Launch a new prelaunched process backend.
 */

void QPrelaunchProcessBackendFactory::idleCpuAvailable()
{
    // qDebug() << Q_FUNC_INFO;
    if (m_prelaunchEnabled && !m_prelaunch && !m_memoryRestricted && m_info) {
        // qDebug() << Q_FUNC_INFO << "...launching";
        m_prelaunch = new QPrelaunchProcessBackend(*m_info, this);
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

void QPrelaunchProcessBackendFactory::prelaunchFinished(int exitCode, QProcess::ExitStatus status)
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

void QPrelaunchProcessBackendFactory::prelaunchError(QProcess::ProcessError err)
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
    emit internalProcessError(err);
}

/*!
   Update our presented state, consisting of whether or not we need
   idle CPU resources and how many internal processes we are running.
*/
void QPrelaunchProcessBackendFactory::updateState()
{
    Q_ASSERT(!m_prelaunch || !m_memoryRestricted);  // If memory is restricted, we must not have a prelaunch process

    setIdleCpuRequest(!m_memoryRestricted && m_prelaunchEnabled && !m_prelaunch && m_info);

    QPidList list;
    if (m_prelaunch && m_prelaunch->isReady())
        list << m_prelaunch->pid();
    setInternalProcesses(list);
}

/*!
    Sets the QProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
    An internal copy is made of the \a processInfo object.
 */
void QPrelaunchProcessBackendFactory::setProcessInfo(QProcessInfo *processInfo)
{
    if (m_info != processInfo) {
        if (m_info) {
            delete m_info;
            m_info = NULL;
        }

        if (processInfo) {
            m_info = new QProcessInfo(*processInfo);
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
    Sets the QProcessInfo that is used to determine the prelaunched runtime to \a processInfo.
 */
void QPrelaunchProcessBackendFactory::setProcessInfo(QProcessInfo& processInfo)
{
    setProcessInfo(&processInfo);
}


/*!
  \fn void QPrelaunchProcessBackendFactory::processInfoChanged()
  This signal is emitted when the internal QProcessInfo record is
  changed.
 */
/*!
  \fn void QPrelaunchProcessBackendFactory::prelaunchEnabledChanged()
  This signal is emitted when the prelaunchEnabled property is changed.
 */
/*!
  \fn void QPrelaunchProcessBackendFactory::processPrelaunched()
  This signal is emitted when the prelaunched process is first created.
 */

#include "moc_qprelaunchprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

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

#include "qprocessbackendmanager.h"
#include "qprocessbackendfactory.h"
#include "qprocessbackend.h"
#include "qcpuidledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QProcessBackendManager
  \brief The QProcessBackendManager class contains a list of QProcessBackendFactory
         objects and is used to create QProcessBackend objects.
  \inmodule QtProcessManager

  The QProcessBackendManager class is a standalone class for creating QProcessBackend
  objects from factories.  Typical use for the backend manager is like:

  \code
  QProcessBackendManager *bm = new QProcessBackendManager;
  ProcessInfo info;
  info.setProgram("/home/me/myprogram");
  bm->add(new QPrelaunchProcessBackendFactory(info);
  pm->add(new QStandardProcessBackendFactory);

  // ...

  QProcessInfo i;
  i.setProgram("/home/me/myprogram");
  i.setWorkingDirectory("/root");
  QProcessBackend *backend = bm->create(i);
  \endcode

  The backend manager does not get involved in starting or tracking
  the lifetime of a process.  In general, you should use the
  QProcessManager class for processes, which contains a backend manager object.

  You may assign a QIdleDelegate to the process manager.  Certain factory
  objects required processing time to launch prelaunched runtime processes.
  A QIdleDelegate is a class that lets the process manager know when the
  system load is low so that the prelaunch programs can be started.  If
  you do not assign a QIdleDelegate, you may subclass the QProcessBackendManager
  to override the default idle calculations.

  If you do not assign a QIdleDelegate, the QCpuIdleDelegate will be
  used by default.

  If you prefer to not use delegates, you can subclass QProcessBackendManager
  and override the \l{handleIdleCpuRequest()} function.  If you do this,
  you must shut off the default QIdleDelegate.  For example:

  \code
  class MyManager : public QProcessBackendManager {
  public
    MyManager(QObject *parent=0) : QProcessBackendManager(parent) {
      setIdleDelegate(0);
      connect(&timer, SIGNAL(timeout()), SLOT(checkCpuLoad()));
      timer.setInterval(1000);
    }

  protected:
    virtual void handleIdleCputRequest(bool request) {
      if (request) timer.start();
      else         timer.stop();
    }

  protected slots:
    void checkCpuLoad() {
       if (calcCpuLoad() < 50)
          idleCpuAvailable();   // Call the Idle CPU function
    }

  private:
    QTimer timer;
  }
  \endcode
*/

/*!
    \property QProcessBackendManager::idleDelegate
    \brief The QIdleDelegate object assigned to this factory.
*/

/*!
  Construct a QProcessBackendManager with an optional \a parent
  By default, a CpuIdleDelegate is assigned to the idleDelegate.
*/

QProcessBackendManager::QProcessBackendManager(QObject *parent)
    : QObject(parent)
    , m_memoryRestricted(false)
    , m_idleCpuRequest(false)
{
    m_idleDelegate = new QCpuIdleDelegate(this);
    connect(m_idleDelegate, SIGNAL(idleCpuAvailable()), SLOT(idleCpuAvailable()));
}

/*!
  Delete the QProcessBackendManager and terminate all factory processes.
*/

QProcessBackendManager::~QProcessBackendManager()
{
}

/*!
  Create a new QProcessBackend based on QProcessInfo \a info and \a parent.
*/

QProcessBackend *QProcessBackendManager::create(const QProcessInfo& info, QObject *parent)
{
    foreach (QProcessBackendFactory *factory, m_factories) {
        if (factory->canCreate(info)) {
            QProcessInfo i = info;
            factory->rewrite(i);
            return factory->create(i, parent);
        }
    }
    return NULL;
}

/*!
  Add a QProcessBackendFactory \a factory to the end of the Factory list.
  The factory becomes a child of the backend manager.
*/

void QProcessBackendManager::addFactory(QProcessBackendFactory *factory)
{
    m_factories.append(factory);
    factory->setParent(this);
    factory->setMemoryRestricted(m_memoryRestricted);
    connect(factory, SIGNAL(internalProcessesChanged()), SLOT(updateInternalProcesses()));
    connect(factory, SIGNAL(internalProcessError(QProcess::ProcessError)),
            SIGNAL(internalProcessError(QProcess::ProcessError)));
    connect(factory, SIGNAL(internalProcessError(QProcess::ProcessError)),
            SLOT(handleInternalProcessError(QProcess::ProcessError)));
    connect(factory, SIGNAL(idleCpuRequestChanged()), SLOT(updateIdleCpuRequest()));
    updateIdleCpuRequest();
}

/*!
  Return a list of all internal processes being used by factories
*/

QPidList QProcessBackendManager::internalProcesses() const
{
    return m_internalProcesses;
}

/*!
  Set memory restrictions.  If \a memoryRestricted is true
  all factories are requested to minimize memory use.
*/

void QProcessBackendManager::setMemoryRestricted(bool memoryRestricted)
{
    if (m_memoryRestricted != memoryRestricted) {
        m_memoryRestricted = memoryRestricted;
        foreach (QProcessBackendFactory *factory, m_factories) {
            factory->setMemoryRestricted(memoryRestricted);
        }
    }
}

/*!
  Return current memory restriction setting
*/

bool QProcessBackendManager::memoryRestricted() const
{
    return m_memoryRestricted;
}

/*!
   Return the current QIdleDelegate object
 */

QIdleDelegate *QProcessBackendManager::idleDelegate() const
{
    return m_idleDelegate;
}

/*!
   Set a new process QIdleDelegate object \a idleDelegate.
   The QProcessBackendManager takes over parentage of the QIdleDelegate.
 */

void QProcessBackendManager::setIdleDelegate(QIdleDelegate *idleDelegate)
{
    if (idleDelegate != m_idleDelegate) {
        if (m_idleDelegate)
            delete m_idleDelegate;
        m_idleDelegate = idleDelegate;
        if (m_idleDelegate) {
            m_idleDelegate->setParent(this);
            connect(m_idleDelegate, SIGNAL(idleCpuAvailable()), SLOT(idleCpuAvailable()));
        }
        emit idleDelegateChanged();
        m_idleCpuRequest = false;  // Force this to be recalculated
        updateIdleCpuRequest();
    }
}

/*!
   \fn bool QProcessBackendManager::idleCpuRequest() const
   Return \c{true} if we need idle CPU cycles.
 */

/*!
  Idle CPU processing is available.  This function distributes
  the idle CPU to the first factory that has requested it.
 */

void QProcessBackendManager::idleCpuAvailable()
{
    foreach (QProcessBackendFactory *factory, m_factories) {
        if (factory->idleCpuRequest()) {
            factory->idleCpuAvailable();
            return;
        }
    }
}

/*!
  Update the current idle cpu request status by polling
  the factories.
 */

void QProcessBackendManager::updateIdleCpuRequest()
{
    bool request = false;
    foreach (QProcessBackendFactory *factory, m_factories)
        request |= factory->idleCpuRequest();

    if (request != m_idleCpuRequest) {
        m_idleCpuRequest = request;
        if (m_idleDelegate)
            m_idleDelegate->requestIdleCpu(m_idleCpuRequest);
        handleIdleCpuRequest();
    }
}

/*!
  Update the list of internal processes
 */

void QProcessBackendManager::updateInternalProcesses()
{
    QList<Q_PID> plist;
    foreach (QProcessBackendFactory *factory, m_factories)
        plist.append(factory->internalProcesses());
    qSort(plist);
    if (!compareSortedLists(plist, m_internalProcesses)) {
        m_internalProcesses = plist;
        handleInternalProcessChange();
        emit internalProcessesChanged();
    }
}

/*!
  Override this function to customize your handling of Idle CPU requests.
 */

void QProcessBackendManager::handleIdleCpuRequest()
{
}

/*!
  Override this function to customize your handling of changes in the
  list of internal processes.
 */

void QProcessBackendManager::handleInternalProcessChange()
{
}

/*!
  Override thie function to customize your handling of internal
  process \a error values.
 */

void QProcessBackendManager::handleInternalProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
}

/*!
  \fn void QProcessBackendManager::internalProcessesChanged()
  Signal emitted whenever the list of internal processes has changed.
*/

/*!
  \fn void QProcessBackendManager::idleDelegateChanged()
  Signal emitted whenever the IdleDelegate is changed.
*/

/*!
  \fn void QProcessBackendManager::internalProcessError(QProcess::ProcessError error)
  Signal emitted when an internal process has an \a error.
*/

#include "moc_qprocessbackendmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

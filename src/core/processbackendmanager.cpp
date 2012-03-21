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

#include "processbackendmanager.h"
#include "processbackendfactory.h"
#include "processbackend.h"
#include "cpuidledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class ProcessBackendManager
  \brief The ProcessBackendManager class contains a list of ProcessBackendFactory
         objects and is used to create ProcessBackend objects.

  The ProcessBackendManager class is a standalone class for creating ProcessBackend
  objects from factories.  Typical use for the backend manager is like:

  \code
  ProcessBackendManager *bm = new ProcessBackendManager;
  ProcessInfo info;
  info.setProgram("/home/me/myprogram");
  bm->add(new PrelaunchProcessBackendFactory(info);
  pm->add(new StandardProcessBackendFactory);

  // ...

  ProcessInfo i;
  i.setProgram("/home/me/myprogram");
  i.setWorkingDirectory("/root");
  ProcessBackend *backend = bm->create(i);
  \endcode

  The backend manager does not get involved in starting or tracking
  the lifetime of a process.  In general, you should use the
  ProcessManager class for processes, which contains a backend manager object.

  You may assign an IdleDelegate to the process manager.  Certain factory
  objects required processing time to launch prelaunched runtime processes.
  An IdleDelegate is a class that lets the process manager know when the
  system load is low so that the prelaunch programs can be started.  If
  you do not assign an IdleDelegate, you may subclass the ProcessBackendManager
  to override the default idle calculations.

  If you do not assign an IdleDelegate, the CpuIdleDelegate will be
  used by default.

  If you prefer to not use delegates, you can subclass ProcessBackendManager
  and override the \l{handleIdleCpuRequest()} function.  If you do this,
  you must shut off the default IdleDelegate.  For example:

  \code
  class MyManager : public ProcessBackendManager {
  public
    MyManager(QObject *parent=0) : ProcessBackendManager(parent) {
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
    \property ProcessBackendManager::idleDelegate
    \brief The IdleDelegate object assigned to this factory.
*/

/*!
  Construct a ProcessBackendManager with an optional \a parent
  By default, a CpuIdleDelegate is assigned to the idleDelegate.
*/

ProcessBackendManager::ProcessBackendManager(QObject *parent)
    : QObject(parent)
    , m_memoryRestricted(false)
    , m_idleCpuRequest(false)
{
    m_idleDelegate = new CpuIdleDelegate(this);
    connect(m_idleDelegate, SIGNAL(idleCpuAvailable()), SLOT(idleCpuAvailable()));
}

/*!
  Delete the ProcessBackendManager and terminate all factory processes.
*/

ProcessBackendManager::~ProcessBackendManager()
{
}

/*!
  Create a new ProcessBackend based on ProcessInfo \a info and \a parent.
*/

ProcessBackend *ProcessBackendManager::create(const ProcessInfo& info, QObject *parent)
{
    foreach (ProcessBackendFactory *factory, m_factories) {
        if (factory->canCreate(info)) {
            ProcessInfo i = info;
            factory->rewrite(i);
            return factory->create(i, parent);
        }
    }
    return NULL;
}

/*!
  Add a ProcessBackendFactory \a factory to the end of the Factory list.
  The factory becomes a child of the backend manager.
*/

void ProcessBackendManager::addFactory(ProcessBackendFactory *factory)
{
    m_factories.append(factory);
    factory->setParent(this);
    factory->setMemoryRestricted(m_memoryRestricted);
    connect(factory, SIGNAL(idleCpuRequestChanged()), SLOT(updateIdleCpuRequest()));
    updateIdleCpuRequest();
}

/*!
  Return a list of all internal processes being used by factories
*/

QList<Q_PID> ProcessBackendManager::internalProcesses()
{
    QList<Q_PID> plist;
    foreach (ProcessBackendFactory *factory, m_factories)
        plist.append(factory->internalProcesses());
    return plist;
}

/*!
  Set memory restrictions.  If \a memoryRestricted is true
  all factories are requested to minimize memory use.
*/

void ProcessBackendManager::setMemoryRestricted(bool memoryRestricted)
{
    if (m_memoryRestricted != memoryRestricted) {
        m_memoryRestricted = memoryRestricted;
        foreach (ProcessBackendFactory *factory, m_factories) {
            factory->setMemoryRestricted(memoryRestricted);
        }
    }
}

/*!
  Return current memory restriction setting
*/

bool ProcessBackendManager::memoryRestricted() const
{
    return m_memoryRestricted;
}

/*!
   Return the current IdleDelegate object
 */

IdleDelegate *ProcessBackendManager::idleDelegate() const
{
    return m_idleDelegate;
}

/*!
   Set a new process IdleDelegate object \a idleDelegate.
   The ProcessBackendManager takes over parentage of the IdleDelegate.
 */

void ProcessBackendManager::setIdleDelegate(IdleDelegate *idleDelegate)
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
   \fn bool ProcessBackendManager::idleCpuRequest() const
   Return \c{true} if we need idle CPU cycles.
 */

/*!
  Idle CPU processing is available.  This function distributes
  the idle CPU to the first factory that has requested it.
 */

void ProcessBackendManager::idleCpuAvailable()
{
    foreach (ProcessBackendFactory *factory, m_factories) {
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

void ProcessBackendManager::updateIdleCpuRequest()
{
    bool request = false;
    foreach (ProcessBackendFactory *factory, m_factories)
        request |= factory->idleCpuRequest();

    if (request != m_idleCpuRequest) {
        m_idleCpuRequest = request;
        if (m_idleDelegate)
            m_idleDelegate->requestIdleCpu(m_idleCpuRequest);
        handleIdleCpuRequest(m_idleCpuRequest);
    }
}

/*!
  Override this function to customize your handling of Idle CPU requests.
  The \a request variable will be \c{true} if Idle CPU events are needed.
 */

void ProcessBackendManager::handleIdleCpuRequest(bool request)
{
    Q_UNUSED(request);
}

/*!
  \fn void ProcessBackendManager::idleDelegateChanged()
  Signal emitted whenever the IdleDelegate is changed.
*/

#include "moc_processbackendmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

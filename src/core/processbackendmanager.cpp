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
*/

/*!
  Construct a ProcessBackendManager with an optional \a parent
*/

ProcessBackendManager::ProcessBackendManager(QObject *parent)
    : QObject(parent)
    , m_memoryRestricted(false)
{
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
}

/*!
  Return a list of all internal processes being used by factories
*/

QList<Q_PID> ProcessBackendManager::internalProcesses()
{
    QList<Q_PID> list;
    foreach (ProcessBackendFactory *factory, m_factories)
    list.append(factory->internalProcesses());
    return list;
}

/*!
  Set memory restrictions.  If \a memoryRestricted is true
  all factories are requested to minimize memory use.
*/

void ProcessBackendManager::setMemoryRestricted(bool memoryRestricted)
{
    if (m_memoryRestricted != memoryRestricted) {
    m_memoryRestricted = memoryRestricted;
    foreach (ProcessBackendFactory *factory, m_factories)
        factory->setMemoryRestricted(memoryRestricted);
    }
}

/*!
  Return current memory restriction setting
*/

bool ProcessBackendManager::memoryRestricted() const
{
    return m_memoryRestricted;
}

#include "moc_processbackendmanager.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

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

#include "processbackendfactory.h"
#include "matchdelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class ProcessBackendFactory
  \brief The ProcessBackendFactory class is a virtual class for creating processes.

  Subclass this class to create particular types of processes.
*/

/*!
    Construct a ProcessBackendFactory with an optional \a parent.
*/

ProcessBackendFactory::ProcessBackendFactory(QObject *parent)
    : QObject(parent)
    , m_matchDelegate(NULL)
    , m_memoryRestricted(false)
{
}

/*!
    Destroy a process factory
*/

ProcessBackendFactory::~ProcessBackendFactory()
{
}

/*!
    Control memory restrictions.  Setting \a memoryRestricted to
    true requests the factory to free up as much memory as possible.
*/

void ProcessBackendFactory::setMemoryRestricted(bool memoryRestricted)
{
    if (memoryRestricted != m_memoryRestricted) {
    m_memoryRestricted = memoryRestricted;
    handleMemoryRestrictionChange();
    }
}

/*!
  Override this is subclasses to return a list of internal
  processes that are contained in this factory.  The default
  class returns an empty list.
 */

QList<Q_PID> ProcessBackendFactory::internalProcesses()
{
    return QList<Q_PID>();
}

/*!
   Override this in subclasses to handle memory restriction changes
*/

void ProcessBackendFactory::handleMemoryRestrictionChange()
{
}

/*!
   Return the current MatchDelegate object
 */

MatchDelegate *ProcessBackendFactory::matchDelegate() const
{
    return m_matchDelegate;
}

/*!
   Set a new process MatchDelegate object \a matchDelegate.
   The ProcessBackendFactory takes over parentage of the MatchDelegate.
 */

void ProcessBackendFactory::setMatchDelegate(MatchDelegate *matchDelegate)
{
    if (matchDelegate != m_matchDelegate) {
        if (m_matchDelegate)
            delete m_matchDelegate;
        m_matchDelegate = matchDelegate;
        m_matchDelegate->setParent(this);
        emit matchDelegateChanged();
    }
}

/*!
  \fn bool ProcessBackendFactory::canCreate(const ProcessInfo& info) const

  Return true if this ProcessBackendFactory matches the ProcessInfo \a info
  process binding and can create an appropriate process.  The default implementation
  delegates the decision to the MatchDelegate object.  If no MatchDelegate
  has been installed, the default implementation returns true.

  This virtual function may be overridden.
*/

bool ProcessBackendFactory::canCreate(const ProcessInfo& info) const
{
    if (m_matchDelegate)
        return m_matchDelegate->matches(info);
    return true;
}

/*!
  \fn void ProcessBackendFactory::matchDelegateChanged()

  Signal emitted whenever the MatchDelegate is changed.
*/

/*!
  \fn ProcessBackend * ProcessBackendFactory::create(const ProcessInfo& info, QObject *parent)

  Create a ProcessBackend object based on the ProcessInfo \a info and \a parent.
*/

#include "moc_processbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

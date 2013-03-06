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

#include "qprocessbackendfactory.h"
#include "qmatchdelegate.h"
#include "qrewritedelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QProcessBackendFactory
  \brief The QProcessBackendFactory class is a virtual class for creating processes.
  \inmodule QtProcessManager

  Subclass this class to create particular types of processes.
*/

/*!
    \property QProcessBackendFactory::internalProcesses
    \brief A list of the internal processes.
*/

/*!
    \property QProcessBackendFactory::matchDelegate
    \brief A QMatchDelegate object assigned to this factory.
*/

/*!
    \property QProcessBackendFactory::rewriteDelegate
    \brief A QRewriteDelegate object assigned to this factory.
*/

/*!
    \property QProcessBackendFactory::idleCpuRequest
    \brief A boolean value indicating that this factory would like idle CPU cycles
*/

/*!
    Construct a QProcessBackendFactory with an optional \a parent.
*/

QProcessBackendFactory::QProcessBackendFactory(QObject *parent)
    : QObject(parent)
    , m_matchDelegate(NULL)
    , m_rewriteDelegate(NULL)
    , m_memoryRestricted(false)
    , m_idleCpuRequest(false)
{
}

/*!
    Destroy a process factory
*/

QProcessBackendFactory::~QProcessBackendFactory()
{
}

/*!
    Control memory restrictions.  Setting \a memoryRestricted to
    true requests the factory to free up as much memory as possible.
*/

void QProcessBackendFactory::setMemoryRestricted(bool memoryRestricted)
{
    if (m_memoryRestricted != memoryRestricted) {
        m_memoryRestricted = memoryRestricted;
        handleMemoryRestrictionChange();
    }
}

/*!
  Return a list of internal processes that are in use by this factory
 */

QPidList QProcessBackendFactory::internalProcesses() const
{
    return m_internalProcesses;
}

/*!
  Set the list of internal processes.  This should be a sorted
  list.  The \a plist argument is a list of processes.
 */

void QProcessBackendFactory::setInternalProcesses(const QPidList& plist)
{
    if (!compareSortedLists(plist, m_internalProcesses)) {
        m_internalProcesses = plist;
        emit internalProcessesChanged();
    }
}

/*!
   Override this in subclasses to handle memory restriction changes
*/

void QProcessBackendFactory::handleMemoryRestrictionChange()
{
}

/*!
   Return the current QMatchDelegate object
 */

QMatchDelegate *QProcessBackendFactory::matchDelegate() const
{
    return m_matchDelegate;
}

/*!
   Set a new process QMatchDelegate object \a matchDelegate.
   The QProcessBackendFactory takes over parentage of the QMatchDelegate.
 */

void QProcessBackendFactory::setMatchDelegate(QMatchDelegate *matchDelegate)
{
    if (matchDelegate != m_matchDelegate) {
        if (m_matchDelegate)
            delete m_matchDelegate;
        m_matchDelegate = matchDelegate;
        if (m_matchDelegate)
            m_matchDelegate->setParent(this);
        emit matchDelegateChanged();
    }
}

/*!
   Return the current QRewriteDelegate object
 */

QRewriteDelegate *QProcessBackendFactory::rewriteDelegate() const
{
    return m_rewriteDelegate;
}

/*!
   Set a new process QRewriteDelegate object \a rewriteDelegate.
   The QProcessBackendFactory takes over parentage of the QRewriteDelegate.
 */

void QProcessBackendFactory::setRewriteDelegate(QRewriteDelegate *rewriteDelegate)
{
    if (rewriteDelegate != m_rewriteDelegate) {
        if (m_rewriteDelegate)
            delete m_rewriteDelegate;
        m_rewriteDelegate = rewriteDelegate;
        if (m_rewriteDelegate)
            m_rewriteDelegate->setParent(this);
        emit rewriteDelegateChanged();
    }
}

/*!
   Return true if the factory is requesting idle CPU cycles
 */

bool QProcessBackendFactory::idleCpuRequest() const
{
    return m_idleCpuRequest;
}

/*!
   Set the current idle CPU request value to \a value
 */

void QProcessBackendFactory::setIdleCpuRequest(bool value)
{
    if (value != m_idleCpuRequest) {
        m_idleCpuRequest = value;
        emit idleCpuRequestChanged();
    }
}

/*!
  This virtual function gets called when idle CPU is available.
  Subclasses should override this function.
 */

void QProcessBackendFactory::idleCpuAvailable()
{
}

/*!
  \fn bool QProcessBackendFactory::canCreate(const QProcessInfo& info) const

  Return true if this QProcessBackendFactory matches the QProcessInfo \a info
  process binding and can create an appropriate process.  The default implementation
  delegates the decision to the QMatchDelegate object.  If no QMatchDelegate
  has been installed, the default implementation returns true.

  This virtual function may be overridden.
*/

bool QProcessBackendFactory::canCreate(const QProcessInfo& info) const
{
    if (m_matchDelegate)
        return m_matchDelegate->matches(info);
    return true;
}

/*!
  \fn void QProcessBackendFactory::rewrite(QProcessInfo& info)

  Rewrites the QProcessInfo \a info object by passing it to the QRewriteDelegate
  object.  If no QRewriteDelegate object is installed, this function does nothing.

  This virtual function may be overridden.
*/

void QProcessBackendFactory::rewrite(QProcessInfo& info)
{
    if (m_rewriteDelegate)
        m_rewriteDelegate->rewrite(info);
}

/*!
  \fn void QProcessBackendFactory::internalProcessesChanged()

  Signal emitted whenever the list of internal processes has changed.
*/

/*!
  \fn void QProcessBackendFactory::matchDelegateChanged()

  Signal emitted whenever the QMatchDelegate is changed.
*/

/*!
  \fn void QProcessBackendFactory::rewriteDelegateChanged()

  Signal emitted whenever the QRewriteDelegate is changed.
*/

/*!
  \fn void QProcessBackendFactory::idleCpuRequestChanged()

  Signal emitted whenever the idle CPU request is changed
*/

/*!
  \fn QProcessBackend * QProcessBackendFactory::create(const QProcessInfo& info, QObject *parent)

  Create a QProcessBackend object based on the QProcessInfo \a info and \a parent.
*/

/*!
  \fn void QProcessBackendFactory::internalProcessError(QProcess::ProcessError error)
  This signal is emitted when an internal process has an \a error.
*/

#include "moc_qprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

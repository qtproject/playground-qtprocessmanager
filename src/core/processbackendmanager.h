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

#ifndef PROCESS_BACKEND_MANAGER_H
#define PROCESS_BACKEND_MANAGER_H

#include <QObject>
#include <QHash>
#include <QProcessEnvironment>

#include "processmanager-global.h"
#include "processlist.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessFrontend;
class ProcessInfo;
class ProcessBackendFactory;
class ProcessBackend;
class IdleDelegate;

class Q_ADDON_PROCESSMANAGER_EXPORT ProcessBackendManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(IdleDelegate* idleDelegate READ idleDelegate WRITE setIdleDelegate NOTIFY idleDelegateChanged);

public:
    explicit ProcessBackendManager(QObject *parent = 0);
    virtual ~ProcessBackendManager();

    ProcessBackend *create(const ProcessInfo& info, QObject *parent=0);
    void            addFactory(ProcessBackendFactory *factory);
    PidList         internalProcesses() const;

    void setMemoryRestricted(bool);
    bool memoryRestricted() const;

    IdleDelegate * idleDelegate() const;
    void           setIdleDelegate(IdleDelegate *);
    bool           idleCpuRequest() const { return m_idleCpuRequest; }

signals:
    void idleDelegateChanged();
    void internalProcessesChanged();
    void internalProcessError(QProcess::ProcessError);

protected:
    virtual void handleIdleCpuRequest();
    virtual void handleInternalProcessChange();

protected slots:
    void idleCpuAvailable();
    virtual void handleInternalProcessError(QProcess::ProcessError);

private slots:
    void updateIdleCpuRequest();
    void updateInternalProcesses();

private:
    QList<ProcessBackendFactory*> m_factories;
    PidList                       m_internalProcesses;
    IdleDelegate                 *m_idleDelegate;
    bool                          m_memoryRestricted;
    bool                          m_idleCpuRequest;
};

QT_END_NAMESPACE_PROCESSMANAGER

QT_PROCESSMANAGER_DECLARE_METATYPE_PTR(ProcessBackendManager)

#endif // PROCESS_BACKEND_MANAGER_H

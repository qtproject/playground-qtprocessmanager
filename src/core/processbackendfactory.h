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

#ifndef PROCESS_BACKEND_FACTORY_H
#define PROCESS_BACKEND_FACTORY_H

#include <QObject>
#include <QProcessEnvironment>

#include "processmanager-global.h"
#include "processlist.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessBackend;
class ProcessInfo;
class MatchDelegate;
class RewriteDelegate;

class Q_ADDON_PROCESSMANAGER_EXPORT ProcessBackendFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PidList internalProcesses READ internalProcesses NOTIFY internalProcessesChanged)
    Q_PROPERTY(MatchDelegate* matchDelegate READ matchDelegate WRITE setMatchDelegate NOTIFY matchDelegateChanged)
    Q_PROPERTY(RewriteDelegate* rewriteDelegate READ rewriteDelegate WRITE setRewriteDelegate NOTIFY rewriteDelegateChanged)
    Q_PROPERTY(bool idleCpuRequest READ idleCpuRequest NOTIFY idleCpuRequestChanged)

public:
    ProcessBackendFactory(QObject *parent = 0);
    virtual ~ProcessBackendFactory();
    virtual bool            canCreate(const ProcessInfo& info) const;
    virtual void            rewrite(ProcessInfo& info);
    virtual ProcessBackend *create(const ProcessInfo& info, QObject *parent) = 0;

    void              setMemoryRestricted(bool);

    PidList           internalProcesses() const;

    MatchDelegate *   matchDelegate() const;
    void              setMatchDelegate(MatchDelegate *);

    RewriteDelegate * rewriteDelegate() const;
    void              setRewriteDelegate(RewriteDelegate *);

    bool              idleCpuRequest() const;
    virtual void      idleCpuAvailable();

signals:
    void internalProcessesChanged();
    void internalProcessError(QProcess::ProcessError);
    void matchDelegateChanged();
    void rewriteDelegateChanged();
    void idleCpuRequestChanged();

protected:
    void         setIdleCpuRequest(bool);
    virtual void setInternalProcesses(const PidList&);
    virtual void handleMemoryRestrictionChange();

protected:
    PidList          m_internalProcesses;
    MatchDelegate   *m_matchDelegate;
    RewriteDelegate *m_rewriteDelegate;
    bool             m_memoryRestricted;
    bool             m_idleCpuRequest;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESS_BACKEND_FACTORY_H

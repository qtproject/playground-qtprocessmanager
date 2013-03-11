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

#ifndef PROCESS_BACKEND_FACTORY_H
#define PROCESS_BACKEND_FACTORY_H

#include <QObject>
#include <QProcessEnvironment>

#include "qprocessmanager-global.h"
#include "qprocesslist.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QProcessBackend;
class QProcessInfo;
class QMatchDelegate;
class QRewriteDelegate;

class Q_ADDON_PROCESSMANAGER_EXPORT QProcessBackendFactory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QPidList internalProcesses READ internalProcesses NOTIFY internalProcessesChanged)
    Q_PROPERTY(QMatchDelegate* matchDelegate READ matchDelegate WRITE setMatchDelegate NOTIFY matchDelegateChanged)
    Q_PROPERTY(QRewriteDelegate* rewriteDelegate READ rewriteDelegate WRITE setRewriteDelegate NOTIFY rewriteDelegateChanged)
    Q_PROPERTY(bool idleCpuRequest READ idleCpuRequest NOTIFY idleCpuRequestChanged)

public:
    QProcessBackendFactory(QObject *parent = 0);
    virtual ~QProcessBackendFactory();
    virtual bool            canCreate(const QProcessInfo& info) const;
    virtual void            rewrite(QProcessInfo& info);
    virtual QProcessBackend *create(const QProcessInfo& info, QObject *parent) = 0;

    void              setMemoryRestricted(bool);

    QPidList           internalProcesses() const;

    QMatchDelegate *   matchDelegate() const;
    void              setMatchDelegate(QMatchDelegate *);

    QRewriteDelegate * rewriteDelegate() const;
    void              setRewriteDelegate(QRewriteDelegate *);

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
    virtual void setInternalProcesses(const QPidList&);
    virtual void handleMemoryRestrictionChange();

protected:
    QPidList          m_internalProcesses;
    QMatchDelegate   *m_matchDelegate;
    QRewriteDelegate *m_rewriteDelegate;
    bool             m_memoryRestricted;
    bool             m_idleCpuRequest;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESS_BACKEND_FACTORY_H

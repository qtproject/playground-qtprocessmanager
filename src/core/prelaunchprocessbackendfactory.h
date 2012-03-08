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

#ifndef PRELAUNCH_PROCESS_BACKEND_FACTORY_H
#define PRELAUNCH_PROCESS_BACKEND_FACTORY_H

#include "processbackendfactory.h"
#include "processmanager-global.h"
#include <QTimer>

class CPULoad;

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessInfo;
class PrelaunchProcessBackend;

class Q_ADDON_PROCESSMANAGER_EXPORT PrelaunchProcessBackendFactory : public ProcessBackendFactory
{
    Q_OBJECT
    Q_PROPERTY(int launchInterval READ launchInterval WRITE setLaunchInterval NOTIFY launchIntervalChanged)
    Q_PROPERTY(ProcessInfo* processInfo READ processInfo WRITE setProcessInfo NOTIFY processInfoChanged)
    Q_PROPERTY(bool prelaunchEnabled READ prelaunchEnabled WRITE setPrelaunchEnabled NOTIFY prelaunchEnabledChanged)

public:
    PrelaunchProcessBackendFactory(QObject *parent = 0);
    virtual ~PrelaunchProcessBackendFactory();

    virtual bool canCreate(const ProcessInfo &info) const;
    virtual ProcessBackend *create(const ProcessInfo& info, QObject *parent);

    virtual QList<Q_PID>    internalProcesses();

    ProcessInfo *processInfo() const;
    void setProcessInfo(ProcessInfo *processInfo);
    void setProcessInfo(ProcessInfo& processInfo);

    int  launchInterval() const;
    void setLaunchInterval(int interval);

    bool prelaunchEnabled() const;
    void setPrelaunchEnabled(bool value);

    bool hasPrelaunchedProcess() const;

signals:
    void launchIntervalChanged();
    void processInfoChanged();
    void prelaunchEnabledChanged();
    void processPrelaunched();

protected:
    virtual void handleMemoryRestrictionChange();
    PrelaunchProcessBackend *prelaunchProcessBackend() const;

private slots:
    void timeout();
    void prelaunchFinished(int, QProcess::ExitStatus);
    void prelaunchError(QProcess::ProcessError);
    void checkCPULoadUpdated();

private:
    void startPrelaunchTimer();
    void enableCPULoadPolling(bool enable);
    void prelaunchWhenPossible();
    void disablePrelaunch();

private:
    PrelaunchProcessBackend *m_prelaunch;
    ProcessInfo             *m_info;
    bool                     m_prelaunchEnabled;
    QTimer                   m_timer;
    bool                     m_pollingCpu;
    int                      m_accu;
    int                      m_waitTime;
    int                      m_prelaunchDelay;
    CPULoad                 *m_cpuLoad;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PRELAUNCH_PROCESS_BACKEND_FACTORY_H

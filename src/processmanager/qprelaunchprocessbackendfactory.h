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

#ifndef PRELAUNCH_PROCESS_BACKEND_FACTORY_H
#define PRELAUNCH_PROCESS_BACKEND_FACTORY_H

#include "qprocessbackendfactory.h"
#include "qprocessmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QProcessInfo;
class QPrelaunchProcessBackend;

class Q_ADDON_PROCESSMANAGER_EXPORT QPrelaunchProcessBackendFactory : public QProcessBackendFactory
{
    Q_OBJECT
    Q_PROPERTY(QProcessInfo* processInfo READ processInfo WRITE setProcessInfo NOTIFY processInfoChanged)
    Q_PROPERTY(bool prelaunchEnabled READ prelaunchEnabled WRITE setPrelaunchEnabled NOTIFY prelaunchEnabledChanged)

public:
    QPrelaunchProcessBackendFactory(QObject *parent = 0);
    virtual ~QPrelaunchProcessBackendFactory();

    virtual bool canCreate(const QProcessInfo &info) const;
    virtual QProcessBackend *create(const QProcessInfo& info, QObject *parent);

    QProcessInfo *processInfo() const;
    void setProcessInfo(QProcessInfo *processInfo);
    void setProcessInfo(QProcessInfo& processInfo);

    bool prelaunchEnabled() const;
    void setPrelaunchEnabled(bool value);

    bool hasPrelaunchedProcess() const;

signals:
    void processInfoChanged();
    void prelaunchEnabledChanged();
    void processPrelaunched();

protected:
    virtual void handleMemoryRestrictionChange();
    QPrelaunchProcessBackend *prelaunchProcessBackend() const;

protected slots:
    virtual void idleCpuAvailable();

private slots:
    void prelaunchFinished(int, QProcess::ExitStatus);
    void prelaunchError(QProcess::ProcessError);
    void updateState();

private:
    QPrelaunchProcessBackend *m_prelaunch;
    QProcessInfo             *m_info;
    bool                     m_prelaunchEnabled;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PRELAUNCH_PROCESS_BACKEND_FACTORY_H

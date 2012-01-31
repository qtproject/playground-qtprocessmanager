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

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QHash>
#include <QProcessEnvironment>

#include "processmanager-global.h"
#include "process.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessFrontend;
class ProcessInfo;
class ProcessBackendFactory;
class ProcessBackendManager;
class ProcessBackend;

class ProcessManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool memoryRestricted READ memoryRestricted
               WRITE setMemoryRestricted NOTIFY memoryRestrictedChanged)
public:
    explicit ProcessManager(QObject *parent = 0);
    virtual ~ProcessManager();

    Q_INVOKABLE ProcessFrontend *create(const ProcessInfo& info);
    Q_INVOKABLE ProcessFrontend *create(const QVariantMap& info);

    Q_INVOKABLE QStringList      names() const;

    Q_INVOKABLE ProcessFrontend *processForName(const QString &name) const;
    Q_INVOKABLE ProcessFrontend *processForPID(qint64 PID) const;
    Q_INVOKABLE int              size() const;

    Q_INVOKABLE void             addBackendFactory(ProcessBackendFactory *factory);
    Q_INVOKABLE QList<Q_PID>     internalProcesses();

    void setMemoryRestricted(bool);
    bool memoryRestricted() const;

signals:
    void memoryRestrictedChanged();

protected slots:
    virtual void processFrontendAboutToStart();
    virtual void processFrontendAboutToStop();
    virtual void processFrontendStarted();
    virtual void processFrontendError(QProcess::ProcessError);
    virtual void processFrontendFinished(int, QProcess::ExitStatus);
    virtual void processFrontendStateChanged(QProcess::ProcessState);
    virtual void processFrontendDestroyed();

protected:
    virtual ProcessFrontend *createFrontend(ProcessBackend *backend);

protected:
    QList<ProcessFrontend*> m_processlist;
    ProcessBackendManager  *m_backend;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESSMANAGER_H

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

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QHash>
#include <QProcessEnvironment>

#include "qprocessmanager-global.h"
#include "qpmprocess.h"
#include "qprocesslist.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QProcessFrontend;
class QProcessInfo;
class QProcessBackendFactory;
class QProcessBackendManager;
class QProcessBackend;
class QIdleDelegate;

class Q_ADDON_PROCESSMANAGER_EXPORT QProcessManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool memoryRestricted READ memoryRestricted
               WRITE setMemoryRestricted NOTIFY memoryRestrictedChanged)
    Q_PROPERTY(QIdleDelegate* idleDelegate READ idleDelegate WRITE setIdleDelegate NOTIFY idleDelegateChanged);

public:
    explicit QProcessManager(QObject *parent = 0);
    virtual ~QProcessManager();

    Q_INVOKABLE QProcessFrontend *create(const QProcessInfo& info);
    Q_INVOKABLE QProcessFrontend *create(const QVariantMap& info);

    Q_INVOKABLE QStringList      names() const;

    Q_INVOKABLE QProcessFrontend *processForName(const QString &name) const;
    Q_INVOKABLE QProcessFrontend *processForPID(qint64 PID) const;
    Q_INVOKABLE int              size() const;

    Q_INVOKABLE void             addBackendFactory(QProcessBackendFactory *factory);
    Q_INVOKABLE QPidList          internalProcesses() const;

    void setMemoryRestricted(bool);
    bool memoryRestricted() const;

    QIdleDelegate * idleDelegate() const;
    void           setIdleDelegate(QIdleDelegate *);

signals:
    void memoryRestrictedChanged();
    void idleDelegateChanged();
    void internalProcessesChanged();
    void internalProcessError(QProcess::ProcessError);

protected slots:
    virtual void processFrontendAboutToStart();
    virtual void processFrontendAboutToStop();
    virtual void processFrontendStarted();
    virtual void processFrontendError(QProcess::ProcessError);
    virtual void processFrontendFinished(int, QProcess::ExitStatus);
    virtual void processFrontendStateChanged(QProcess::ProcessState);
    virtual void processFrontendDestroyed();

protected:
    virtual QProcessFrontend *createFrontend(QProcessBackend *backend);

protected:
    QList<QProcessFrontend*> m_processlist;
    QProcessBackendManager  *m_backend;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESSMANAGER_H

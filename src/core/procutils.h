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

#ifndef PROCUTILS_H
#define PROCUTILS_H

#include <QString>
#include <QList>
#include <QObject>

QT_FORWARD_DECLARE_CLASS(QLocalSocket)

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

struct ProcessPrivateData;

class Q_ADDON_PROCESSMANAGER_EXPORT ExecutingProcessInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(pid_t pid READ pid CONSTANT)
    Q_PROPERTY(pid_t ppid READ ppid CONSTANT)
    Q_PROPERTY(pid_t pgrp READ pgrp CONSTANT)
    Q_PROPERTY(pid_t sid READ sid CONSTANT)
    Q_PROPERTY(uid_t uid READ uid CONSTANT)
    Q_PROPERTY(uid_t euid READ euid CONSTANT)
    Q_PROPERTY(uid_t suid READ suid CONSTANT)
    Q_PROPERTY(gid_t gid READ gid CONSTANT)
    Q_PROPERTY(gid_t egid READ egid CONSTANT)
    Q_PROPERTY(gid_t sgid READ sgid CONSTANT)
    Q_PROPERTY(QList<gid_t> groups READ groups CONSTANT)
    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(int nice READ nice CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    ExecutingProcessInfo(pid_t pid);

    bool exists() const;
    pid_t pid() const;
    pid_t ppid() const;
    pid_t pgrp() const;
    pid_t sid() const;
    uid_t uid() const;
    uid_t euid() const;
    uid_t suid() const;
    gid_t gid() const;
    gid_t egid() const;
    gid_t sgid() const;
    int   priority() const;
    int   nice() const;

    QList<gid_t> groups() const { return m_groups; }
    QString      name() const { return m_name; }

private:
    struct ProcessPrivateData *m_data;
    QList<gid_t>               m_groups;
    QString                    m_name;
};

class Q_ADDON_PROCESSMANAGER_EXPORT ProcUtils
{
    ProcUtils();
    Q_DISABLE_COPY(ProcUtils)
public:
    static QString execNameForPid(qint64 pid);
    static qint64 ppidForPid(pid_t pid);
    static qint64 pidForFilename(const QString &filename);
    static qint64 pidForLocalSocket(const QLocalSocket *socket);
    static QByteArray cmdlineForPid(qint64 pid);

    static qint32 oomAdjustment(pid_t pid, bool *ok=NULL);
    static bool   setOomAdjustment(pid_t pid, qint32 oomAdjustment);

    static void   sendSignalToProcess(pid_t pid, int sig);
    static void   setPriority(pid_t pid, qint32 priority);
    static int    getThreadCount(pid_t pid);
    static QList<qint32> getThreadPriorities(pid_t pid);
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCUTILS_H

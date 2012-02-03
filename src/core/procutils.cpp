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

#include "procutils.h"

#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <QFile>
#include <QFileInfo>
#include <QLocalSocket>

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

ProcUtils::ProcUtils()
{
}

QString ProcUtils::execNameForPid(qint64 pid)
{
#if defined(Q_OS_LINUX)
    enum { BUFFERSIZE = 1024 };
    char buf[BUFFERSIZE];
    QString fn = QLatin1String("/proc/") + QString::number(pid).toLatin1() + QLatin1String("/exe");
    ssize_t len = readlink(fn.toLatin1(), buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return QString(buf);
    }
#else
    Q_UNUSED(pid);
#endif
    return QString();
}

qint64 ProcUtils::ppidForPid(pid_t pid)
{
    int ppid = 0;
#if defined(Q_OS_LINUX)
    QFile statFile(QLatin1String("/proc/") + QString::number(pid) + "/stat");
    statFile.open(QIODevice::ReadOnly);

    QByteArray contents = statFile.readAll();
    statFile.close();
    // 954 (ofono-jdb-daemo) S 1
    int readPid = 0;
    char strDummy[64];
    char state;
    sscanf(contents.constData(), "%d %s %c %d %s", &readPid, strDummy, &state, &ppid, strDummy);
#else
    Q_UNUSED(pid);
#endif
    return ppid;
}

qint64 ProcUtils::pidForFilename(const QString &filename)
{
#if defined(Q_OS_LINUX)
    QFile file(filename);
    if (!file.exists())
        return 0;
    QFileInfo fileInfo(file);

    DIR *d = opendir(QLatin1String("/proc").latin1());
    if (!d) {
        qWarning() << "failed to open proc";
        return 0;
    }

    while (dirent *dirent = readdir(d)) {
        QString dirname = QString::fromLatin1(dirent->d_name);
        bool ok = false;
        qint64 pid = dirname.toLongLong(&ok, 10);
        if (ok) {
            const QString execFilename = execNameForPid(pid);
            if (execFilename == fileInfo.fileName())
                return pid;
        }
    }
    closedir(d);
#else
    Q_UNUSED(filename);
#endif
    return 0;
}

qint64 ProcUtils::pidForLocalSocket(const QLocalSocket *socket)
{
#if defined(Q_OS_LINUX)
    if (socket) {
        struct ucred cr;
        socklen_t len = sizeof(struct ucred);
        int r = ::getsockopt(socket->socketDescriptor(), SOL_SOCKET, SO_PEERCRED, &cr, &len);
        if (r == 0)
            return (qint64)cr.pid;
    }
#else
    Q_UNUSED(socket);
#endif
    return 0;
}

QByteArray ProcUtils::cmdlineForPid(qint64 pid)
{
    QByteArray cmdline;
#if defined(Q_OS_LINUX)
    if (pid) {
        QFile file(QLatin1String("/proc/") + QString::number(pid) + QLatin1String("/cmdline"));
        file.open(QIODevice::ReadOnly);
        cmdline = file.readAll();
    }
#else
    Q_UNUSED(pid);
#endif
    return cmdline;
}

qint32 ProcUtils::oomAdjustment(pid_t pid, bool *ok)
{
    if (ok)
        *ok = false;
#if defined(Q_OS_LINUX)
    QFile file(QString::fromLatin1("/proc/%1/oom_score_adj").arg(pid));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray ba = file.read(100);
        ba.truncate(ba.count() - 1);  // Drop the '\n' at the end
        if (ba.count())
            return ba.toInt(ok);
    }
#else
    Q_UNUSED(pid);
#endif
    return -1001;
}

bool ProcUtils::setOomAdjustment(pid_t pid, qint32 oomAdjustment)
{
#if defined(Q_OS_LINUX)
    QFile file(QString::fromLatin1("/proc/%1/oom_score_adj").arg(pid));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QByteArray value = QByteArray::number(oomAdjustment) + '\n';
    if (file.write(value) == value.count())
        return true;
#else
    Q_UNUSED(pid);
    Q_UNUSED(oomAdjustment);
#endif
    return false;
}


QT_END_NAMESPACE_PROCESSMANAGER

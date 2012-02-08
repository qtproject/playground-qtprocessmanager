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


#include "unixsandboxprocess.h"
#include <sys/stat.h>

#if defined(Q_OS_LINUX)
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#endif
#include <pwd.h>

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class UnixSandboxProcess
  \brief The UnixSandboxProcess class sets the UID and GID on a Unix process
*/

/*!
  Construct a UnixProcessBackend with \a uid, \a gid, and optional \a parent.
*/

UnixSandboxProcess::UnixSandboxProcess(qint64 uid, qint64 gid, QObject *parent)
    : QProcess(parent)
    , m_uid(uid)
    , m_gid(gid)
{
}

/*!
  \internal Set up the user and group id
*/

void UnixSandboxProcess::setupChildProcess()
{
    // qDebug() << "Setting up child process" << m_uid << m_gid;
    if (m_gid >= 0)
        ::setgid(m_gid);
    if (m_uid >= 0)
        ::setuid(m_uid);
    ::umask(S_IWGRP | S_IWOTH);
    ::setpgid(0,0);

    struct passwd * pw = getpwent();
    if (pw)
        ::initgroups(pw->pw_name, pw->pw_gid);
    else {
        qWarning() << "Unable to find UID" << ::getuid() << "to set groups";
        ::setgroups(0,0);
    }
}

#include "moc_unixsandboxprocess.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

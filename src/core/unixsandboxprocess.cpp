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
#include <errno.h>

#if defined(Q_OS_LINUX)
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <sys/prctl.h>
#if !defined(Q_OS_LINUX_ANDROID)
#include <sys/capability.h>
#endif
#include <signal.h>
#endif
#include <pwd.h>

#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class UnixSandboxProcess
  \brief The UnixSandboxProcess class sets the UID and GID on a Unix process
*/

/*!
  Construct a UnixProcessBackend with \a uid, \a gid, \a umask, and optional \a parent.
*/

UnixSandboxProcess::UnixSandboxProcess(qint64 uid, qint64 gid, qint64 umask, qint64 dropCapabilities, QObject *parent)
    : QProcess(parent)
    , m_uid(uid)
    , m_gid(gid)
    , m_umask(umask)
    , m_dropCapabilities(dropCapabilities)
{
}

/*!
  Set up child process UID, GID, and supplementary group list.
  Also set the child process to be in its own process group and fix the umask
  and the POSIX capabilities to drop.
  Under Linux, the child process will be set to receive a SIGTERM signal
  when the parent process dies.

  The creator of the child process may have specified a UID and/or a
  GID for the child process.  Here are the currently supported cases:

  \table
  \header
    \li UID
    \li GID
    \li Result

  \row
    \li Invalid
    \li Invalid
    \li The child runs with the same UID and GID as the parent
  \row
    \li \b{Valid}
    \li Invalid
    \li The UID is looked up from \c{/etc/passwd}.  If it is
        found, the UID and GID of the child are set from the values
        found.  The supplemetary group list is initialized from
        \c{/etc/groups}.  If the UID is not found, the child is killed.
  \row
    \li Invalid
    \li \b{Valid}
    \li The child's GID is set.  The supplementary
        group list is cleared.
  \row
    \li \b{Valid}
    \li \b{Valid}
    \li The child's UID and GID are set to the values specified.  If
        they happen to match the values in \c{/etc/passwd}
        the supplementary group list is set as well.  If they don't
        match, the supplementary group list is cleared.

  \endtable

  In the "normal" use case, the calling process will set the UID and the
  child process will automatically get the correct GID and
  supplementary group list by looking up information from \c{/etc/passwd}.

  In the "alternative" use cases, you can set both the UID/GID or you
  can set just the GID.  These cases are designed for running
  processes that have a UID and/or GID that doesn't exist in the
  \c{/etc/passwd} database.

*/

void UnixSandboxProcess::setupChildProcess()
{
#if defined(Q_OS_LINUX)
    if (::prctl(PR_SET_PDEATHSIG, SIGTERM))
        qFatal("UnixSandboxProcess prctl unable to set death signal: %s", strerror(errno));
#endif
    if (::setpgid(0,0))
        qFatal("UnixSandboxProcess setpgid(): %s", strerror(errno));

    if (m_umask >= 0) {
        mode_t umask = m_umask;
        ::umask(umask);
    }

    if (m_uid >= 0) {
        errno = 0;
        struct passwd *pw = ::getpwuid(m_uid);
        if (!pw && errno)
            qFatal("UnixSandboxProcess getpwuid(%ld): %s", (long) m_uid, strerror(errno));

        gid_t gid = m_gid;
        if (m_gid < 0) {   // UID set, GID unset
            if (!pw)
                qFatal("UnixSandboxProcess did not find uid %ld in database", (long) m_uid);
            if (::initgroups(pw->pw_name, pw->pw_gid))
                qFatal("UnixSandboxProcess initgroups(%s, %ld): %s", pw->pw_name, (long) pw->pw_gid, strerror(errno));
            gid = pw->pw_gid;
        }
        else {  // UID set, GID set
            if (pw && pw->pw_gid == m_gid) {
                if (::initgroups(pw->pw_name, pw->pw_gid))
                    qFatal("UnixSandboxProcess initgroups(%s, %ld): %s", pw->pw_name, (long) pw->pw_gid, strerror(errno));
            } else {
                if (::setgroups(0, NULL))
                    qFatal("UnixSandboxProcess setgroups(0,NULL): %s", strerror(errno));
            }
        }
        if (::setgid(gid))
            qFatal("UnixSandboxProcess setgid(%ld): %s", (long) gid, strerror(errno));
        if (::setuid(m_uid))
            qFatal("UnixSandboxProcess setuid(%ld): %s", (long) m_uid, strerror(errno));
    }
    else if (m_gid >=0) {   // UID unset, GID set
        if (::setgroups(0, NULL))
            qFatal("UnixSandboxProcess setgroups(0,NULL): %s", strerror(errno));
        if (::setgid(m_gid))
            qFatal("UnixSandboxProcess setgid(%ld): %s", (long) m_gid, strerror(errno));
    }


#if defined (Q_OS_LINUX) && !defined(Q_OS_LINUX_ANDROID)
    if (m_dropCapabilities >= 0) {
        cap_t caps;
        cap_value_t cap_list[63];
        quint64 bit = 1;
        int n = 0;
        for (int i = 0; i < 63; ++i) {
            if (m_dropCapabilities & (bit << i))
                cap_list[n++] = i;
        }
        caps = ::cap_get_proc();
        ::cap_set_flag(caps, CAP_EFFECTIVE, n, cap_list, CAP_CLEAR);
        ::cap_set_flag(caps, CAP_INHERITABLE, n, cap_list, CAP_CLEAR);
        ::cap_set_flag(caps, CAP_PERMITTED, n, cap_list, CAP_CLEAR);
        if (::cap_set_proc(caps) == -1)
            qFatal("UnixSandboxProcess cap_set_proc (%ld): %s", (long) m_dropCapabilities, strerror(errno));
        ::cap_free(caps);
    }
#endif
}

#include "moc_unixsandboxprocess.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

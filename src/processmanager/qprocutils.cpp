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

#include "qprocutils.h"

#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>
#include <QDebug>

#if defined(Q_OS_LINUX)
#include <QRegExp>
#elif defined(Q_OS_MAC)
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_vm.h>
#include <mach/thread_info.h>
#endif


QT_BEGIN_NAMESPACE_PROCESSMANAGER

struct ProcessPrivateData {
    pid_t pid, ppid, pgrp, sid;
    uid_t uid, euid, suid;
    gid_t gid, egid, sgid;
    int   priority, nice;
};

/*!
  Construct a ExecutingProcessInfo object from \a pid.
 */

QExecutingProcessInfo::QExecutingProcessInfo(pid_t pid)
    : m_data(0)
{
#if defined(Q_OS_LINUX)
    static QRegExp statFile(QStringLiteral("^(\\d+) \\(.*\\) [RSDZTW] (\\d+) (\\d+) (\\d+)(?: \\d+){11} (\\d+) (\\d+)"));
    /*
       Index, name, format, description (from man 5 page for proc)
       0  pid %d        The process ID.
       1  name (%s)     The process name, surrounded by parenthesis
       2  state %c      One of R (running), S (sleeping), D (disk sleep), ...
       3  ppid %d       The PID of the parent.
       4  pgrp %d       The process group ID of the process.
       5  session %d    The session ID of the process.
       17 priority %ld  For processes running under a non-real-time scheduling policy, this is
                        the raw nice value (setpriority(2)) as represented in the kernel.
                        The kernel stores nice values as numbers in the range 0 (high) to 39 (low),
                        corresponding to the user- visible nice range of -20 to 19.
       18 nice %ld      The nice value (see setpriority(2)), a value in the range
                        19 (low priority) to -20 (high priority).
     */

    QFile file(QString::fromLatin1("/proc/%1/stat").arg(pid));
    if (!file.open(QIODevice::ReadOnly))
        return;

    m_data = new ProcessPrivateData;
    memset(m_data, 0, sizeof(ProcessPrivateData));
    QByteArray contents = file.readAll();
    file.close();

    if (statFile.indexIn(QString::fromLocal8Bit(contents)) != 0) {
        qWarning("Did not match pid=%d", pid);
        return;
    }

    m_data->pid      = statFile.cap(1).toInt();
    m_data->ppid     = statFile.cap(2).toInt();
    m_data->pgrp     = statFile.cap(3).toInt();
    m_data->sid      = statFile.cap(4).toInt();
    m_data->priority = statFile.cap(5).toLong();
    m_data->nice     = statFile.cap(6).toLong();

    /* Some of the contents (from kernel documentation proc.txt)
       Field       Content
       Name        filename of the executable
       Pid         process id
       PPid        process id of the parent process
       Uid         Real, effective, saved set, and  file system UIDs
       Gid         Real, effective, saved set, and  file system GIDs
       Groups      supplementary group list
         ...
     */
    QFile status(QString::fromLatin1("/proc/%1/status").arg(pid));
    if (!status.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open status file for pid=%d", pid);
        return;
    }

    char   buf[256];
    qint64 len;
    while ((len = status.readLine(buf, 256)) != -1) {
        if (!strncmp("Name:", buf, 5))
            m_name = QString::fromLocal8Bit(buf + 6, len-6);
        else if (!strncmp("Uid:", buf, 4)) {
            QList<QByteArray> uids = QByteArray(buf + 5, len-5).split('\t');
            m_data->uid  = uids.at(0).toInt();
            m_data->euid = uids.at(1).toInt();
            m_data->suid = uids.at(2).toInt();
        }
        else if (!strncmp("Gid:", buf, 4)) {
            QList<QByteArray> gids = QByteArray(buf + 5, len-5).split('\t');
            m_data->gid  = gids.at(0).toInt();
            m_data->egid = gids.at(1).toInt();
            m_data->sgid = gids.at(2).toInt();
        }
        else if (!strncmp("Groups:", buf, 7)) {
            QList<QByteArray> groups = QByteArray(buf+8, len-8).split(' ');
            while (groups.size()) {
                bool ok;
                gid_t group = groups.takeFirst().toInt(&ok);
                if (ok)
                    m_groups.append(group);
            }
        }
    }
    status.close();
#elif defined(Q_OS_MAC)
    int    name[4];
    size_t bufferSize;
    name[0] = CTL_KERN;
    name[1] = KERN_PROC;
    name[2] = KERN_PROC_PID;
    name[3] = pid;

    if (::sysctl(name, 4, NULL, &bufferSize, NULL, 0) < 0)
        return;
    struct kinfo_proc *kinfo = (struct kinfo_proc *) malloc(bufferSize);
    if (!kinfo)
        return;
    if (::sysctl(name, 4, kinfo, &bufferSize, NULL, 0) < 0) {
        free(kinfo);
        return;
    }
    int n = bufferSize / sizeof(struct kinfo_proc);
    if (n != 1) {
        free(kinfo);
        return;
    }

    m_data = new ProcessPrivateData;
    memset(m_data, 0, sizeof(ProcessPrivateData));

    // Proc data
    m_data->pid  = kinfo->kp_proc.p_pid;
    m_data->priority = kinfo->kp_proc.p_priority;    // Process priority (u_char)
    m_data->nice     = kinfo->kp_proc.p_nice;        // Process 'nice' value (char)

    // Effective proc data
    m_data->ppid = kinfo->kp_eproc.e_ppid;  // Parent process id
    m_data->pgrp = kinfo->kp_eproc.e_pgid;  // Process group id
    m_data->uid  = kinfo->kp_eproc.e_pcred.p_ruid;   // Real user ID
    m_data->suid = kinfo->kp_eproc.e_pcred.p_svuid;  // Saved user ID
    m_data->gid  = kinfo->kp_eproc.e_pcred.p_rgid;   // Real group ID
    m_data->sgid = kinfo->kp_eproc.e_pcred.p_svgid;   // Saved group ID

    // Credentials
    m_data->euid = kinfo->kp_eproc.e_ucred.cr_uid;   // Effective user ID
    for (int i = 0 ; i < kinfo->kp_eproc.e_ucred.cr_ngroups ; i++)
        m_groups.append(kinfo->kp_eproc.e_ucred.cr_groups[i]);
    m_data->egid = m_groups.at(0);   // I believe this is correct
    free(kinfo);

    // Session
    m_data->sid = ::getsid(m_data->pid);

    // Name requires two additional sysctl calls
    name[0] = CTL_KERN;
    name[1] = KERN_ARGMAX;
    int maxArgs;
    bufferSize = sizeof(maxArgs);
    if (::sysctl(name, 2, &maxArgs, &bufferSize, NULL, 0) == -1)
        return;
    char *args = (char *) malloc(maxArgs);
    if (!args)
        return;

    name[0] = CTL_KERN;
    name[1] = KERN_PROCARGS2;
    name[2] = pid;
    bufferSize = (size_t) maxArgs;
    if (::sysctl(name, 3, args, &bufferSize, NULL, 0) == -1) {
        free(args);
        return;
    }
    char *p = args + sizeof(int);  // First few bytes are the number of arguments
    char *p2 = strrchr(p, '/');
    if (p2)
        m_name = QString::fromLocal8Bit(p2+1);
    else
        m_name = QString::fromLocal8Bit(p);
    free(args);
#endif
}

bool  QExecutingProcessInfo::exists() const { return (m_data != 0); }
pid_t QExecutingProcessInfo::pid() const  { return (m_data ? m_data->pid : 0); }
pid_t QExecutingProcessInfo::ppid() const { return (m_data ? m_data->ppid : 0); }
pid_t QExecutingProcessInfo::pgrp() const { return (m_data ? m_data->pgrp : 0); }
pid_t QExecutingProcessInfo::sid() const { return (m_data ? m_data->sid : 0); }
uid_t QExecutingProcessInfo::uid() const  { return (m_data ? m_data->uid : 0); }
uid_t QExecutingProcessInfo::euid() const { return (m_data ? m_data->euid : 0); }
uid_t QExecutingProcessInfo::suid() const { return (m_data ? m_data->suid : 0); }
gid_t QExecutingProcessInfo::gid() const  { return (m_data ? m_data->gid : 0); }
gid_t QExecutingProcessInfo::egid() const { return (m_data ? m_data->egid : 0); }
gid_t QExecutingProcessInfo::sgid() const { return (m_data ? m_data->sgid : 0); }
int   QExecutingProcessInfo::priority() const { return (m_data ? m_data->priority : 0); }
int   QExecutingProcessInfo::nice() const { return (m_data ? m_data->nice : 0); }


QProcUtils::QProcUtils()
{
}


/*!
  Note: Under MacOSX, we need to use sysctl with CTL_KERN, KERN_PROC, KERN_PROCARGS2, pid
  which returns a buffer containing the process name, all arguments, all environment variables
  and sometimes other stuff.
 */

QString QProcUtils::execNameForPid(qint64 pid)
{
#if defined(Q_OS_LINUX)
    enum { BUFFERSIZE = 1024 };
    char buf[BUFFERSIZE];
    QString fn = QLatin1String("/proc/") + QString::number(pid) + QLatin1String("/exe");
    ssize_t len = readlink(fn.toLatin1(), buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return QString::fromLocal8Bit(buf);
    }
#else
    Q_UNUSED(pid);
#endif
    return QString();
}

qint64 QProcUtils::ppidForPid(pid_t pid)
{
    int ppid = 0;
#if defined(Q_OS_LINUX)
    QFile statFile(QLatin1String("/proc/") + QString::number(pid) + QStringLiteral("/stat"));
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

qint64 QProcUtils::pidForFilename(const QString &filename)
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

qint64 QProcUtils::pidForLocalSocket(const QLocalSocket *socket)
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

QByteArray QProcUtils::cmdlineForPid(qint64 pid)
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

qint32 QProcUtils::oomAdjustment(pid_t pid, bool *ok)
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

bool QProcUtils::setOomAdjustment(pid_t pid, qint32 oomAdjustment)
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

/*!
  Send a signal to a process or process group.
 */

void QProcUtils::sendSignalToProcess(pid_t pid, int sig)
{
    pid_t pgrp = ::getpgid(pid);
    if (pgrp != -1 && pgrp != ::getpgrp() && ::killpg(pgrp, sig) == 0)
        return;

    qWarning("Unable terminate process group: %d, directly killing process %d", pgrp, pid);
    ::kill(pid, sig);
}

/*!
  Set a process or process group priority
 */

void QProcUtils::setPriority(pid_t pid, qint32 priority)
{
    pid_t pgrp = ::getpgid(pid);
    if (pgrp != -1 && pgrp != ::getpgrp()) {
        if (::setpriority(PRIO_PGRP, pgrp, priority) == 0)
            return;
        qErrnoWarning(errno, "Failed to set process group %d priority to %d", pgrp, priority);
    }
    if (::setpriority(PRIO_PROCESS, pid, priority))
        qErrnoWarning(errno, "Failed to set process %d priority to %d", pid, priority);
}

/*!
  Return a count of the number of threads in a process

  Under Mach this requires some pretty strong permissions.  Under Linux, anyone
  can read the number of threads.
 */

int QProcUtils::getThreadCount(pid_t pid)
{
#if defined(Q_OS_LINUX)
    QDir pdir(QString::fromLatin1("/proc/%1/task").arg(pid));
    return pdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).size();
#elif defined(Q_OS_MAC)
    mach_port_t task;
    thread_act_array_t threads;
    mach_msg_type_number_t thread_count;
    kern_return_t err = task_for_pid(mach_task_self(), pid, &task);
    if (err != KERN_SUCCESS) {
        qWarning("Unable to convert pid to task: %d", err);
        return 1;
    }
    err = task_threads(task, &threads, &thread_count);
    if (err != KERN_SUCCESS) {
        qWarning("Call to task_threads failed: %d", err);
        mach_port_deallocate(mach_task_self(), task);
    }
    qDebug("Found task with %d threads", thread_count);
    mach_port_deallocate(mach_task_self(), task);
    err = mach_vm_deallocate(mach_task_self(), (mach_vm_address_t) threads,
                        sizeof(*threads) * thread_count);
    if (err != KERN_SUCCESS)
        qWarning("Troubling freeing thread list");
    return thread_count;
#endif
    return 1;
}

/*!
  Return a list of priorities, one per thread in the process.  This function
  returns the "user visible" priority, a number from -20 to +19.
 */

QList<qint32> QProcUtils::getThreadPriorities(pid_t pid)
{
    QList<qint32> plist;
#if defined(Q_OS_LINUX)
    static QRegExp statFile(QStringLiteral("^(\\d+) \\(.*\\) [RSDZTW] (\\d+) (\\d+) (\\d+)(?: \\d+){11} (\\d+) (\\d+)"));

    QDir pdir(QString::fromLatin1("/proc/%1/task").arg(pid));
    foreach (const QString& entry, pdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QFile file(QString::fromLatin1("%1/%2/stat").arg(pdir.path()).arg(entry));
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Unable to read thread stat" << file.fileName();
            return plist;
        }
        QByteArray contents = file.readAll();
        if (statFile.indexIn(QString::fromLocal8Bit(contents)) != 0) {
            qWarning("Did not match stat file expression");
            return plist;
        }
        plist << statFile.cap(6).toLong();
    }
#endif
    return plist;
}


#include "moc_qprocutils.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

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

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QMap>
#include <QtEndian>
#include <QFile>
#include <QElapsedTimer>
#include <QProcess>

#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>

// Linux only?
#include <sys/wait.h>
#include <grp.h>

#include "forklauncher.h"
#include "remoteprotocol.h"
#include "processinfo.h"
#include "procutils.h"

#if defined(Q_OS_LINUX)
#include <sys/prctl.h>
#endif

#if defined(Q_OS_MAC) && !defined(QT_NO_CORESERVICES)
// Shared libraries don't have direct access to environ until runtime
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#else
  extern char **environ;
#endif


QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \headerfile <forklauncher.h>
  \title Forklauncher
  \ingroup funclists

  \brief The <forklauncher.h> file provides the \l{forklauncher()} function
  to convert a runtime into a forking runtime factory.
 */

/*!
  \fn void forklauncher(int *argc, char ***argv)
  \relates <forklauncher.h>
  \brief The fork launcher class converts any standard runtime object
         into remote process backend that forks off new children.

  In many cases the ProcessManager class is repeatedly asked to start
  the same program (for example, a QML runtime environment).  Rather
  than launch the process from scratch, it is convenient to launch the
  process just once, do the dynamic symbol resolution and any stateless
  initialization, and then fork off copies of it self each time the process
  is being started.

  The forklauncher functions makes it easy to convert any program into
  a self-forking parent process.  Simply call \c{forklauncher()} just
  after the program has started and initialized any stateless global
  values.  You must pass a pointer to \a argc and a pointer to \a
  argv.  For example:

  \code
  // Program "myForkProgram"
  int main(int argc, char **argv)
  {
    forklauncher(&argc, &argv);
    QCoreApplication app(argc, argv);
    MyAppStuff stuff;
    return app.exec();
  }
  \endcode

  The \c{forklauncher} function grabs control of the process and listens
  on STDIN for JSON-formatted messages that follow the remote protocol.
  This matches logically with the \l{PipeProcessBackendFactory} class, which
  spawns off a process and expects to talk with that process over the
  STDIN/STDOUT pair connected to that process.  So your server code
  looks something like:

  \code
  ProcessBackendManager *manager = new ProcessBackendManager;
  ProcessInfo info;
  info.setValue("program", "myForkProgram");
  PipeProcessBackendFactory *factory = new PipeProcessBackendFactory;
  factory->setProcessInfo(info);
  manager->addFactory(factory);
  \endcode

  Any processes creation requests to the \l{PipeProcessBackendFactory} will
  be sent to \c{myForkProgram}.  The \c{forkLauncher()} function will
  receive the process creation request, call \c{fork()} to create a child
  process, and then return from the \c{forkLauncher()} function call
  in the child process.  Which means that the \c{myForkProgram} will
  run the rest of the program as written, and never realize that it is
  a forked copy of a master program which is still listening to the
  \l{PipeProcessBackendFactory}.

  The process creation routine passes a complete \l{ProcessInfo} record,
  including program name and command line arguments.  This program name
  and these command line arguments are written into the child's \c{argc},
  and \c{argv} variables.  Just remember, you can only access the child's
  command line arguments \e{after} you return from \c{forklauncher()}

*/

static int sig_child_pipe[2];
static struct sigaction old_sig_child_handler;

static void sig_child_handler(int sig)
{
    ::write(sig_child_pipe[1], "@", 1);

    // Complicated way of calling the old child handler
    void (*oldAction)(int) = ((volatile struct sigaction *)&old_sig_child_handler)->sa_handler;
    if (oldAction && oldAction != SIG_IGN)
        oldAction(sig);
}

static void readToBuffer(int fd, QByteArray& buffer)
{
    const int bufsize = 1024;
    uint oldSize = buffer.size();
    buffer.resize(oldSize + bufsize);
    int n = ::read(fd, buffer.data()+oldSize, bufsize);
    if (n > 0)
        buffer.resize(oldSize+n);
    else
        buffer.resize(oldSize);
}

static void writeFromBuffer(int fd, QByteArray& buffer)
{
    int n = ::write(fd, buffer.data(), buffer.size());
    if (n == -1) {
        qDebug() << "Failed to write to " << fd;
        exit(-1);
    }
    if (n < buffer.size())
        buffer = buffer.mid(n);
    else
        buffer.clear();
}

static void copyToOutgoing(QByteArray& outgoing, const QString& channel, QByteArray& buf, int id)
{
    QJsonObject message;
    message.insert(RemoteProtocol::event(), RemoteProtocol::output());
    message.insert(RemoteProtocol::id(), id);
    message.insert(channel, QString::fromLocal8Bit(buf.data(), buf.size()));

    outgoing.append(QJsonDocument(message).toBinaryData());
    buf.clear();
}

/*
  Update the current process to match the information in info
 */

static void fixProcessState(const ProcessInfo& info, int *argc_ptr, char ***argv_ptr)
{
    // Fix the UID & GID values
    ::setpgid(0,0);
    if (info.contains(ProcessInfoConstants::Gid))
        ::setgid(info.gid());
    if (info.contains(ProcessInfoConstants::Uid))
        ::setuid(info.uid());
    ::umask(S_IWGRP | S_IWOTH);
    struct passwd *pw = getpwent();
    if (pw)
        ::initgroups(pw->pw_name, pw->pw_gid);
    else
        ::setgroups(0,0);

    if (info.contains(ProcessInfoConstants::Priority)) {
        int priority = info.priority();
        if (::setpriority(PRIO_PROCESS, ::getpid(), priority) == -1)
            qWarning("Unable to set priority of pid=%d to %d", ::getpid(), priority);
    }

    if (info.contains(ProcessInfoConstants::OomAdjustment)) {
        int adj = info.oomAdjustment();
        if (!ProcUtils::setOomAdjustment(::getpid(), adj))
            qWarning("Unable to set oom adjustment of pid=%d to %d", ::getpid(), adj);
    }

    if (info.contains(ProcessInfoConstants::WorkingDirectory)) {
        QByteArray wd = QFile::encodeName(info.workingDirectory());
        if (!::chdir(wd.constData()))
            qWarning("Unable to chdir to %s", wd.constData());
    }

    // Fix the environment
    if (info.contains(ProcessInfoConstants::Environment)) {
        const char *entry;
        QList<QByteArray> envlist;
        for (int count = 0 ; (entry = environ[count]) ; ++count) {
            const char *equal = strchr(entry, '=');
            if (!equal)
                envlist.append(QByteArray(entry));
            else
                envlist.append(QByteArray(entry, equal - entry));
        }

        QVariantMap env = info.environment();
        foreach (const QByteArray& ba, envlist) {
            QString key = QString::fromLocal8Bit(ba.constData(), ba.size());
            if (!env.contains(key))   // Remove only keys that don't exist in the planned environment
                ::unsetenv(ba.constData());
        }

        QMapIterator<QString, QVariant> iter(env);
        while (iter.hasNext()) {
            iter.next();
            QByteArray key   = iter.key().toLocal8Bit();
            QByteArray value = iter.value().toByteArray();
            ::setenv(key.constData(), value.constData(), 1);
        }
    }

    // Fix up the argument list
    if (info.contains(ProcessInfoConstants::Arguments)) {
        const int argc = info.arguments().size() + 1;
        char **argv = new char *[argc + 1];
        if (info.contains(ProcessInfoConstants::Program))
            argv[0] = strdup(info.program().toLocal8Bit().constData());
        else
            argv[0] = (*argv_ptr)[0];
        for (int i = 1 ; i < argc ; i++ )
            argv[i] = strdup(info.arguments().at(i-1).toLocal8Bit().constData());
        argv[argc] = 0;
        *argc_ptr = argc;
        *argv_ptr = argv;
    }
    else if (info.contains(ProcessInfoConstants::Program)) {
        // No new arguments; just copy in the new program name
        (*argv_ptr)[0] = strdup(info.program().toLocal8Bit().constData());
    }
}


/**************************************************************************/

// ### TODO:  Should we watch for 'startOutputPattern'???

class ChildProcess {
public:
    ChildProcess(int id);
    ~ChildProcess();

    int  updateFdSet(int n, fd_set& rfds, fd_set& wfds);
    void processFdSet(QByteArray& outgoing, fd_set& rfds, fd_set& wfds);
    void stop(int timeout);
    void setPriority(int priority);
    void setOomAdjustment(int oomAdjustment);
    bool doFork();

    void  write(const QByteArray& buf) { m_inbuf.append(buf); }
    pid_t pid() const { return m_pid; }
    int   id() const { return m_id; }
    bool needTimeout() const { return m_state == SentSigTerm; }

    void sendStateChanged(QByteArray& outgoing, QProcess::ProcessState state);
    void sendStarted(QByteArray& outgoing);
    void sendFinished(QByteArray& outgoing, int exitCode, QProcess::ExitStatus);
    void sendError(QByteArray& outgoing, QProcess::ProcessError err, const QString& errString);

    enum ProcessState {
        NotRunning,
        Running,
        SentSigTerm,
        SentSigKill,
        Finished
    };
private:
    ProcessState m_state;
    pid_t m_pid;
    int   m_id;  // Unique id for this process
    QElapsedTimer m_timer;
    int m_timeout;
    int m_stdin, m_stdout, m_stderr;
    QByteArray m_inbuf;  // Data being written
    QByteArray m_outbuf; // Data being read
    QByteArray m_errbuf; // Data being read
};

ChildProcess::ChildProcess(int id)
    : m_state(NotRunning)
    , m_pid(-1)
    , m_id(id)
    , m_stdin(-1)
    , m_stdout(-1)
    , m_stderr(-1)
{
}

ChildProcess::~ChildProcess()
{
    if (m_stdin > 0)  close(m_stdin);
    if (m_stdout > 0) close(m_stdout);
    if (m_stderr > 0) close(m_stderr);
}

int ChildProcess::updateFdSet(int n, fd_set& rfds, fd_set& wfds)
{
    FD_SET(m_stdout, &rfds);
    int n2 = qMax(n, m_stdout);
    FD_SET(m_stderr, &rfds);
    n2 = qMax(n2, m_stderr);
    if (m_inbuf.size()) {
        FD_SET(m_stdin, &wfds);
        n2 = qMax(n2, m_stdin);
    }
    return n2;
}

void ChildProcess::processFdSet(QByteArray& outgoing, fd_set& rfds, fd_set& wfds)
{
    if (FD_ISSET(m_stdin, &wfds)) {   // Data to write
        writeFromBuffer(m_stdin, m_inbuf);
    }
    if (FD_ISSET(m_stdout, &rfds)) {  // Data to read
        readToBuffer(m_stdout, m_outbuf);
        if (m_outbuf.size())
            copyToOutgoing(outgoing, RemoteProtocol::stdout(), m_outbuf, m_id);
    }
    if (FD_ISSET(m_stderr, &rfds)) {  // Data to read
        readToBuffer(m_stderr, m_errbuf);
        if (m_errbuf.size())
            copyToOutgoing(outgoing, RemoteProtocol::stderr(), m_errbuf, m_id);
    }
    if (m_state == SentSigTerm && m_timer.hasExpired(m_timeout)) {
        m_state = SentSigKill;
        ProcUtils::sendSignalToProcess(m_pid, SIGKILL);
    }
}

/*
  Stop the child process from running.  Pass in a timeout value in milliseconds.
 */

void ChildProcess::stop(int timeout)
{
    if (m_state == Running) {
        // ### TODO:  Kill by progress group...
        if (timeout > 0) {
            m_state = SentSigTerm;
            m_timeout = timeout;
            m_timer.start();
            ProcUtils::sendSignalToProcess(m_pid, SIGTERM);
        }
        else {
            m_state = SentSigKill;
            ProcUtils::sendSignalToProcess(m_pid, SIGKILL);
        }
    }
}

void ChildProcess::setPriority(int priority)
{
    if (::setpriority(PRIO_PROCESS, m_pid, priority) == -1)
        qWarning("Unable to set priority of pid=%d to %d", m_pid, priority);
}

void ChildProcess::setOomAdjustment(int oomAdjustment)
{
    if (!ProcUtils::setOomAdjustment(m_pid, oomAdjustment))
        qWarning("Unable to set oom adjustment of pid=%d to %d", m_pid, oomAdjustment);
}

static void makePipe(int fd[])
{
    if (::pipe(fd) == -1)
        qFatal("Unable to create pipe: %s", strerror(errno));
    if (::fcntl(fd[0], F_SETFL, O_NONBLOCK) == -1)  // Set non-block on read end
        qFatal("Unable to set nonblocking: %s", strerror(errno));
}

bool ChildProcess::doFork()
{
    m_state = Running;

    int fd1[2];  // Stdin of the child
    int fd2[2];  // Stdout of the child
    int fd3[2];  // Stderr of the child
    makePipe(fd1);
    makePipe(fd2);
    makePipe(fd3);

    m_pid = fork();
    if (m_pid < 0)  // failed to fork
        qFatal("Failed to fork: %s", strerror(errno));

    if (m_pid == 0) {  // child
        dup2(fd1[0], STDIN_FILENO);   // Duplicate input side of pipe to stdin
        dup2(fd2[1], STDOUT_FILENO);  // Duplicate output side of the pipe to stdout
        dup2(fd3[1], STDERR_FILENO);  // Duplicate output side of the pipe to stderr
        // Close all of the original pipes
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        close(fd3[0]);
        close(fd3[1]);
#if defined(Q_OS_LINUX)
            ::prctl(PR_SET_PDEATHSIG, SIGTERM);  // Ask to be killed when parent dies
#endif
        return true;
    }

    // Execute parent code here....
    m_stdin = fd1[1];
    m_stdout = fd2[0];
    m_stderr = fd3[0];
    close(fd1[0]);
    close(fd2[1]);
    close(fd3[1]);
    return false;   // Parent returns false
}

void ChildProcess::sendStateChanged(QByteArray& outgoing, QProcess::ProcessState state)
{
    QJsonObject msg;
    msg.insert(RemoteProtocol::event(), RemoteProtocol::stateChanged());
    msg.insert(RemoteProtocol::id(), m_id);
    msg.insert(RemoteProtocol::stateChanged(), state);
    outgoing.append(QJsonDocument(msg).toBinaryData());
}

void ChildProcess::sendStarted(QByteArray& outgoing)
{
    QJsonObject msg;
    msg.insert(RemoteProtocol::event(), RemoteProtocol::started());
    msg.insert(RemoteProtocol::id(), m_id);
    msg.insert(RemoteProtocol::pid(), m_pid);
    outgoing.append(QJsonDocument(msg).toBinaryData());
}

void ChildProcess::sendFinished(QByteArray& outgoing, int exitCode, QProcess::ExitStatus exitStatus)
{
    QJsonObject msg;
    msg.insert(RemoteProtocol::event(), RemoteProtocol::finished());
    msg.insert(RemoteProtocol::id(), m_id);
    msg.insert(RemoteProtocol::exitCode(), exitCode);
    msg.insert(RemoteProtocol::exitStatus(), exitStatus);
    outgoing.append(QJsonDocument(msg).toBinaryData());
}

void ChildProcess::sendError(QByteArray& outgoing, QProcess::ProcessError err, const QString& errString)
{
    QJsonObject msg;
    msg.insert(RemoteProtocol::event(), RemoteProtocol::error());
    msg.insert(RemoteProtocol::id(), m_id);
    msg.insert(RemoteProtocol::error(), err);
    msg.insert(RemoteProtocol::errorString(), errString);
    outgoing.append(QJsonDocument(msg).toBinaryData());
}


/**************************************************************************/

class ParentProcess {
public:
    ParentProcess(int *argc, char ***argv);
    ~ParentProcess();

    ChildProcess *childFromPid(pid_t pid);

    int  updateFdSet(fd_set& rfds, fd_set& wfds);
    bool processFdSet(fd_set& rfds, fd_set& wfds);
    void waitForChildren();
    bool handleMessage(QJsonObject& message);
    bool needTimeout() const;

private:
    int *m_argc_ptr;
    char ***m_argv_ptr;
    QMap<int, ChildProcess *> m_children;
    QByteArray m_sendbuf;
    QByteArray m_recvbuf;
};


ParentProcess::ParentProcess(int *argc, char ***argv)
  : m_argc_ptr(argc), m_argv_ptr(argv)
{
    // Set up a signal handler for child events
    makePipe(sig_child_pipe);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = sig_child_handler;
    action.sa_flags = SA_NOCLDSTOP;
    ::sigaction(SIGCHLD, &action, &old_sig_child_handler);
}

ParentProcess::~ParentProcess()
{
    // The normal destructor is only executed by a child process, so all
    // we do is close down the extra file descriptors
    foreach (ChildProcess *child, m_children)
        delete child;   // This closes the file descriptors included in the child processes

    ::sigaction(SIGCHLD, &old_sig_child_handler, 0);
    ::close(sig_child_pipe[0]);
    ::close(sig_child_pipe[1]);
}

int ParentProcess::updateFdSet(fd_set& rfds, fd_set& wfds)
{
    FD_SET(0, &rfds);  // Always read from stdin
    FD_SET(sig_child_pipe[0], &rfds);  // Watch for signals
    if (m_sendbuf.size() > 0)
        FD_SET(1, &wfds);

    int n = sig_child_pipe[0];  // We're pretty sure this is the largest so far
    foreach (ChildProcess *child, m_children)
        n = child->updateFdSet(n, rfds, wfds);
    return n;
}

ChildProcess *ParentProcess::childFromPid(pid_t pid)
{
    foreach (ChildProcess *child, m_children)
        if (child->pid() == pid)
            return child;
    return NULL;
}

void ParentProcess::waitForChildren()
{
    int status;
    while (1) {
        pid_t pid = ::waitpid(-1, &status, WNOHANG);
        if (pid == 0)
            return;
        if (pid < 0)
            qFatal("Error in wait %s", strerror(errno));
        bool crashed = !WIFEXITED(status);
        int exitCode = WEXITSTATUS(status);
        ChildProcess *child = childFromPid(pid);
        if (child) {
            m_children.take(child->id());
            if (crashed)
                child->sendError(m_sendbuf, QProcess::Crashed, QStringLiteral("Process crashed"));
            child->sendStateChanged(m_sendbuf, QProcess::NotRunning);
            child->sendFinished(m_sendbuf, exitCode,
                                (crashed ? QProcess::CrashExit : QProcess::NormalExit));
            delete child;
        }
    }
}

// Return 'true' if we're a new child process
bool ParentProcess::processFdSet(fd_set& rfds, fd_set& wfds)
{
    // Handle the children first because other messages may change the child list
    foreach (ChildProcess *child, m_children)
        child->processFdSet(m_sendbuf, rfds, wfds);

    if (FD_ISSET(sig_child_pipe[0], &rfds)) {  // A child process died
        char c;
        if (::read(sig_child_pipe[0], &c, 1) == 1)
            waitForChildren();
        else
            qDebug() << "############################ READ SIG PROBLEM";
    }
    if (FD_ISSET(0, &rfds)) {  // Data available on stdin
        readToBuffer(0, m_recvbuf);
        // Process messages here
        while (m_recvbuf.size() >= 12) {
            qint32 message_size = qFromLittleEndian(((qint32 *)m_recvbuf.data())[2]) + 8;
            if (m_recvbuf.size() < message_size)
                break;
            QByteArray msg = m_recvbuf.left(message_size);
            m_recvbuf = m_recvbuf.mid(message_size);
            QJsonObject object = QJsonDocument::fromBinaryData(msg).object();
            if (handleMessage(object))
                return true;
        }
    }
    if (m_sendbuf.size() && FD_ISSET(1, &wfds))   // Write to stdout
        writeFromBuffer(1, m_sendbuf);
    return false;
}

// Return 'true' if this is a child process
bool ParentProcess::handleMessage(QJsonObject& message)
{
    if (message.value(RemoteProtocol::remote()).toString() == RemoteProtocol::halt()) {
        // Force all children to stop
        foreach (ChildProcess *child, m_children)
            child->stop(0);
        exit(0);
    }
    else {
        QString command = message.value(RemoteProtocol::command()).toString();
        int id = message.value(RemoteProtocol::id()).toDouble();
        if (command == RemoteProtocol::stop()) {
            ChildProcess *child = m_children.value(id);
            if (child) {
                int timeout = message.value(RemoteProtocol::timeout()).toDouble();
                child->stop(timeout);
            }
        } else if (command == RemoteProtocol::set()) {
            ChildProcess *child = m_children.value(id);
            if (child) {
                QString key = message.value(RemoteProtocol::key()).toString();
                int value = message.value(RemoteProtocol::value()).toDouble();
                if (key == RemoteProtocol::priority())
                    child->setPriority(value);
                else if (key == RemoteProtocol::oomAdjustment())
                    child->setOomAdjustment(value);
            }
        } else if (command == RemoteProtocol::start()) {
            ProcessInfo info(message.value(RemoteProtocol::info()).toObject().toVariantMap());
            ChildProcess *child = new ChildProcess(id);
            if (child->doFork()) {
                delete child;
                fixProcessState(info, m_argc_ptr, m_argv_ptr);
                return true;
            }
            else {
                m_children.insert(id, child);
                child->sendStateChanged(m_sendbuf, QProcess::Starting);
                child->sendStateChanged(m_sendbuf, QProcess::Running);
                child->sendStarted(m_sendbuf);
            }
        } else if (command == RemoteProtocol::write()) {
            ChildProcess *child = m_children.value(id);
            if (child)
                child->write(QByteArray::fromBase64(message.value(RemoteProtocol::data()).toString().toLatin1()));
        }
    }
    return false;
}

/*!
  Return true if some child is in a "needs a timeout" phase
 */

bool ParentProcess::needTimeout() const
{
    foreach (ChildProcess *child, m_children)
        if (child->needTimeout())
            return true;
    return false;
}

/**************************************************************************/

void forklauncher(int *argc, char ***argv )
{
    ParentProcess parent(argc, argv);

    while (1) {
        fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        int n = parent.updateFdSet(rfds, wfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        struct timeval *tptr = (parent.needTimeout() ? &timeout : NULL);

        // Select on the inputs
        int retval = ::select(n+1, &rfds, &wfds, NULL, tptr);
        if (retval == -1 && errno == EINTR)
            continue;
        if (retval < 0)
            qFatal("select: %s", strerror(errno));

        if (parent.processFdSet(rfds, wfds))
            return;
    }
}

void displayFileDescriptors(int argc, char **argv)
{
    QList<QByteArray> arglist;
    for (int i = 0 ; i < argc ; i++)
        arglist << argv[i];

    QList<QByteArray> envlist;
    const char *entry;
    for (int count = 0 ; (entry = environ[count]) ; count++)
        envlist.append(entry);

    struct rlimit data;
    if (::getrlimit(RLIMIT_NOFILE, &data) < 0)
        qFatal("Unable to read rlimit");
    QList<int> fdlist;
    for (unsigned int i=0 ; i < data.rlim_cur ; i++) {
        if (::fcntl(i, F_GETFD, 0) != -1)
            fdlist << i;
    }

    qDebug() << "##### " << arglist;
    qDebug() << "##### " << envlist;
    qDebug() << "##### FD " << fdlist;
}

QT_END_NAMESPACE_PROCESSMANAGER

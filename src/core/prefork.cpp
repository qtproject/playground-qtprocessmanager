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

#include "prefork.h"

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>

#if defined(Q_OS_LINUX)
#include <sys/prctl.h>
#endif

#include <QtGlobal>
#include <QDebug>
#include <QFileInfo>


QT_BEGIN_NAMESPACE_PROCESSMANAGER

static struct sigaction old_child_handler;

static void prefork_child_handler(int sig, siginfo_t* info, void *)
{
    if (sig == SIGCHLD)
        Prefork::instance()->checkChildDied(info->si_pid);

    // Complicated way of calling the old child handler
    void (*oldAction)(int) = ((volatile struct sigaction *)&old_child_handler)->sa_handler;
    if (oldAction && oldAction != SIG_IGN)
        oldAction(sig);
}

static void makePipe(int fd[])
{
    if (::pipe(fd) == -1)
        qFatal("Unable to create pipe: %s", strerror(errno));
    if (::fcntl(fd[0], F_SETFL, O_NONBLOCK) == -1)  // Set non-block on read end
        qFatal("Unable to set nonblocking: %s", strerror(errno));
}

/**************************************************************************/

/*!
  \class Prefork

  \brief The Prefork class forks into a series of pipe-connected
         processes with dynamically loaded main functions

  Many modern programs have complicated initialization routines that
  preload tables of information into each process.  The Prefork class
  allows you to initialize data just once, fork into multiple
  processes, and use the dynamic linker to load and execute the
  \c{main()} function from each desired child process.  By not
  invoking the normal \c{fork(); exec();} method of launching a new
  process, we ensure that the original initialized data does not have
  to be reloaded for each child.

  Using the Prefork class is as simple as:

  \code
  #include "prefork.h"

  int main(int argc, char **argv)
  {
    // ... Initialize common data items here
    Prefork::instance()->execute(&argc, &argv); // This function never returns
  }
  \endcode

  The command line arguments for the various children are delineated
  by "--" arguments.  For example:

  \code
    myprefork -a -foo -- program1 -arg1 -arg2 -- program2 -arg3 -arg4 -- program5
  \endcode

  The \c{-a} and \c{-foo} arguments are processed by your own
  program.  The first, or "master" process will load
  \c{program1} as a dynamic library and execute the main function
  passing \c{-arg1} and \c{-arg2} as its arguments.  The second, or
  "child 1" process will load \c{program2} as a dynamic library and
  execute its main function passing the \c{-arg3} and \c{-arg4}
  arguments.  The third, or "child 2" process will load \c{program5}
  as a dynamic library and execute its main function with no
  arguments.

  After preforking, the master process needs a method of finding which
  file descriptors are used to talk with which child process.  It uses
  the \l{instance()} to retrieve the singleton
  Prefork object and the Prefork::size() and Prefork::at() functions
  to retrieve information about the child processes.

  The PreforkProcessBackendFactory class is a wrapper around the
  Prefork object to make it easy to write a program that uses
  preforking to launch child processes.
 */

/*!
  Return the singleton Prefork instance
 */

Prefork *Prefork::instance() {
    static Prefork *s_forkit;
    if (!s_forkit)
        s_forkit = new Prefork;
    return s_forkit;
}

/*!
  \internal
 */

Prefork::Prefork()
  : m_argc(0)
  , m_argv(NULL)
  , m_argv_size(0)
  , m_count(0)
  , m_children(NULL)
{
}

/*!
  \internal
 */

int Prefork::nextMarker(int index)
{
    while (index < m_argc && ::strcmp(m_argv[index], "--") != 0)
        ++index;
    return index;
}

typedef int (*main_func_t)(int argc, char *argv[]);

// ### TODO:  Remove the directory path from the program name

/*!
  Open the application as a dll and jump to main
  This function never returns.
 */

void Prefork::launch(int start, int end)
{
    // Move over the old arguments to the front of the m_argv array
    size_t bytes_to_skip = m_argv[start] - m_argv[0];
    size_t bytes_to_move = (m_argv[end-1] + ::strlen(m_argv[end-1])) - m_argv[start];

    memmove(m_argv[0], m_argv[start], bytes_to_move);
    memset(m_argv[0] + bytes_to_move, 0, m_argv_size - bytes_to_move);

    // Shift all of the pointers
    for (int i = start ; i < end ; i++)
        m_argv[i-start] = m_argv[i] - bytes_to_skip;

    void *lib = dlopen(m_argv[0], RTLD_LAZY);
    if (!lib)
        qFatal("Unable to open: %s", dlerror());
    void *symbol = dlsym(lib, "main");
    if (!symbol)
        qFatal("No main routine in %s", m_argv[0]);

    qDebug() << "Running" << m_argv[0] << "as process" << getpid();
    int result = ((main_func_t)symbol)(end-start, m_argv);
    ::exit(result);
}

/*!
  Starting at \a start, look for an end and create
  a child.  Return the index of the next starting point
 */

int Prefork::makeChild(int start)
{
    int end = nextMarker(start);
    if (start < end) {
        int fd1[2];  // Stdin of the child
        int fd2[2];  // Stdout of the child
        makePipe(fd1);
        makePipe(fd2);
        int pid = ::fork();
        if (pid < 0)
            qFatal("Failed to fork: %s", strerror(errno));
        if (pid == 0) {  // Child
            ::dup2(fd1[0], STDIN_FILENO);
            ::dup2(fd2[1], STDOUT_FILENO);
            ::close(fd1[0]);
            ::close(fd1[1]);
            ::close(fd2[0]);
            ::close(fd2[1]);
#if defined(Q_OS_LINUX)
            ::prctl(PR_SET_PDEATHSIG, SIGHUP);  // Ask to be killed when parent dies
#endif
            launch(start, end);  // This function never returns
        }
        else {
            // Parent
            m_children[m_count].stdin  = fd1[1]; // Stdin of the child (write to this)
            m_children[m_count].stdout = fd2[0]; // Stdout of the child (read from this)
            m_children[m_count].pid    = pid;
            m_count++;
            ::close(fd1[0]);
            ::close(fd2[1]);
        }
    }
    return end + 1;
}

/*!
  Fork into the appropriate child processes which will be
  pipe-connected.  You must pass \a argc_ptr and \a argv_ptr, which
  are pointers to the standard \c{argc} and \c{argv} arguments.  Each
  child process will have \c{argc} and \c{argv} correctly rewritten.
 */

void Prefork::execute(int *argc_ptr, char ***argv_ptr)
{
    m_argc = *argc_ptr;
    m_argv = *argv_ptr;
    m_argv_size = (m_argv[m_argc-1] + ::strlen(m_argv[m_argc-1])) - m_argv[0];

    int marker_count = 0;
    for (int i = 0 ; i < m_argc ; i++)
        if (!::strcmp(m_argv[i], "--") == 0)
            marker_count++;

    if (marker_count < 2)
        qFatal("Expected to see at least two '--' markers");
    // This is excessive paranoia - I worry about a QList access in a signal handler
    int count = marker_count - 1;
    m_children  = (PreforkChildData *) ::calloc(sizeof(PreforkChildData), count);
    if (!m_children)
        qFatal("Memory error");

    int start = nextMarker(0) + 1;
    int end   = nextMarker(start);
    if (start >= end)
        qFatal("no defined main application");
    int index = end + 1;
    if (index >= m_argc)
        qFatal("no defined child application");
    while ((index = makeChild(index)) < m_argc)
        ;

    // Set up a signal handler - we kill everyone if something goes wrong
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = prefork_child_handler;
    action.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    ::sigaction(SIGCHLD, &action, &old_child_handler);

    launch(start, end);  // This function never returns
}

/*!
  Check to see if \a pid is one of our child processes.  This function
  is called from the \c{SIGCHILD} signal handler.  When any of our
  children die, the entire set of processes should shut down.
 */

void Prefork::checkChildDied(pid_t pid)
{
    for (int i = 0 ; i < m_count ; i++) {
        if (m_children[i].pid == pid) {
            // ### TODO: Should we kill all of the children here?
            // ### TODO: Is qFatal the right way to do this?
            qFatal("Prefork child died");
        }
    }
}

/*!
  Return how many child processes exist.  There is one new child
  process for each \c{fork()} call in the main process.
 */

int Prefork::size() const
{
    return m_count;
}

/*!
  Return information about the child at index \a i.
 */

const PreforkChildData *Prefork::at(int i) const
{
    Q_ASSERT(i >= 0 && i < m_count);
    return &m_children[i];
}

/*!
  \class PreforkChildData
  \brief The PreforkChildData class provides information about a single preforked child
 */

/*!
  \variable PreforkChildData::stdin
  \brief The file descriptor of the child's stdin (write to this)
*/

/*!
  \variable PreforkChildData::stdout
  \brief The file descriptor of the child's stdout (read from this)
*/

/*!
  \variable PreforkChildData::pid
  \brief The child's process id
*/

QT_END_NAMESPACE_PROCESSMANAGER

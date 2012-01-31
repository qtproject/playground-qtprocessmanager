/*
 * Copyright (C) 2011 Nokia Corporation
 */

#include "declarativeprocess.h"

#include <QDateTime>

/***************************************************************************************/

/*!
    \class DeclarativeProcess
    \brief The DeclarativeProcess class is a generalized representation of a process.
*/

/*!
    \property DeclarativeProcess::identifier
    \brief the application identifier of the process.
*/

/*!
    \property DeclarativeProcess::program
    \brief the filename of the binary executable that is launched to start up the process.
*/

/*!
    \property DeclarativeProcess::arguments
    \brief the arguments that will be passed to the program upon process startup
*/

/*!
    \property DeclarativeProcess::environment
    \brief a map of the environment variables that will be used by the process
*/

/*!
    \property DeclarativeProcess::workingDirectory
    \brief the directory that will be switched to before launching the process
*/

/*!
    \property DeclarativeProcess::uid
    \brief the user id (uid) of the process.
*/

/*!
    \property DeclarativeProcess::gid
    \brief the group id (gid) of the process.
*/

/*!
    \property DeclarativeProcess::pid
    \brief the process id (PID) of the process.

    Returns 0 if the process has not been started or if this is a "fake" process.
*/

/*!
    \property DeclarativeProcess::startTime
    \brief the start time of the process, measured in milliseconds since the epoch (1st Jan 1970 00:00).

    Returns 0 if process has not been started.
*/

/*!
    \property DeclarativeProcess::priority
    \brief The Unix process priority (niceness).

    Returns the current process priority if the process is running.  Otherwise,
    it returns the DeclarativeProcess priority setting.  You can only set the priority once the
    process is running.
*/

#if defined(Q_OS_LINUX)
/*!
    \property DeclarativeProcess::oomAdjustment
    \brief The Unix process /proc/<pid>/oom_score_adj (likelihood of being killed)

    Returns the current OOM adjustment score if the process is running.  Otherwise,
    it returns the DeclarativeProcess OOM adjustment score setting.  You can only set the OOM adjustment
    score when the process is running.
*/
#endif

/*!
    \internal
    Constructs a DeclarativeProcess instance with DeclarativeProcess \a info and optional \a parent
*/
DeclarativeProcess::DeclarativeProcess(ProcessBackend *process, QObject *parent)
    : ProcessFrontend(process, parent)
{
}

/*!
  Destroy this process object.
*/

DeclarativeProcess::~DeclarativeProcess()
{
}

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

#include "standardprocessbackend.h"
#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class StandardProcessBackend
    \brief The StandardProcessBackend class wraps a single QProcess object
*/

/*!
    Construct a StandardProcessBackend with ProcessInfo \a info and optional \a parent
*/

StandardProcessBackend::StandardProcessBackend(const ProcessInfo& info, QObject *parent)
    : UnixProcessBackend(info, parent)
{
}

/*!
    \brief Starts the process.

    After the process is started, the started() signal is emitted or
    an error(QProcess::FailedToStart) will be emitted.
*/

void StandardProcessBackend::start()
{
    if (createProcess())
        startProcess();
}

/*!
    Calls the parent class function and emits the started() signal.
*/

void StandardProcessBackend::handleProcessStarted()
{
    UnixProcessBackend::handleProcessStarted();
    emit started();
}

/*!
    Calls the parent class function and emits the error() signal with \a err.
*/

void StandardProcessBackend::handleProcessError(QProcess::ProcessError err)
{
    UnixProcessBackend::handleProcessError(err);
    emit error(err);
}

/*!
    Calls the parent class function and emits the finished() signal with
    the \a exitCode and \a exitStatus.
*/

void StandardProcessBackend::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    UnixProcessBackend::handleProcessFinished(exitCode, exitStatus);
    emit finished(exitCode, exitStatus);
}

/*!
    Calls the parent class function and emits the stateChanged() signal with \a state.
*/

void StandardProcessBackend::handleProcessStateChanged(QProcess::ProcessState state)
{
    UnixProcessBackend::handleProcessStateChanged(state);
    emit stateChanged(state);
}

#include "moc_standardprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qstandardprocessbackend.h"
#include <QDebug>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \class QStandardProcessBackend
    \brief The QStandardProcessBackend class wraps a single QProcess object
    \inmodule QtProcessManager
*/

/*!
    Construct a QStandardProcessBackend with QProcessInfo \a info and optional \a parent
*/

QStandardProcessBackend::QStandardProcessBackend(const QProcessInfo &info, QObject *parent)
    : QUnixProcessBackend(info, parent)
{
}

/*!
    \brief Starts the process.

    After the process is started, the started() signal is emitted or
    an error(QProcess::FailedToStart) will be emitted.
*/

void QStandardProcessBackend::start()
{
    if (createProcess())
        startProcess();
}

/*!
    Calls the parent class function and emits the started() signal.
*/

void QStandardProcessBackend::handleProcessStarted()
{
    QUnixProcessBackend::handleProcessStarted();
    emit started();
}

/*!
    Calls the parent class function and emits the error() signal with \a err.
*/

void QStandardProcessBackend::handleProcessError(QProcess::ProcessError err)
{
    QUnixProcessBackend::handleProcessError(err);
    emit error(err);
}

/*!
    Calls the parent class function and emits the finished() signal with
    the \a exitCode and \a exitStatus.
*/

void QStandardProcessBackend::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QUnixProcessBackend::handleProcessFinished(exitCode, exitStatus);
    emit finished(exitCode, exitStatus);
}

/*!
    Calls the parent class function and emits the stateChanged() signal with \a state.
*/

void QStandardProcessBackend::handleProcessStateChanged(QProcess::ProcessState state)
{
    QUnixProcessBackend::handleProcessStateChanged(state);
    emit stateChanged(state);
}

#include "moc_qstandardprocessbackend.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

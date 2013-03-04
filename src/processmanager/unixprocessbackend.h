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

#ifndef UNIX_PROCESS_BACKEND_H
#define UNIX_PROCESS_BACKEND_H

#include "processbackend.h"
#include <QTimer>

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT UnixProcessBackend : public ProcessBackend
{
    Q_OBJECT

public:
    UnixProcessBackend(const ProcessInfo& info, QObject *parent=0);
    virtual ~UnixProcessBackend();

    virtual Q_PID  pid() const;

    virtual qint32 actualPriority() const;
    virtual void   setDesiredPriority(qint32);

#if defined(Q_OS_LINUX)
    virtual qint32 actualOomAdjustment() const;
    virtual void   setDesiredOomAdjustment(qint32);
#endif

    virtual QProcess::ProcessState state() const;
    virtual void stop(int timeout = 500);

    virtual qint64 write(const char *data, qint64 maxSize);

    virtual QString errorString() const;

protected:
    bool createProcess();
    void startProcess();

    virtual void handleProcessStarted();
    virtual void handleProcessError(QProcess::ProcessError error);
    virtual void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    virtual void handleProcessStateChanged(QProcess::ProcessState state);

private slots:
    void unixProcessStarted();
    void unixProcessError(QProcess::ProcessError error);
    void unixProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void unixProcessStateChanged(QProcess::ProcessState state);

    void killTimeout();
    void readyReadStandardOutput();
    void readyReadStandardError();

protected:
    QProcess           *m_process;
    QTimer              m_killTimer;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // UNIX_PROCESS_BACKEND_H

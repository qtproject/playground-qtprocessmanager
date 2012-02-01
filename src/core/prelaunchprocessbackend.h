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

#ifndef PRELAUNCHPROCESSBACKEND_H
#define PRELAUNCHPROCESSBACKEND_H

#include "unixprocessbackend.h"
#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QueuedSignal
{
public:
    enum SignalName { StateChanged, Started, Error, Finished };

    SignalName name;
    union {
        QProcess::ProcessError error;
        struct {
            int                   exitCode;
            QProcess::ExitStatus  exitStatus;
        } f;
        QProcess::ProcessState state;
    } n;
};

class PrelaunchProcessBackend : public UnixProcessBackend
{
    Q_OBJECT

public:
    PrelaunchProcessBackend(const ProcessInfo& info, QObject *parent);
    virtual ~PrelaunchProcessBackend();

    void prestart();
    void setInfo(const ProcessInfo& info);
    bool isReady() const;

    virtual void start();
    virtual QProcess::ProcessState state() const;

private slots:
    void handleProcessStarted();
    void handleProcessError(QProcess::ProcessError error);
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessStateChanged(QProcess::ProcessState state);

private:
    QList<QueuedSignal> m_queue;
    bool                m_started;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PRELAUNCHPROCESSBACKEND_H

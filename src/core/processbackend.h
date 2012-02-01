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

#ifndef PROCESS_BACKEND_H
#define PROCESS_BACKEND_H

#include <QObject>
#include "processinfo.h"
#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessBackend : public QObject
{
    Q_OBJECT

protected:
    ProcessBackend(const ProcessInfo& info, QObject *parent);

public:
    virtual ~ProcessBackend();

    virtual QString     name() const;
    virtual QString     identifier() const;
    virtual QString     program() const;
    virtual QStringList arguments() const;
    virtual QVariantMap environment() const;
    virtual QString     workingDirectory() const;

    virtual QString     errorString() const;

    virtual qint64 uid() const;
    virtual qint64 gid() const;
    virtual Q_PID  pid() const;

    virtual qint32 actualPriority() const;
    virtual qint32 desiredPriority() const;
    virtual void   setDesiredPriority(qint32);

    virtual qint32 actualOomAdjustment() const;
    virtual qint32 desiredOomAdjustment() const;
    virtual void   setDesiredOomAdjustment(qint32);

    virtual QProcess::ProcessState state() const = 0;
    virtual void   start() = 0;
    virtual void   stop(int timeout = 500) = 0;
    virtual qint64 write(const char *data, qint64 maxSize);

    qint64 write(const char *data);
    qint64 write(const QByteArray& byteArray);

    enum EchoOutput { EchoNone, EchoStdoutOnly, EchoStderrOnly, EchoStdoutStderr };
    EchoOutput echo() const;
    void       setEcho(EchoOutput);

    ProcessInfo processInfo() const;

protected:
    virtual void handleStandardOutput(const QByteArray &output);
    virtual void handleStandardError(const QByteArray &output);

    void createName();

signals:
    void started();
    void error(QProcess::ProcessError error);
    void finished(int, QProcess::ExitStatus);
    void stateChanged(QProcess::ProcessState);
    void standardOutput(const QByteArray&);
    void standardError(const QByteArray&);

protected:
    QString     m_name;
    int         m_id;
    ProcessInfo m_info;
    EchoOutput  m_echo;
};

QT_END_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE_PROCESSMANAGER(ProcessBackend)*)

#endif // PROCESS_BACKEND_H

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

#ifndef PROCESS_BACKEND_H
#define PROCESS_BACKEND_H

#include <QObject>
#include "qprocessinfo.h"
#include "qprocessmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QProcessBackend : public QObject
{
    Q_OBJECT

protected:
    QProcessBackend(const QProcessInfo& info, QObject *parent);

public:
    virtual ~QProcessBackend();

    virtual QString     name() const;
    virtual QString     identifier() const;
    virtual QString     program() const;
    virtual QStringList arguments() const;
    virtual QVariantMap environment() const;
    virtual QString     workingDirectory() const;

    virtual QString     errorString() const;

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

    QProcessInfo processInfo() const;

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
    QProcessInfo m_info;
    EchoOutput  m_echo;
};

QT_END_NAMESPACE_PROCESSMANAGER

QT_PROCESSMANAGER_DECLARE_METATYPE_PTR(QProcessBackend)

#endif // PROCESS_BACKEND_H

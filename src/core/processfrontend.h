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


#ifndef PROCESS_FRONTEND_H
#define PROCESS_FRONTEND_H

#include <QObject>
#include "processinfo.h"

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessBackend;

class Q_ADDON_PROCESSMANAGER_EXPORT ProcessFrontend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString identifier READ identifier CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString program READ program CONSTANT)
    Q_PROPERTY(QStringList arguments READ arguments CONSTANT)
    Q_PROPERTY(QVariantMap environment READ environment CONSTANT)
    Q_PROPERTY(QString workingDirectory READ workingDirectory CONSTANT)

    Q_PROPERTY(qint64 pid READ pid NOTIFY started)
    Q_PROPERTY(qint64 startTime READ startTime NOTIFY started)

    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(int oomAdjustment READ oomAdjustment WRITE setOomAdjustment NOTIFY oomAdjustmentChanged)

    Q_PROPERTY(QString errorString READ errorString)

public:
    virtual ~ProcessFrontend();

    QString     identifier() const;
    QString     name() const;
    QString     program() const;
    QStringList arguments() const;
    QVariantMap environment() const;
    QString     workingDirectory() const;

    Q_PID  pid() const;

    qint32 priority() const;
    void   setPriority(qint32);

    qint32 oomAdjustment() const;
    void   setOomAdjustment(qint32);

    QProcess::ProcessState state() const;
    Q_INVOKABLE virtual void start();
    Q_INVOKABLE virtual void stop(int timeout = 500);

    qint64 write(const char *data, qint64 maxSize);
    qint64 write(const char *data);
    qint64 write(const QByteArray& byteArray);

    qint64 startTime() const;

    Q_INVOKABLE QVariantMap processInfo() const;

    QString errorString() const;

signals:
    void aboutToStart();
    void aboutToStop();

    void started();
    void error(QProcess::ProcessError);
    void finished(int, QProcess::ExitStatus);
    void stateChanged(QProcess::ProcessState);
    void standardOutput(const QByteArray&);
    void standardError(const QByteArray&);

    void priorityChanged();
    void oomAdjustmentChanged();

protected slots:
    void handleStarted();
    void handleError(QProcess::ProcessError);
    void handleFinished(int, QProcess::ExitStatus);
    void handleStateChanged(QProcess::ProcessState);

protected:
    ProcessFrontend(ProcessBackend *backend, QObject *parent=0);
    ProcessBackend *backend() const;

private slots:
    void handleStandardOutput(const QByteArray&);
    void handleStandardError(const QByteArray&);

protected:
    qint64          m_startTimeSinceEpoch;

private:
    ProcessBackend *m_backend;

    friend class ProcessManager;
};

QT_END_NAMESPACE_PROCESSMANAGER

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE_PROCESSMANAGER(ProcessFrontend)*)
Q_DECLARE_METATYPE(const QT_PREPEND_NAMESPACE_PROCESSMANAGER(ProcessFrontend)*)

#endif // PROCESS_FRONTEND_H

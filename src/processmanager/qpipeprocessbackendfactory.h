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

#ifndef PIPE_PROCESS_BACKEND_FACTORY_H
#define PIPE_PROCESS_BACKEND_FACTORY_H

#include "qremoteprocessbackendfactory.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QPipeProcessBackendFactory : public QRemoteProcessBackendFactory
{
    Q_OBJECT
    Q_PROPERTY(QProcessInfo* processInfo READ processInfo WRITE setProcessInfo NOTIFY processInfoChanged)

public:
    QPipeProcessBackendFactory(QObject *parent = 0);
    virtual ~QPipeProcessBackendFactory();

    virtual bool canCreate(const QProcessInfo &info) const;

    QProcessInfo *processInfo() const;
    void setProcessInfo(QProcessInfo *processInfo);
    void setProcessInfo(QProcessInfo& processInfo);

signals:
    void processInfoChanged();

protected:
    virtual QPidList localInternalProcesses() const;
    virtual bool send(const QJsonObject&);

private slots:
    void pipeReadyReadStandardOutput();
    void pipeReadyReadStandardError();
    void pipeStarted();
    void pipeError(QProcess::ProcessError error);
    void pipeFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void pipeStateChanged(QProcess::ProcessState state);

private:
    void stopRemoteProcess();

private:
    QProcess    *m_process;
    QProcessInfo *m_info;
    QByteArray   m_buffer;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PIPE_PROCESS_BACKEND_FACTORY_H

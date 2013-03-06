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

#ifndef REMOTE_PROCESS_BACKEND_H
#define REMOTE_PROCESS_BACKEND_H

#include <QJsonObject>

#include "qprocessmanager-global.h"
#include "qprocessbackend.h"
#include "qremoteprocessbackendfactory.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QRemoteProcessBackend : public QProcessBackend
{
    Q_OBJECT

public:
    QRemoteProcessBackend(const QProcessInfo& info, QRemoteProcessBackendFactory *factory,
                         int id, QObject *parent);
    virtual ~QRemoteProcessBackend();

    virtual Q_PID  pid() const;
    virtual qint32 actualPriority() const;
    virtual void   setDesiredPriority(qint32);
#if defined(Q_OS_LINUX)
    virtual qint32 actualOomAdjustment() const;
    virtual void   setDesiredOomAdjustment(qint32);
#endif

    virtual QProcess::ProcessState state() const;
    virtual void   start();
    virtual void   stop(int timeout = 500);
    virtual qint64 write(const char *data, qint64 maxSize);

    virtual QString errorString() const;

private:
    friend class QRemoteProcessBackendFactory;
    void killTimeout();
    void receive(const QJsonObject&);
    void factoryDestroyed();

private:
    QRemoteProcessBackendFactory *m_factory;
    QProcess::ProcessState       m_state;
    Q_PID                        m_pid;
    qint32                       m_id;
    QString                      m_errorString;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // REMOTE_PROCESS_BACKEND_H

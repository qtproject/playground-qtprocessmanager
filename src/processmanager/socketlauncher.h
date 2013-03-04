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

#ifndef SOCKET_LAUNCHER_H
#define SOCKET_LAUNCHER_H

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <qjsonserver.h>

#include "processbackendmanager.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class LauncherClient;

class Q_ADDON_PROCESSMANAGER_EXPORT SocketLauncher : public ProcessBackendManager {
    Q_OBJECT

public:
    SocketLauncher(QObject *parent=0);
    Q_INVOKABLE bool listen(int port, QtAddOn::QtJsonStream::QJsonAuthority *authority = 0);
    Q_INVOKABLE bool listen(const QString& socketname, QtAddOn::QtJsonStream::QJsonAuthority *authority=0);

    QtAddOn::QtJsonStream::QJsonServer * server() const;

protected:
    virtual void handleIdleCpuRequest();
    virtual void handleInternalProcessChange();

protected slots:
    virtual void handleInternalProcessError(QProcess::ProcessError);

private slots:
    void connectionAdded(const QString& identifier);
    void connectionRemoved(const QString& identifier);
    void messageReceived(const QString& identifier, const QJsonObject& message);
    void send(const QJsonObject& message);

private:
    void sendToClient(const QJsonObject& message, LauncherClient *client);

private:
    QtAddOn::QtJsonStream::QJsonServer *m_server;
    QMap<QString, LauncherClient*>      m_idToClient;
    QMap<LauncherClient*, QString>      m_clientToId;
};

QT_END_NAMESPACE_PROCESSMANAGER

QT_PROCESSMANAGER_DECLARE_METATYPE_PTR(SocketLauncher)

#endif // SOCKET_LAUNCHER_H

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

#ifndef SOCKET_LAUNCHER_H
#define SOCKET_LAUNCHER_H

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <qjsonserver.h>

#include "qprocessbackendmanager.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QLauncherClient;

class Q_ADDON_PROCESSMANAGER_EXPORT QSocketLauncher : public QProcessBackendManager {
    Q_OBJECT

public:
    QSocketLauncher(QObject *parent=0);
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
    void sendToClient(const QJsonObject& message, QLauncherClient *client);

private:
    QtAddOn::QtJsonStream::QJsonServer *m_server;
    QMap<QString, QLauncherClient*>      m_idToClient;
    QMap<QLauncherClient*, QString>      m_clientToId;
};

QT_END_NAMESPACE_PROCESSMANAGER

QT_PROCESSMANAGER_DECLARE_METATYPE_PTR(QSocketLauncher)

#endif // SOCKET_LAUNCHER_H

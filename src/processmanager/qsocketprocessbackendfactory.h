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

#ifndef SOCKET_PROCESS_BACKEND_FACTORY_H
#define SOCKET_PROCESS_BACKEND_FACTORY_H

#include "qremoteprocessbackendfactory.h"

class QLocalSocket;

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QSocketProcessBackendFactory : public QRemoteProcessBackendFactory
{
    Q_OBJECT
    Q_PROPERTY(QString socketName READ socketName WRITE setSocketName NOTIFY socketNameChanged)

public:
    QSocketProcessBackendFactory(QObject *parent = 0);
    virtual ~QSocketProcessBackendFactory();
    virtual bool canCreate(const QProcessInfo& info) const;

    QString socketName() const;
    void    setSocketName(const QString&);

signals:
    void    socketNameChanged();

protected:
    virtual bool send(const QJsonObject&);

private slots:
    void readyRead();
    void connected();
    void disconnected();

private:
    QLocalSocket *m_socket;
    QByteArray    m_buffer;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // SOCKET_PROCESS_BACKEND_FACTORY_H

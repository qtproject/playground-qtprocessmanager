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

#ifndef REMOTE_PROCESS_BACKEND_FACTORY_H
#define REMOTE_PROCESS_BACKEND_FACTORY_H

#include "processbackendfactory.h"
#include <QJsonObject>
#include <QMap>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class RemoteProcessBackend;

class Q_ADDON_PROCESSMANAGER_EXPORT RemoteProcessBackendFactory : public ProcessBackendFactory
{
    Q_OBJECT
public:
    RemoteProcessBackendFactory(QObject *parent = 0);
    virtual ~RemoteProcessBackendFactory();

    virtual ProcessBackend *create(const ProcessInfo& info, QObject *parent);

protected:
    virtual     bool send(const QJsonObject&) = 0;
    Q_INVOKABLE void receive(const QJsonObject&);

private:
    void backendDestroyed(int);
    friend class RemoteProcessBackend;

protected:
    int                              m_idCount;
    QMap<int, RemoteProcessBackend*> m_backendMap;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // REMOTE_PROCESS_BACKEND_FACTORY_H

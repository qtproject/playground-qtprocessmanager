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

#ifndef PREFORK_PROCESS_BACKEND_FACTORY_H
#define PREFORK_PROCESS_BACKEND_FACTORY_H

#include "qjsonpipe.h"
#include "qremoteprocessbackendfactory.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QPreforkProcessBackendFactory : public QRemoteProcessBackendFactory
{
    Q_OBJECT
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)

public:
    QPreforkProcessBackendFactory(QObject *parent = 0);
    virtual ~QPreforkProcessBackendFactory();

    int index() const;
    void setIndex(int index);

signals:
    void indexChanged();

protected:
    virtual QPidList localInternalProcesses() const;
    virtual bool send(const QJsonObject&);

private:
    int m_index;
    QtAddOn::QtJsonStream::QJsonPipe *m_pipe;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PREFORK_PROCESS_BACKEND_FACTORY_H

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

#ifndef DECLARATIVE_SOCKET_LAUNCHER_H
#define DECLARATIVE_SOCKET_LAUNCHER_H

#include <QtQml>
#include "qsocketlauncher.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QDeclarativeSocketLauncher : public QSocketLauncher,
                                                                public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QtAddOn::QtProcessManager::QProcessBackendFactory> factories READ factories)
    Q_PROPERTY(QQmlListProperty<QObject> children READ children)
    Q_CLASSINFO("DefaultProperty", "children")

public:
    QDeclarativeSocketLauncher(QObject *parent=0);
    QQmlListProperty<QProcessBackendFactory> factories();
    QQmlListProperty<QObject>               children();

    void classBegin();
    void componentComplete();

private:
    static void append_factory(QQmlListProperty<QProcessBackendFactory>*,
                               QProcessBackendFactory*);
    QList<QObject *> m_children;
};

QT_END_NAMESPACE_PROCESSMANAGER

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE_PROCESSMANAGER(QDeclarativeSocketLauncher))

#endif // DECLARATIVE_SOCKET_LAUNCHER_H

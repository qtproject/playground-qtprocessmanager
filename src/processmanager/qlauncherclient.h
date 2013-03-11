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

#ifndef _LAUNCHER_CLIENT_H
#define _LAUNCHER_CLIENT_H

#include <QObject>
#include <QJsonObject>
#include <QProcess>
#include <QMap>

#include "qprocessmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QProcessBackend;
class QProcessBackendManager;

class QLauncherClient : public QObject {
    Q_OBJECT
public:
    QLauncherClient(QProcessBackendManager *manager);
    void receive(const QJsonObject& message);

signals:
    void send(const QJsonObject& message);

private slots:
    void started();
    void finished(int, QProcess::ExitStatus);
    void error(QProcess::ProcessError);
    void stateChanged(QProcess::ProcessState);
    void standardOutput(const QByteArray&);
    void standardError(const QByteArray&);

private:
    QProcessBackendManager      *m_manager;
    QMap<int, QProcessBackend *> m_idToBackend;
    QMap<QProcessBackend *, int> m_backendToId;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // _LAUNCHER_CLIENT_H

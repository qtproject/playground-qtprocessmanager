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

#ifndef DECLARATIVE_PROCESS_MANAGER_H
#define DECLARATIVE_PROCESS_MANAGER_H

#include "processmanager.h"
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <qqml.h>

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT DeclarativeProcessManager : public ProcessManager,
                                                                public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<ProcessBackendFactory> factories READ factories)

public:
    static void registerTypes(const char *uri);

    DeclarativeProcessManager(QObject *parent=0);
    QQmlListProperty<ProcessBackendFactory> factories();

    void classBegin();
    void componentComplete();

signals:
    void processAboutToStart(const QString& name);
    void processAboutToStop(const QString& name);
    void processStarted(const QString& name);
    void processError(const QString& name, int error);
    void processFinished(const QString& name, int exitCode, int exitStatus);
    void processStateChanged(const QString& name, int state);
    void processDestroyed(const QString &name);

protected slots:
    virtual void processFrontendAboutToStart();
    virtual void processFrontendAboutToStop();
    virtual void processFrontendStarted();
    virtual void processFrontendError(QProcess::ProcessError);
    virtual void processFrontendFinished(int, QProcess::ExitStatus);
    virtual void processFrontendStateChanged(QProcess::ProcessState);
    virtual void processFrontendDestroyed();

private:
    static void append_factory(QQmlListProperty<ProcessBackendFactory>*,
                               ProcessBackendFactory*);
};

QT_END_NAMESPACE_PROCESSMANAGER

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE_PROCESSMANAGER(DeclarativeProcessManager))

#endif // DECLARATIVE_PROCESS_MANAGER_H

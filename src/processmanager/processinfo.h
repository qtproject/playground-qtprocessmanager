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

#ifndef PROCESSINFO_H
#define PROCESSINFO_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QProcessEnvironment>

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

namespace ProcessInfoConstants {
const QLatin1String Identifier = QLatin1String("identifier");
const QLatin1String Program = QLatin1String("program");
const QLatin1String Arguments = QLatin1String("arguments");
const QLatin1String Environment = QLatin1String("environment");
const QLatin1String WorkingDirectory = QLatin1String("workingDirectory");
const QLatin1String Uid = QLatin1String("uid");
const QLatin1String Gid = QLatin1String("gid");
const QLatin1String Umask = QLatin1String("umask");
const QLatin1String DropCapabilities = QLatin1String("dropCapabilities");
const QLatin1String Priority = QLatin1String("priority");
const QLatin1String OomAdjustment = QLatin1String("oomAdjustment");
const QLatin1String StartOutputPattern = QLatin1String("startOutputPattern");
}

class Q_ADDON_PROCESSMANAGER_EXPORT ProcessInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QString program READ program WRITE setProgram NOTIFY programChanged)
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)
    Q_PROPERTY(QVariantMap environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY workingDirectoryChanged)
    Q_PROPERTY(qint64 uid READ uid WRITE setUid NOTIFY uidChanged)
    Q_PROPERTY(qint64 gid READ gid WRITE setGid NOTIFY gidChanged)
    Q_PROPERTY(qint64 umask READ umask WRITE setUmask NOTIFY umaskChanged)
    Q_PROPERTY(qint64 dropCapabilities READ dropCapabilities WRITE setDropCapabilities NOTIFY dropCapabilitiesChanged)
    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(int oomAdjustment READ oomAdjustment WRITE setOomAdjustment NOTIFY oomAdjustmentChanged)
    Q_PROPERTY(QByteArray startOutputPattern READ startOutputPattern WRITE setStartOutputPattern NOTIFY startOutputPatternChanged)
public:
    explicit ProcessInfo(QObject *parent = 0);
    ProcessInfo(const ProcessInfo &other);
    ProcessInfo(const QVariantMap& map);
    ProcessInfo &operator =(const ProcessInfo &other);

    QString identifier() const;
    void setIdentifier(const QString &identifier);

    QString program() const;
    void setProgram(const QString &program);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);

    QVariantMap environment() const;
    void setEnvironment(const QProcessEnvironment &environment);
    Q_INVOKABLE void setEnvironment(const QVariantMap &environment);
    Q_INVOKABLE void insertEnvironment(const QVariantMap &environment);

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &workingDir);

    qint64 uid() const;
    void setUid(qint64 uid);

    qint64 gid() const;
    void setGid(qint64 gid);

    qint64 umask() const;
    void setUmask(qint64 umask);

    qint64 dropCapabilities() const;
    void setDropCapabilities(qint64 dropCapabilities);

    int priority() const;
    void setPriority(int priority);

    int oomAdjustment() const;
    void setOomAdjustment(int oomAdjustment);

    QByteArray startOutputPattern() const;
    void setStartOutputPattern(const QByteArray &outputPattern);

    Q_INVOKABLE QStringList keys() const;
    Q_INVOKABLE bool contains(const QString &key) const;
    Q_INVOKABLE QVariant value(const QString &key) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE void setData(const QVariantMap &data);
    Q_INVOKABLE void insert(const QVariantMap &data);
    QVariantMap toMap() const;

signals:
    void identifierChanged();
    void programChanged();
    void argumentsChanged();
    void environmentChanged();
    void workingDirectoryChanged();
    void uidChanged();
    void gidChanged();
    void umaskChanged();
    void dropCapabilitiesChanged();
    void priorityChanged();
    void oomAdjustmentChanged();
    void startOutputPatternChanged();

public slots:

protected:
    virtual void emitChangeSignal(const QString &key);

private:
    QVariantMap m_info;
};

QT_END_NAMESPACE_PROCESSMANAGER

QT_PROCESSMANAGER_DECLARE_METATYPE_PTR(ProcessInfo)

#endif // PROCESSINFO_H

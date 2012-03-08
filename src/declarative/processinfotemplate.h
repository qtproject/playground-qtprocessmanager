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

#ifndef PROCESSINFOTEMPLATE_H
#define PROCESSINFOTEMPLATE_H

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QtQml/QQmlScriptString>

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessInfo;

class Q_ADDON_PROCESSMANAGER_EXPORT ProcessInfoTemplate : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QQmlScriptString programName READ programName WRITE setProgramName NOTIFY programNameChanged)
    Q_PROPERTY(QQmlScriptString uid READ uid WRITE setUid NOTIFY uidChanged)
    Q_PROPERTY(QQmlScriptString gid READ gid WRITE setGid NOTIFY gidChanged)
    Q_PROPERTY(QQmlScriptString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY workingDirectoryChanged)
    Q_PROPERTY(QQmlScriptString environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    Q_PROPERTY(QQmlScriptString arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)
    Q_PROPERTY(QQmlScriptString startOutputPattern READ startOutputPattern WRITE setStartOutputPattern NOTIFY startOutputPatternChanged)
    Q_PROPERTY(QQmlScriptString priority READ priority WRITE setPriority NOTIFY priorityChanged)
    Q_PROPERTY(QQmlScriptString customValues READ customValues WRITE setCustomValues NOTIFY customValuesChanged)
    Q_PROPERTY(QQmlScriptString oomAdjustment READ oomAdjustment WRITE setOomAdjustment NOTIFY oomAdjustmentChanged)
public:
    ProcessInfoTemplate(QObject *parent = 0);

    QString identifier() const;
    QQmlScriptString programName() const;
    QQmlScriptString uid() const;
    QQmlScriptString gid() const;
    QQmlScriptString workingDirectory() const;
    QQmlScriptString environment() const;
    QQmlScriptString arguments() const;
    QQmlScriptString startOutputPattern() const;
    QQmlScriptString priority() const;
    QQmlScriptString customValues() const;
    QQmlScriptString oomAdjustment() const;

    Q_INVOKABLE virtual ProcessInfo *createProcessInfo(const QVariantMap &dict);

    Q_INVOKABLE QVariant bind(const QString &tag, const QVariant &defaultValue = QVariant());
    Q_INVOKABLE QString absoluteFilePath(const QString &url, const QString &filename = QString()) const;

    Q_INVOKABLE bool contains(const QString &tag);

public slots:
    void setIdentifier(const QString &identifier);
    void setProgramName(const QQmlScriptString &programName);
    void setUid(const QQmlScriptString &uid);
    void setGid(const QQmlScriptString &gid);
    void setWorkingDirectory(const QQmlScriptString &dir);
    void setEnvironment(QQmlScriptString env);
    void setArguments(const QQmlScriptString &args);
    void setStartOutputPattern(const QQmlScriptString &startOutputPattern);
    void setPriority(const QQmlScriptString &value);
    void setCustomValues(const QQmlScriptString &vals);
    void setOomAdjustment(const QQmlScriptString &vals);

signals:
    void identifierChanged();
    void programNameChanged();
    void uidChanged();
    void gidChanged();
    void workingDirectoryChanged();
    void environmentChanged();
    void argumentsChanged();
    void startOutputPatternChanged();
    void priorityChanged();
    void customValuesChanged();
    void oomAdjustmentChanged();

protected:
    virtual QVariantMap bindData(const QVariantMap &dict);

    QQmlScriptString scriptString(const QString &name) const;
    void setScriptString(const QString &name, const QQmlScriptString &script);

    QVariant bindScript(const QQmlScriptString &script);
    Q_INVOKABLE QVariant bindValue(const QString &name);
    Q_INVOKABLE void setDict(const QVariantMap &dict);

    static const QString kCustomValuesStr;

private:
    QVariantMap m_templateData;
    QVariantMap m_dict;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESSINFOTEMPLATE_H

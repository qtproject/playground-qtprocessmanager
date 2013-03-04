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

#ifndef DECLARATIVE_REWRITE_DELEGATE_H
#define DECLARATIVE_REWRITE_DELEGATE_H

#include <QtQml>
#include "rewritedelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT DeclarativeRewriteDelegate : public RewriteDelegate,
                                                                 public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlScriptString script READ script WRITE setScript NOTIFY scriptChanged)

public:
    DeclarativeRewriteDelegate(QObject *parent=0);
    virtual void rewrite(ProcessInfo& info);

    QQmlScriptString script() const;
    void                     setScript(const QQmlScriptString&);

    void classBegin();
    void componentComplete();

signals:
    void scriptChanged();

private:
    QQmlScriptString m_script;
    QQmlContext     *m_modelContext;
};

QT_END_NAMESPACE_PROCESSMANAGER

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE_PROCESSMANAGER(DeclarativeRewriteDelegate))

#endif // DECLARATIVE_REWRITE_DELEGATE_H

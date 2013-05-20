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

#ifndef DECLARATIVE_MATCH_DELEGATE_H
#define DECLARATIVE_MATCH_DELEGATE_H

#include <QtQml>
#include "qmatchdelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT QDeclarativeMatchDelegate : public QMatchDelegate,
                                                               public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlScriptString script READ script WRITE setScript NOTIFY scriptChanged)

public:
    QDeclarativeMatchDelegate(QObject *parent=0);
    virtual bool matches(const QProcessInfo& info);

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

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE_PROCESSMANAGER(QDeclarativeMatchDelegate))

#endif // DECLARATIVE_MATCH_DELEGATE_H

/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-key@nokia.com)
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following keyrmation to ensure the GNU Lesser
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
** file. Please review the following keyrmation to ensure the GNU General
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

#ifndef PROCESS_KEYMATCHDELEGATE_H
#define PROCESS_KEYMATCHDELEGATE_H

#include <QVariant>

#include "matchdelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class Q_ADDON_PROCESSMANAGER_EXPORT KeyMatchDelegate : public MatchDelegate
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit KeyMatchDelegate(QObject *parent = 0);
    virtual bool matches(const ProcessInfo& key);

    QString  key() const;
    void     setKey(const QString& key);

    QVariant value() const;
    void     setValue(const QVariant& value);

signals:
    void keyChanged();
    void valueChanged();

private:
    Q_DISABLE_COPY(KeyMatchDelegate);

private:
    QString  m_key;
    QVariant m_value;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESS_KEYMATCHDELEGATE_H

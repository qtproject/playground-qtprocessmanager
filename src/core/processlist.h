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

#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H

#include <QList>
#include <QProcess>
#include <QJsonArray>

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

typedef QList<Q_PID> PidList;

inline bool compareSortedLists(const PidList& a, const PidList& b)
{
    if (a.size() != b.size())
        return false;
    for (int i=0 ; i < a.size() ; i++)
        if (a.at(i) != b.at(i))
            return false;
    return true;
}

inline QJsonArray pidListToArray(const PidList& plist)
{
    QJsonArray array;
    foreach (Q_PID pid, plist)
        array.append((double) pid);
    return array;
}

inline PidList arrayToPidList(const QJsonArray& array)
{
    PidList plist;
    for (int i = 0 ; i < array.size() ; i++)
        plist.append((int) array.at(i).toDouble());
    return plist;
}

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PROCESS_LIST_H

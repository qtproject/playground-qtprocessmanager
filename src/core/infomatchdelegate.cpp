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

#include <QFileInfo>
#include <QDebug>
#include "infomatchdelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class InfoMatchDelegate
  \brief The InfoMatchDelegate class matches based on a ProcessInfo structure.

  The InfoMatchDelegate class matches based on a ProcessInfo record.  If an
  attribute is set in the ProcessInfo record, the value must match
  exactly.  The only exception are the environment variables, which
  only need to match exactly for environment values set in the ProcessInfo
  record
*/

/*!
  \property InfoMatchDelegate::info
  \brief The ProcessInfo object used by this delegate for matching.
 */

/*!
    Construct a InfoMatchDelegate with an optional \a parent.
*/

InfoMatchDelegate::InfoMatchDelegate(QObject *parent)
    : MatchDelegate(parent)
{
}

/*!
  Return the current ProcessInfo data.
*/

ProcessInfo InfoMatchDelegate::info() const
{
    return m_info;
}

/*!
  Set a new ProcessInfo \a info structure.
 */

void InfoMatchDelegate::setInfo(const ProcessInfo& info)
{
    m_info = info;
    emit infoChanged();
}

/*!
    \fn InfoMatchDelegate::matches(const ProcessInfo& info)

    We look at each configured field in the ProcessInfo \a info record.
    Return true if the ProcessInfo \a info record matches.

    The program name is validated by comparing canonicalFilePath values.
    The environment is compared key by key.
    All other record entries must match exactly.
*/

bool InfoMatchDelegate::matches(const ProcessInfo& info)
{
    QMapIterator<QString, QVariant> iter(m_info.toMap());
    while (iter.hasNext()) {
        iter.next();
        if (iter.key() == ProcessInfoConstants::Environment) {
            QMapIterator<QString, QVariant> iter2(iter.value().toMap());
            QVariantMap env = info.environment();
            while (iter2.hasNext()) {
                iter2.next();
                if (!env.contains(iter2.key()) || iter2.value() != env.value(iter2.key()))
                    return false;
            }
        }
        else {
            if (!info.contains(iter.key()) || iter.value() != info.value(iter.key()))
                return false;
        }
    }
    return true;
}

/*!
  \fn void InfoMatchDelegate::infoChanged()
  Signal emitted when the ProcessInfo object on this delegate has been changed.
*/

#include "moc_infomatchdelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

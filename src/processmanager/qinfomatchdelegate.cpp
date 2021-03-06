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

#include <QFileInfo>
#include <QDebug>
#include "qinfomatchdelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QInfoMatchDelegate
  \brief The QInfoMatchDelegate class matches based on a ProcessInfo structure.
  \inmodule QtProcessManager

  The QInfoMatchDelegate class matches based on a ProcessInfo record.  If an
  attribute is set in the ProcessInfo record, the value must match
  exactly.  The only exception are the environment variables, which
  only need to match exactly for environment values set in the ProcessInfo
  record
*/

/*!
  \property QInfoMatchDelegate::info
  \brief The ProcessInfo object used by this delegate for matching.
 */

/*!
    Construct a QInfoMatchDelegate with an optional \a parent.
*/

QInfoMatchDelegate::QInfoMatchDelegate(QObject *parent)
    : QMatchDelegate(parent)
{
}

/*!
  Return the current ProcessInfo data.
*/

QProcessInfo QInfoMatchDelegate::info() const
{
    return m_info;
}

/*!
  Set a new ProcessInfo \a info structure.
 */

void QInfoMatchDelegate::setInfo(const QProcessInfo& info)
{
    m_info = info;
    emit infoChanged();
}

/*!
    \fn QInfoMatchDelegate::matches(const QProcessInfo& info)

    We look at each configured field in the ProcessInfo \a info record.
    Return true if the ProcessInfo \a info record matches.

    The program name is validated by comparing canonicalFilePath values.
    The environment is compared key by key.
    All other record entries must match exactly.
*/

bool QInfoMatchDelegate::matches(const QProcessInfo& info)
{
    QMapIterator<QString, QVariant> iter(m_info.toMap());
    while (iter.hasNext()) {
        iter.next();
        if (iter.key() == QProcessInfoConstants::Environment) {
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
  \fn void QInfoMatchDelegate::infoChanged()
  Signal emitted when the ProcessInfo object on this delegate has been changed.
*/

#include "moc_qinfomatchdelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

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

#include "keymatchdelegate.h"
#include "processinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class KeyMatchDelegate
  \brief The KeyMatchDelegate class matches based on a key-value pair

  The KeyMatchDelegate class matches based on key-value pair.  It only
  matches ProcessInfo records which contain "key".  If a "value" attribute
  is also set, then the KeyMatchDelegate will only match a ProcessInfo
  record if the value of the "key" record matches the value.
*/

/*!
  \property KeyMatchDelegate::key
  \brief The key value required in the ProcessInfo object for a match.
 */

/*!
  \property KeyMatchDelegate::value
  \brief If set, this value must match the record in the ProcessInfo object.
 */

/*!
    Construct a KeyMatchDelegate with an optional \a parent.
*/

KeyMatchDelegate::KeyMatchDelegate(QObject *parent)
    : MatchDelegate(parent)
{
}

/*!
  Return the current key
*/

QString KeyMatchDelegate::key() const
{
    return m_key;
}

/*!
  Set a new \a key.
 */

void KeyMatchDelegate::setKey(const QString& key)
{
    m_key = key;
    emit keyChanged();
}

/*!
  Return the current value
*/

QVariant KeyMatchDelegate::value() const
{
    return m_value;
}

/*!
  Set a new \a value.
 */

void KeyMatchDelegate::setValue(const QVariant& value)
{
    m_value = value;
    emit valueChanged();
}

/*!
    \fn KeyMatchDelegate::matches(const ProcessInfo& info)
    \brief Check \a info for key and value attributes
*/

bool KeyMatchDelegate::matches(const ProcessInfo& info)
{
    return (info.contains(m_key) && (m_value.isNull() || info.value(m_key) == m_value));
}

/*!
  \fn void KeyMatchDelegate::keyChanged()
  Signal emitted when the key on this delegate has been changed.
*/

/*!
  \fn void KeyMatchDelegate::valueChanged()
  Signal emitted when the value on this delegate has been changed.
*/

#include "moc_keymatchdelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

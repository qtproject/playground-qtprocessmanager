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

#include "declarativematchdelegate.h"
#include "processinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass PmScriptMatch DeclarativeMatchDelegate
  \brief The PmScriptMatch class allows a factory to use a Javascript function for matching

  The PmScriptMatch object can be used in factory objects
  to specify match conditions.

  \qml
  PmManager {
      factories: [
          StandardProcessBackendFactory {
              matchDelegate: PmScriptMatch {
                  script: { return (model.program == "ls"); }
              }
          }
      ]
  }
  \endqml
*/

/*!
  \qmlproperty QQmlScriptString PmScriptMatch::script
  \brief Script to execute to see if there is a match

  This property holds the script to run.  The ProcessInfo object will be passed
  in as a global "model" variable.  The script should return "true" if the
  factory can create this object.
 */

/*!
  \class DeclarativeMatchDelegate
  \brief The DeclarativeMatchDelegate class allows a factory to use Javascript functions for matching.

  The DeclarativeMatchDelegate class can be used in factory objects to
  specify match conditions.
*/

/*!
  Construct a DeclarativeMatchDelegate with optional \a parent.
 */

DeclarativeMatchDelegate::DeclarativeMatchDelegate(QObject *parent)
    : MatchDelegate(parent)
    , m_modelContext(0)
{
}

/*!
  \internal
 */

void DeclarativeMatchDelegate::classBegin()
{
}

/*!
  \internal
 */

void DeclarativeMatchDelegate::componentComplete()
{
}

/*!
  Return true if the script object evaluates to \c{true} for this
  ProcessInfo object \a info.  The ProcessInfo object is internally
  bound to the \c{model} object property.
 */

bool DeclarativeMatchDelegate::matches(const ProcessInfo& info)
{
    if (m_script.isEmpty())
        return false;

    if (!m_modelContext)
        m_modelContext = new QQmlContext(QQmlEngine::contextForObject(this), this);

    m_modelContext->setContextProperty(QStringLiteral("model"), (QObject *) &info);

    QQmlExpression expr(m_script, m_modelContext);
    return expr.evaluate().toBool();
}

/*!
  Return the script object.
 */

QQmlScriptString DeclarativeMatchDelegate::script() const
{
    return m_script;
}

/*!
  Set the script object to \a script
 */

void DeclarativeMatchDelegate::setScript(const QQmlScriptString& script)
{
    m_script = script;
    emit scriptChanged();
}

/*!
  \fn void DeclarativeMatchDelegate::scriptChanged()
  This signal is emitted when the script object is changed.

  \sa setScript()
 */

#include "moc_declarativematchdelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

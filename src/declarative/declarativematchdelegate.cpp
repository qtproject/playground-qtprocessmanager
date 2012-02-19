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
  \qmlclass ScriptMatch DeclarativeMatchDelegate
  \brief The ScriptMatch class allows a factory to use a Javascript function for matching

  The ScriptMatch element can be used in factory
  objects to specify match conditions.

  \qml
  DeclarativeProcessManager {
      factories: [
          StandardProcessBackendFactory {
              matchDelegate: ScriptMatch {
                  script: { return (model.program == "ls"); }
              }
          }
      ]
  }
  \endqml
*/

/*!
  \qmlproperty script ScriptMatch::script

  This property holds the script to run.  The ProcessInfo object will be passed
  in as a global "model" variable.  The script should return "true" if the
  factory can create this object.
 */

DeclarativeMatchDelegate::DeclarativeMatchDelegate(QObject *parent)
    : MatchDelegate(parent)
    , m_modelContext(0)
{
}

void DeclarativeMatchDelegate::classBegin()
{
}

void DeclarativeMatchDelegate::componentComplete()
{
}

bool DeclarativeMatchDelegate::matches(const ProcessInfo& info)
{
    if (m_script.script().isEmpty())
        return false;

    if (!m_modelContext)
        m_modelContext = new QDeclarativeContext(m_script.context(), this);
    m_modelContext->setContextProperty(QStringLiteral("model"), (QObject *) &info);
    QDeclarativeExpression expr(m_modelContext, m_script.scopeObject(), m_script.script());
    return expr.evaluate().toBool();
}

QDeclarativeScriptString DeclarativeMatchDelegate::script() const
{
    return m_script;
}

void DeclarativeMatchDelegate::setScript(const QDeclarativeScriptString& script)
{
    m_script = script;
    emit scriptChanged();
}

#include "moc_declarativematchdelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

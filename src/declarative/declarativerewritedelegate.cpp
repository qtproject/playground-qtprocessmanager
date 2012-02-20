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

#include <QDebug>
#include "declarativerewritedelegate.h"
#include "processinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass ScriptRewrite DeclarativeRewriteDelegate
  \brief The ScriptRewrite class allows a factory to use a Javascript function for rewriting
  ProcessInfo objects.

  \qml
  DeclarativeProcessManager {
      factories: [
          StandardProcessBackendFactory {
              rewriteDelegate: ScriptRewrite {
                  script: {
                      var oldprog = model.program;
                      model.program = "gdb";
                      var args = model.arguments;
                      args.unshift(oldprog);
                      args.unshift("--");
                      model.arguments = args;
                  }
              }
          }
      ]
  }
  \endqml
*/

/*!
  \qmlproperty script ScriptRewrite::script

  This property holds the script to run.  The ProcessInfo object will be passed
  in as a global "model" variable.
 */

DeclarativeRewriteDelegate::DeclarativeRewriteDelegate(QObject *parent)
    : RewriteDelegate(parent)
    , m_modelContext(0)
{
}

void DeclarativeRewriteDelegate::classBegin()
{
}

void DeclarativeRewriteDelegate::componentComplete()
{
}

void DeclarativeRewriteDelegate::rewrite(ProcessInfo& info)
{
    if (!m_script.script().isEmpty()) {
        if (!m_modelContext)
            m_modelContext = new QDeclarativeContext(m_script.context(), this);
        m_modelContext->setContextProperty(QStringLiteral("model"), (QObject *) &info);
        QDeclarativeExpression expr(m_modelContext, m_script.scopeObject(), m_script.script());
        expr.evaluate();
    }
}

QDeclarativeScriptString DeclarativeRewriteDelegate::script() const
{
    return m_script;
}

void DeclarativeRewriteDelegate::setScript(const QDeclarativeScriptString& script)
{
    m_script = script;
    emit scriptChanged();
}

#include "moc_declarativerewritedelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

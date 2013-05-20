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

#include <QDebug>
#include "qdeclarativerewritedelegate.h"
#include "qprocessinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass PmScriptRewrite QDeclarativeRewriteDelegate
  \brief The PmScriptRewrite class allows a factory to use a Javascript function for rewriting
  ProcessInfo objects.

  \qml
  PmManager {
      factories: [
          StandardProcessBackendFactory {
              rewriteDelegate: PmScriptRewrite {
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
  \qmlproperty script PmScriptRewrite::script

  This property holds the script to run.  The ProcessInfo object will be passed
  in as a global "model" variable.
 */

/*!
  \class QDeclarativeRewriteDelegate
  \brief The QDeclarativeRewriteDelegate class is a lightweight wrapper around a RewriteDelegate.

  The QDeclarativeRewriteDelegate class rewrites ProcessInfo objects by
  passing them to a script object containing Javascript code.
 */

/*!
  Construct a QDeclarativeRewriteDelegate object with optional \a parent.
 */

QDeclarativeRewriteDelegate::QDeclarativeRewriteDelegate(QObject *parent)
    : QRewriteDelegate(parent)
    , m_modelContext(0)
{
}

/*!
  \internal
*/

void QDeclarativeRewriteDelegate::classBegin()
{
}

/*!
  \internal
*/

void QDeclarativeRewriteDelegate::componentComplete()
{
}

/*!
  Rewrite the \a info object by passing it to the stored Javascript.
  The \a info object is bound to the \c{model} property in the
  stored Javascript.
*/

void QDeclarativeRewriteDelegate::rewrite(QProcessInfo& info)
{
    if (!m_script.isEmpty()) {
        if (!m_modelContext)
            m_modelContext = new QQmlContext(QQmlEngine::contextForObject(this), this);
        m_modelContext->setContextProperty(QStringLiteral("model"), (QObject *) &info);

        QQmlExpression expr(m_script, m_modelContext);
        expr.evaluate();
    }
}

/*!
  Return a copy of the Javascript.
*/

QQmlScriptString QDeclarativeRewriteDelegate::script() const
{
    return m_script;
}

/*!
  Set the Javascript object to \a script.
*/

void QDeclarativeRewriteDelegate::setScript(const QQmlScriptString& script)
{
    m_script = script;
    emit scriptChanged();
}

/*!
  \fn void QDeclarativeRewriteDelegate::scriptChanged()
  This signal is emitted when the internal script object is changed.
*/

#include "moc_qdeclarativerewritedelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

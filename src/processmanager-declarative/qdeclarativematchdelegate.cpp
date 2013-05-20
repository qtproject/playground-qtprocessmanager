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

#include "qdeclarativematchdelegate.h"
#include "qprocessinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass PmScriptMatch QDeclarativeMatchDelegate
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
  \class QDeclarativeMatchDelegate
  \brief The QDeclarativeMatchDelegate class allows a factory to use Javascript functions for matching.

  The QDeclarativeMatchDelegate class can be used in factory objects to
  specify match conditions.
*/

/*!
  Construct a QDeclarativeMatchDelegate with optional \a parent.
 */

QDeclarativeMatchDelegate::QDeclarativeMatchDelegate(QObject *parent)
    : QMatchDelegate(parent)
    , m_modelContext(0)
{
}

/*!
  \internal
 */

void QDeclarativeMatchDelegate::classBegin()
{
}

/*!
  \internal
 */

void QDeclarativeMatchDelegate::componentComplete()
{
}

/*!
  Return true if the script object evaluates to \c{true} for this
  ProcessInfo object \a info.  The ProcessInfo object is internally
  bound to the \c{model} object property.
 */

bool QDeclarativeMatchDelegate::matches(const QProcessInfo& info)
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

QQmlScriptString QDeclarativeMatchDelegate::script() const
{
    return m_script;
}

/*!
  Set the script object to \a script
 */

void QDeclarativeMatchDelegate::setScript(const QQmlScriptString& script)
{
    m_script = script;
    emit scriptChanged();
}

/*!
  \fn void QDeclarativeMatchDelegate::scriptChanged()
  This signal is emitted when the script object is changed.

  \sa setScript()
 */

#include "moc_qdeclarativematchdelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

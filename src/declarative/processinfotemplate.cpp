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

#include "processinfotemplate.h"
#include "processinfo.h"

#include <QtQml/QQmlExpression>

#include <QFileInfo>
#include <QDir>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

const QString ProcessInfoTemplate::kCustomValuesStr = QString::fromLatin1("_customValues");

/*!
    \class ProcessInfoTemplate
    \brief The ProcessInfoTemplate class stores template information used to create ProcessInfo objects

    ProcessInfoTemplate is a declarative class used to store template information that will be used to
    create ProcessInfo objects.  These objects are created by calling \l createProcessInfo().

    Values stored in the template properties are not evaluated until \l createProcessInfo() is executed.
    Values can use \l bind() to bind to values that are contained in a dictionary that is specified when
     \l createProcessInfo() is executed.
*/

/*!
    \property ProcessInfoTemplate::identifier
    \brief unique identifier for this template

    Specifies a unique identifier for this template.
*/

/*!
    \property ProcessInfoTemplate::programName
    \brief unique programName for this template

    Specifies a unique programName for this template.
*/

/*!
    \property ProcessInfoTemplate::uid
    \brief unique uid for this template

    Specifies a unique uid for this template.
*/

/*!
    \property ProcessInfoTemplate::gid
    \brief unique gid for this template

    Specifies a unique gid for this template.
*/

/*!
    \property ProcessInfoTemplate::workingDirectory
    \brief unique workingDirectory for this template

    Specifies a unique workingDirectory for this template.
*/

/*!
    \property ProcessInfoTemplate::environment
    \brief unique environment for this template

    Specifies a unique environment for this template.
*/

/*!
    \property ProcessInfoTemplate::arguments
    \brief unique arguments for this template

    Specifies a unique arguments for this template.
*/

/*!
    \property ProcessInfoTemplate::startOutputPattern
    \brief unique startOutputPattern for this template

    Specifies a unique startOutputPattern for this template.
*/

/*!
    \property ProcessInfoTemplate::priority
    \brief unique priority for this template

    Specifies a unique priority for this template.
*/

/*!
    \property ProcessInfoTemplate::customValues
    \brief unique customValues for this template

    Specifies a unique customValues for this template.
*/

/*!
    \property ProcessInfoTemplate::oomAdjustment
    \brief oomAdjustment for this template

    Specifies a oomAdjustment for this template.
*/

/*!
   Constructs a ProcessInfoTemplate with optional \a parent.
 */

ProcessInfoTemplate::ProcessInfoTemplate(QObject *parent) :
    QObject(parent)
{
}

/*!
    Return the template identifier
*/

QString ProcessInfoTemplate::identifier() const
{
    return m_templateData.value(ProcessInfoConstants::Identifier).toString();
}

/*!
    Return the template programName
*/

QQmlScriptString ProcessInfoTemplate::programName() const
{
    return scriptString(ProcessInfoConstants::Program);
}

/*!
    Return the template uid
*/

QQmlScriptString ProcessInfoTemplate::uid() const
{
    return scriptString(ProcessInfoConstants::Uid);
}

/*!
    Return the template gid
*/

QQmlScriptString ProcessInfoTemplate::gid() const
{
    return scriptString(ProcessInfoConstants::Gid);
}

/*!
    Return the template workingDirectory
*/

QQmlScriptString ProcessInfoTemplate::workingDirectory() const
{
    return scriptString(ProcessInfoConstants::WorkingDirectory);
}

/*!
    Return the template environment
*/

QQmlScriptString ProcessInfoTemplate::environment() const
{
    return scriptString(ProcessInfoConstants::Environment);
}

/*!
    Return the template arguments
*/

QQmlScriptString ProcessInfoTemplate::arguments() const
{
    return scriptString(ProcessInfoConstants::Arguments);
}

/*!
    Return the template startOutputPattern
*/

QQmlScriptString ProcessInfoTemplate::startOutputPattern() const
{
    return scriptString(ProcessInfoConstants::StartOutputPattern);
}

/*!
    Return the template priority
*/

QQmlScriptString ProcessInfoTemplate::priority() const
{
    return scriptString(ProcessInfoConstants::Priority);
}

/*!
    Return the template customValues
*/

QQmlScriptString ProcessInfoTemplate::customValues() const
{
    return scriptString(kCustomValuesStr);
}

/*!
    Return the template oomAdjustment
*/

QQmlScriptString ProcessInfoTemplate::oomAdjustment() const
{
    return scriptString(ProcessInfoConstants::OomAdjustment);
}

/*!
    Set the template \a identifier
*/

void ProcessInfoTemplate::setIdentifier(const QString &identifier)
{
    m_templateData.insert(ProcessInfoConstants::Identifier, QVariant(identifier));
    emit identifierChanged();
}

/*!
    Set the template \a programName
*/

void ProcessInfoTemplate::setProgramName(const QQmlScriptString &programName)
{
    setScriptString(ProcessInfoConstants::Program, programName);
    emit programNameChanged();
}

/*!
    Set the template \a uid
*/

void ProcessInfoTemplate::setUid(const QQmlScriptString &uid)
{
    setScriptString(ProcessInfoConstants::Uid, uid);
    emit uidChanged();
}

/*!
    Set the template \a gid
*/

void ProcessInfoTemplate::setGid(const QQmlScriptString &gid)
{
    setScriptString(ProcessInfoConstants::Gid, gid);
    emit gidChanged();
}

/*!
    Set the template \a workingDirectory
*/

void ProcessInfoTemplate::setWorkingDirectory(const QQmlScriptString &workingDirectory)
{
    setScriptString(ProcessInfoConstants::WorkingDirectory, workingDirectory);
    emit workingDirectoryChanged();
}

/*!
    Set the template \a env environment
*/

void ProcessInfoTemplate::setEnvironment(QQmlScriptString environment)
{
    setScriptString(ProcessInfoConstants::Environment, environment);
    emit environmentChanged();
}

/*!
    Set the template \a arguments
*/

void ProcessInfoTemplate::setArguments(const QQmlScriptString &arguments)
{
    setScriptString(ProcessInfoConstants::Arguments, arguments);
    emit argumentsChanged();
}

/*!
    Set the template \a startOutputPattern
*/

void ProcessInfoTemplate::setStartOutputPattern(const QQmlScriptString &startOutputPattern)
{
    setScriptString(ProcessInfoConstants::StartOutputPattern, startOutputPattern);
    emit startOutputPatternChanged();
}

/*!
    Set the template \a priority
*/

void ProcessInfoTemplate::setPriority(const QQmlScriptString &priority)
{
    setScriptString(ProcessInfoConstants::Priority, priority);
    emit priorityChanged();
}

/*!
    Set the template \a customValues
*/

void ProcessInfoTemplate::setCustomValues(const QQmlScriptString &customValues)
{
    setScriptString(kCustomValuesStr, customValues);
    emit customValuesChanged();
}

/*!
    Set the template \a oomAdjustment
*/

void ProcessInfoTemplate::setOomAdjustment(const QQmlScriptString &oomAdjustment)
{
    setScriptString(ProcessInfoConstants::OomAdjustment, oomAdjustment);
    emit oomAdjustmentChanged();
}

/*!
 * Attempts to bind \a tag to a value in a previously specified dictionary.  Returns the
 * bound value, or \a defaultValue if a binding for \a tag does not exist in the dictionary.
 * This method is intended to be used in QML when specifying properties for the template.
 *
 * \sa createProcessInfo()
 */

QVariant ProcessInfoTemplate::bind(const QString &tag, const QVariant &defaultValue)
{
    return m_dict.value(tag, defaultValue);
}

/*!
    Creates and returns a new ProcessInfo object.  All property value scripts are evaluated
    (bound) with \a dict.
 */
ProcessInfo *ProcessInfoTemplate::createProcessInfo(const QVariantMap &dict)
{
    QVariantMap boundData = bindData(dict);
    ProcessInfo *processInfo = new ProcessInfo(boundData);
    return processInfo;
}

/*!
    Return the script string bound to \a name
 */

QQmlScriptString ProcessInfoTemplate::scriptString(const QString &name) const
{
    QVariant var = m_templateData.value(name);
    if (var.canConvert<QQmlScriptString>())
        return var.value<QQmlScriptString>();
    else
        return QQmlScriptString();
}

/*!
  Set the script string \a script to value \a name
 */

void ProcessInfoTemplate::setScriptString(const QString &name, const QQmlScriptString &script)
{
    m_templateData.insert(name, QVariant::fromValue<QQmlScriptString>(script));
}

/*!
  Returns the evaluated expression using \a script data.
 */

QVariant ProcessInfoTemplate::bindScript(const QQmlScriptString &script)
{
    if (script.context() && !script.script().isEmpty()) {
        // This is necessary because QQmlExpression tries to return any JS array script
        // as a QList<QObject *>, and that type is not useful to us (the returned JS array will
        // only contain null values.  So we wrap everything in a simple JSON object, and then pick
        // out the real value after evaluating it.

        QString scriptStr = QStringLiteral("script");
        QString jsonWrapper = QString::fromUtf8("{\"%1\": %2}").arg(scriptStr).arg(script.script());
        QQmlExpression expr(script.context(), this, jsonWrapper);

        return expr.evaluate().toMap().value(scriptStr);
    }
    else
        return QVariant();
}

/*!
  Returns the bindScript value evaluated from slot \a name
 */

QVariant ProcessInfoTemplate::bindValue(const QString &name)
{
    return bindScript(scriptString(name));
}

/*!
  Set the template dictionary to \a dict
*/

void ProcessInfoTemplate::setDict(const QVariantMap &dict)
{
    m_dict = dict;
}


/*!
    Returns the absolute file path that consists of \a url and \a filename.

    If \a filename is an absolute path by itself, \a url is ignored.
    If \a url is empty, only filename is used to resolve the path.
*/
QString ProcessInfoTemplate::absoluteFilePath(const QString &url, const QString &filename) const
{
    if (url.isEmpty())
        return QFileInfo(filename).absoluteFilePath();
    return QFileInfo(QDir(QUrl(url).toLocalFile()), filename).absoluteFilePath();
}

/*!
    Returns true if a value for \a tag hs been set in the dictionary, or false otherwise.
*/
bool ProcessInfoTemplate::contains(const QString &tag)
{
    return m_dict.contains(tag);
}

/*!
  Returns the boundData from binding \a dict.
 */

QVariantMap ProcessInfoTemplate::bindData(const QVariantMap &dict)
{
    setDict(dict);

    QVariantMap boundData;
    QMapIterator<QString, QVariant> it(m_templateData);
    while (it.hasNext()) {
        it.next();
        QVariant value = it.value();
        if (value.canConvert<QQmlScriptString>()) {
            boundData.insert(it.key(), bindScript(value.value<QQmlScriptString>()));
        } else {
            boundData.insert(it.key(), value.toString());
        }
    }
    return boundData;
}

/*!
    \fn void ProcessInfoTemplate::identifierChanged()
    Emitted when the template identifier changes
*/

/*!
    \fn void ProcessInfoTemplate::programNameChanged()
    Emitted when the template programName changes
*/

/*!
    \fn void ProcessInfoTemplate::uidChanged()
    Emitted when the template uid changes
*/

/*!
    \fn void ProcessInfoTemplate::gidChanged()
    Emitted when the template gid changes
*/

/*!
    \fn void ProcessInfoTemplate::workingDirectoryChanged()
    Emitted when the template workingDirectory changes
*/

/*!
    \fn void ProcessInfoTemplate::environmentChanged()
    Emitted when the template environment changes
*/

/*!
    \fn void ProcessInfoTemplate::argumentsChanged()
    Emitted when the template arguments change
*/

/*!
    \fn void ProcessInfoTemplate::startOutputPatternChanged()
    Emitted when the template startOutputPattern changes
*/

/*!
    \fn void ProcessInfoTemplate::priorityChanged()
    Emitted when the template priority changes
*/

/*!
    \fn void ProcessInfoTemplate::customValuesChanged()
    Emitted when the template customValues change
*/

/*!
    \fn void ProcessInfoTemplate::oomAdjustmentChanged()
    Emitted when the template oomAdjustment changes
*/

#include "moc_processinfotemplate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

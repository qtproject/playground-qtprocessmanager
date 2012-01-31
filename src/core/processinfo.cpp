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

#include "processinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \namespace ProcessInfoConstants
    \target ProcessInfoConstants Namespace

    \brief The ProcessInfoConstants namespace contains constant strings for ProcessInfo.
*/

/*!
    \class ProcessInfo
    \brief The ProcessInfo class is the set of information necessary to start a new process running.

    The ProcessInfo is internally implemented as a simple QVariantMap with a set of pre-defined
    keys.  This intentionally allows the ProcessInfo to be extended by the user without
    changing the API.  The predefined keys are:

    \list
      \o Identifier
      \o Program
      \o Arguments
      \o Environment
      \o WorkingDirectory
      \o UID
      \o GID
      \o Priority
      \o OomAdjustment
    \endlist
*/

/*!
    \property ProcessInfo::identifier
    \brief the application identifier of the process.  The identifier is used to construct
    a unique name.
*/

/*!
    \property ProcessInfo::program
    \brief the filename of the binary executable that is launched to start up the process.
*/

/*!
    \property ProcessInfo::arguments
    \brief the arguments that will be passed to the program upon process startup
*/

/*!
    \property ProcessInfo::environment
    \brief a map of the environment variables that will be used by the process
*/

/*!
    \property ProcessInfo::workingDirectory
    \brief the directory that will be switched to before launching the process
*/

/*!
    \property ProcessInfo::uid
    \brief the user id (uid) of the process.
*/

/*!
    \property ProcessInfo::gid
    \brief the group id (gid) of the process.
*/

/*!
    \property ProcessInfo::priority
    \brief the Unix priority "niceness" that the program will run at.
*/

/*!
    \property ProcessInfo::oomAdjustment
    \brief the Unix OOM adjustment that programs run at (+1000 = kill me first)
*/
/*!
    \property ProcessInfo::startOutputPattern
    \brief the start output pattern is QByteArray of a line to match.
*/


/*!
    Constructs a ProcessInfo instance with optional \a parent.
*/

ProcessInfo::ProcessInfo(QObject *parent)
    : QObject(parent)
{
}

/*!
     Copy constructor for ProcessInfo from \a other
*/

ProcessInfo::ProcessInfo(const ProcessInfo &other)
    : QObject(other.parent())
    , m_info(other.m_info)
{
}

/*!
     Copy constructor for ProcessInfo from \a map
*/

ProcessInfo::ProcessInfo(const QVariantMap& map)
    : m_info(map)
{
}

/*!
    Assignment constructor for ProcessInfo from \a other
*/

ProcessInfo &ProcessInfo::operator =(const ProcessInfo &other)
{
    m_info = other.m_info;
    return *this;
}

/*!
    Returns the application identifier of the process.  The identifier
    is used to construct a unique name for the process.
*/

QString ProcessInfo::identifier() const
{
    return m_info.value(ProcessInfoConstants::Identifier).toString();
}

/*!
     Set the application identifier of this process.
*/

void ProcessInfo::setIdentifier(const QString &identifier)
{
    setValue(ProcessInfoConstants::Identifier, identifier);
}

/*!
    Returns the filename of the binary executable that is launched.
*/

QString ProcessInfo::program() const
{
    return m_info.value(ProcessInfoConstants::Program).toString();
}

/*!
    Set the filename of the binary executable to be launched.
*/

void ProcessInfo::setProgram(const QString &program)
{
    m_info.insert(ProcessInfoConstants::Program, program);
}

/*!
    Return the argument list.
*/

QStringList ProcessInfo::arguments() const
{
    return m_info.value(ProcessInfoConstants::Arguments).toStringList();
}

/*!
    Set the argument list to be passed to the program
*/

void ProcessInfo::setArguments(const QStringList &argumentList)
{
    setValue(ProcessInfoConstants::Arguments, argumentList);
}

/*!
    Return the environment map
*/

QVariantMap ProcessInfo::environment() const
{
    return m_info.value(ProcessInfoConstants::Environment).toMap();
}

/*!
    Set the environment map
*/

void ProcessInfo::setEnvironment(const QVariantMap &environmentToSet)
{
    setValue(ProcessInfoConstants::Environment, environmentToSet);
}

/*!
    Set the environment map using a QProcessEnvironment
*/

void ProcessInfo::setEnvironment(const QProcessEnvironment &environment)
{
    QVariantMap env;
    QStringList keys = environment.keys();

    foreach (const QString &envKey, keys) {
        env.insert(envKey, environment.value(envKey));
    }

    setValue(ProcessInfoConstants::Environment, env);
}

/*!
    Return the process working directory
*/

QString ProcessInfo::workingDirectory() const
{
    return m_info.value(ProcessInfoConstants::WorkingDirectory).toString();
}

/*!
    Set the process working directory
*/

void ProcessInfo::setWorkingDirectory(const QString &workingDir)
{
    setValue(ProcessInfoConstants::WorkingDirectory, workingDir);
}

/*!
    Return the process UID

    Initializing this value to -1 will result in it being set to the process manager's uid.
*/

qint64 ProcessInfo::uid() const
{
    return m_info.value(ProcessInfoConstants::Uid).toLongLong();
}

/*!
    Set the process UID
*/

void ProcessInfo::setUid(qint64 newUid)
{
    setValue(ProcessInfoConstants::Uid, newUid);
}

/*!
    Return the process GID

    Initializing this value to -1 (or leaving it uninitialized) will result in it being
    set to the process manager's gid.
*/

qint64 ProcessInfo::gid() const
{
    return m_info.value(ProcessInfoConstants::Priority).toLongLong();
}

/*!
    Set the process GID
*/

void ProcessInfo::setGid(qint64 newGid)
{
    setValue(ProcessInfoConstants::Gid, newGid);
}

/*!
    Return the process priority
*/

int ProcessInfo::priority() const
{
    return m_info.value(ProcessInfoConstants::Priority).toDouble();
}

/*!
    Return the OOM adjustment value
*/

int ProcessInfo::oomAdjustment() const
{
    return m_info.value(ProcessInfoConstants::OomAdjustment).toDouble();
}

/*!
    Set the process priority.

    Normally the priority ranges from -20 to +19, but that
    can vary based on the type of Unix system.  A priority of -20
    delivers most of the CPU to this process; a priority of +19
    is the lowest and means that this process will be starved unless
    nothing else is going on in the sytem.

    We do not do any checking on the value set for process priority.
*/

void ProcessInfo::setPriority(int newPriority)
{
    if (newPriority != priority()) {
        setValue(ProcessInfoConstants::Priority, newPriority);
        emit priorityChanged();
    }
}

/*!
    Set the process OOM adjustment.

    In modern Linux kernels, the /proc/<pid>/oom_score_adj value
    may range from -1000 to +1000.  A -1000 value represents a process
    that will never be killed; a 0 is a process that follows normal
    OOM killer values, and a +1000 represents a process that will
    always be killed first.

    We do not do any checking on the value set for process priority.
*/

void ProcessInfo::setOomAdjustment(int newOomAdjustment)
{
    if (newOomAdjustment != oomAdjustment()) {
        setValue(ProcessInfoConstants::OomAdjustment, newOomAdjustment);
        emit oomAdjustmentChanged();
    }
}

/*!
    Returns the start output pattern.

    \sa setStartOutputPattern
*/
QByteArray ProcessInfo::startOutputPattern() const
{
    return m_info.value(ProcessInfoConstants::StartOutputPattern).toByteArray();
}

/*!
    Sets the start output pattern to \a outputPattern.

    The start output pattern is a string that the process should print when
    it considers itself ready. Typically, a process is ready after it has
    started up and performed its initialization successfully.
*/
void ProcessInfo::setStartOutputPattern(const QByteArray &outputPattern)
{
    setValue(ProcessInfoConstants::StartOutputPattern, outputPattern);
}

/*!
    Returns the keys for which values have been set in this ProcessInfo object.
*/
QStringList ProcessInfo::keys() const
{
    return m_info.keys();
}

/*!
    Return true if a value has been set for this \a key
*/

bool ProcessInfo::contains(const QString &key) const
{
    return m_info.contains(key);
}

/*!
    Return the value of this \a key
*/

QVariant ProcessInfo::value(const QString &key) const
{
    return m_info.value(key);
}

/*!
    Set \a key to \a value and emit appropriate change signals
*/

void ProcessInfo::setValue(const QString &key, const QVariant &value)
{
    if (key.isEmpty())
        return;

    QVariant oldValue = m_info.value(key);
    if (oldValue != value) {
        m_info.insert(key, value);
        emitChangeSignal(key);
    }
}

/*!
    Sets the data provided by this ProcessInfo object to \a data.

    Overwrites all existing values.
*/
void ProcessInfo::setData(const QVariantMap &data)
{
    m_info = data;
}

/*!
    Applies data provided by \a data on top of the existing data.

    Overwrites existing values if \a data contains the same keys as the
    original data. Leaves existing values with keys not contained by \a data
    untouched.
*/
void ProcessInfo::insert(const QVariantMap &data)
{
    QStringList keys = data.keys();
    foreach (const QString &key, keys) {
        m_info.insert(key, data.value(key));
    }
}

/*!
    Return the ProcessInfo object as a QVariantMap
*/
QVariantMap ProcessInfo::toMap() const
{
    return m_info;
}

/*!
  \internal
*/

void ProcessInfo::emitChangeSignal(const QString &key)
{
    if (key == ProcessInfoConstants::Identifier) {
        emit identifierChanged();
    } else if (key == ProcessInfoConstants::Program) {
        emit programChanged();
    } else if (key == ProcessInfoConstants::Arguments) {
        emit argumentsChanged();
    } else if (key == ProcessInfoConstants::Environment) {
        emit environmentChanged();
    } else if (key == ProcessInfoConstants::WorkingDirectory) {
        emit workingDirectoryChanged();
    } else if (key == ProcessInfoConstants::Uid) {
        emit uidChanged();
    } else if (key == ProcessInfoConstants::Gid) {
        emit gidChanged();
    } else if (key == ProcessInfoConstants::Priority) {
        emit priorityChanged();
    } else if (key == ProcessInfoConstants::OomAdjustment) {
        emit oomAdjustmentChanged();
    } else if (key == ProcessInfoConstants::StartOutputPattern) {
        emit startOutputPatternChanged();
    }
}

/*!
  Add \a newEnvironment to the current environment settings.
*/

void ProcessInfo::insertEnvironment(const QVariantMap &newEnvironment)
{
    QVariantMap env = environment();
    QMapIterator<QString, QVariant> it(newEnvironment);
    while (it.hasNext()) {
        it.next();
        env.insert(it.key(), it.value());
    }
    setEnvironment(env);
}

/*!
    \fn void ProcessInfo::identifierChanged()
    This signal is emitted when the process Identifier has been changed
*/
/*!
    \fn void ProcessInfo::programChanged()
    This signal is emitted when the process Program has been changed
*/
/*!
    \fn void ProcessInfo::argumentsChanged()
    This signal is emitted when the process Arguments has been changed
*/
/*!
    \fn void ProcessInfo::environmentChanged()
    This signal is emitted when the process Environment has been changed
*/
/*!
    \fn void ProcessInfo::workingDirectoryChanged()
    This signal is emitted when the process WorkingDirectory has been changed
*/
/*!
    \fn void ProcessInfo::uidChanged()
    This signal is emitted when the process UID has been changed
*/
/*!
    \fn void ProcessInfo::gidChanged()
    This signal is emitted when the process GID has been changed
*/
/*!
    \fn void ProcessInfo::priorityChanged()
    This signal is emitted when the process Priority has been changed
*/
/*!
    \fn void ProcessInfo::oomAdjustmentChanged()
    This signal is emitted when the process OOM adjustment has been changed
*/
/*!
    \fn void ProcessInfo::startOutputPatternChanged()
    This signal is emitted when the startOutputPattern has been changed.
*/

#include "moc_processinfo.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qprocessinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
    \namespace QProcessInfoConstants
    \target QProcessInfoConstants Namespace

    \brief The QProcessInfoConstants namespace contains constant strings for ProcessInfo.
    \inmodule QtProcessManager
*/

/*!
    \class QProcessInfo
    \brief The QProcessInfo class is the set of information necessary to start a new process running.
    \inmodule QtProcessManager

    The QProcessInfo is internally implemented as a simple QVariantMap with a set of pre-defined
    keys.  This intentionally allows the QProcessInfo to be extended by the user without
    changing the API.  The predefined keys are:

    \list
      \li Identifier
      \li Program
      \li Arguments
      \li Environment
      \li WorkingDirectory
      \li UID
      \li GID
      \li Priority
      \li OomAdjustment
    \endlist
*/

/*!
    \property QProcessInfo::identifier
    \brief the application identifier of the process.  The identifier is used to construct
    a unique name.
*/

/*!
    \property QProcessInfo::program
    \brief the filename of the binary executable that is launched to start up the process.
*/

/*!
    \property QProcessInfo::arguments
    \brief the arguments that will be passed to the program upon process startup
*/

/*!
    \property QProcessInfo::environment
    \brief a map of the environment variables that will be used by the process
*/

/*!
    \property QProcessInfo::workingDirectory
    \brief the directory that will be switched to before launching the process
*/

/*!
    \property QProcessInfo::uid
    \brief the user id (uid) of the process.
*/

/*!
    \property QProcessInfo::gid
    \brief the group id (gid) of the process.
*/

/*!
    \property QProcessInfo::umask
    \brief the default umask of the process.
*/

/*!
    \property QProcessInfo::priority
    \brief the Unix priority "niceness" that the program will run at.
*/

/*!
    \property QProcessInfo::oomAdjustment
    \brief the Unix OOM adjustment that programs run at (+1000 = kill me first)
*/
/*!
    \property QProcessInfo::startOutputPattern
    \brief the start output pattern is QByteArray of a line to match.
*/

/*!
    \property QProcessInfo::dropCapabilities
    \brief the capabilities that the process will drop after startup.
*/

/*!
    Constructs a QProcessInfo instance with optional \a parent.
*/

QProcessInfo::QProcessInfo(QObject *parent)
    : QObject(parent)
{
}

/*!
     Copy constructor for QProcessInfo from \a other
*/

QProcessInfo::QProcessInfo(const QProcessInfo &other)
    : QObject(other.parent())
    , m_info(other.m_info)
{
}

/*!
     Copy constructor for QProcessInfo from \a map
*/

QProcessInfo::QProcessInfo(const QVariantMap& map)
    : m_info(map)
{
}

/*!
    Assignment constructor for QProcessInfo from \a other
*/

QProcessInfo &QProcessInfo::operator =(const QProcessInfo &other)
{
    m_info = other.m_info;
    return *this;
}

/*!
    Returns the application identifier of the process.  The identifier
    is used to construct a unique name for the process.
*/

QString QProcessInfo::identifier() const
{
    return m_info.value(QProcessInfoConstants::Identifier).toString();
}

/*!
     Set the application identifier of this process.
*/

void QProcessInfo::setIdentifier(const QString &identifier)
{
    setValue(QProcessInfoConstants::Identifier, identifier);
}

/*!
    Returns the filename of the binary executable that is launched.
*/

QString QProcessInfo::program() const
{
    return m_info.value(QProcessInfoConstants::Program).toString();
}

/*!
    Set the filename of the binary executable to be launched.
*/

void QProcessInfo::setProgram(const QString &program)
{
    m_info.insert(QProcessInfoConstants::Program, program);
}

/*!
    Return the argument list.
*/

QStringList QProcessInfo::arguments() const
{
    return m_info.value(QProcessInfoConstants::Arguments).toStringList();
}

/*!
    Set the argument list to be passed to the program
*/

void QProcessInfo::setArguments(const QStringList &argumentList)
{
    setValue(QProcessInfoConstants::Arguments, argumentList);
}

/*!
    Return the environment map
*/

QVariantMap QProcessInfo::environment() const
{
    return m_info.value(QProcessInfoConstants::Environment).toMap();
}

/*!
    Set the environment map
*/

void QProcessInfo::setEnvironment(const QVariantMap &environmentToSet)
{
    setValue(QProcessInfoConstants::Environment, environmentToSet);
}

/*!
    Set the environment map using a QProcessEnvironment
*/

void QProcessInfo::setEnvironment(const QProcessEnvironment &environment)
{
    QVariantMap env;
    QStringList keys = environment.keys();

    foreach (const QString &envKey, keys) {
        env.insert(envKey, environment.value(envKey));
    }

    setValue(QProcessInfoConstants::Environment, env);
}

/*!
    Return the process working directory
*/

QString QProcessInfo::workingDirectory() const
{
    return m_info.value(QProcessInfoConstants::WorkingDirectory).toString();
}

/*!
    Set the process working directory
*/

void QProcessInfo::setWorkingDirectory(const QString &workingDir)
{
    setValue(QProcessInfoConstants::WorkingDirectory, workingDir);
}

/*!
    Return the process UID

    Initializing this value to -1 will result in it being set to the process manager's uid.
*/

qint64 QProcessInfo::uid() const
{
    return m_info.value(QProcessInfoConstants::Uid).toLongLong();
}

/*!
    Set the process UID
*/

void QProcessInfo::setUid(qint64 newUid)
{
    setValue(QProcessInfoConstants::Uid, newUid);
}

/*!
    Return the process GID

    Initializing this value to -1 (or leaving it uninitialized) will result in it being
    set to the process manager's gid.
*/

qint64 QProcessInfo::gid() const
{
    return m_info.value(QProcessInfoConstants::Gid).toLongLong();
}

/*!
    Set the process GID
*/

void QProcessInfo::setGid(qint64 newGid)
{
    setValue(QProcessInfoConstants::Gid, newGid);
}

/*!
    Return the process default umask
*/

qint64 QProcessInfo::umask() const
{
    return m_info.value(QProcessInfoConstants::Umask).toLongLong();
}

/*!
    Set the process default umask

    Initializing this value to -1 (or leaving it uninitialized) will result in the
    process inheriting its parent's umask.
*/

void QProcessInfo::setUmask(qint64 newUmask)
{
    setValue(QProcessInfoConstants::Umask, newUmask);
}

/*!
    Return the capabilities that the process will drop
*/

qint64 QProcessInfo::dropCapabilities() const
{
    return m_info.value(QProcessInfoConstants::DropCapabilities).toLongLong();
}

/*!
    Set the capabilities that the process will drop

    Initializing this value to 0 (or leaving it uninitialized) will result in the
    process not dropping any capabilities.
*/

void QProcessInfo::setDropCapabilities(qint64 dropCapabilities)
{
    setValue(QProcessInfoConstants::DropCapabilities, dropCapabilities);
}

/*!
    Return the process priority
*/

int QProcessInfo::priority() const
{
    return m_info.value(QProcessInfoConstants::Priority).toDouble();
}

/*!
    Return the OOM adjustment value
*/

int QProcessInfo::oomAdjustment() const
{
    return m_info.value(QProcessInfoConstants::OomAdjustment).toDouble();
}

/*!
    Set the process priority.

    Normally the priority ranges from -20 to +19, but that
    can vary based on the type of Unix system.  A priority of -20
    delivers most of the CPU to this process; a priority of +19
    is the lowest and means that this process will be starved unless
    nothing else is going on in the system.

    We do not do any checking on the value set for process priority.
*/

void QProcessInfo::setPriority(int newPriority)
{
    if (newPriority != priority()) {
        setValue(QProcessInfoConstants::Priority, newPriority);
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

void QProcessInfo::setOomAdjustment(int newOomAdjustment)
{
    if (newOomAdjustment != oomAdjustment()) {
        setValue(QProcessInfoConstants::OomAdjustment, newOomAdjustment);
        emit oomAdjustmentChanged();
    }
}

/*!
    Returns the start output pattern.

    \sa setStartOutputPattern
*/
QByteArray QProcessInfo::startOutputPattern() const
{
    return m_info.value(QProcessInfoConstants::StartOutputPattern).toByteArray();
}

/*!
    Sets the start output pattern to \a outputPattern.

    The start output pattern is a string that the process should print when
    it considers itself ready. Typically, a process is ready after it has
    started up and performed its initialization successfully.
*/
void QProcessInfo::setStartOutputPattern(const QByteArray &outputPattern)
{
    setValue(QProcessInfoConstants::StartOutputPattern, outputPattern);
}

/*!
    Returns the keys for which values have been set in this QProcessInfo object.
*/
QStringList QProcessInfo::keys() const
{
    return m_info.keys();
}

/*!
    Return true if a value has been set for this \a key
*/

bool QProcessInfo::contains(const QString &key) const
{
    return m_info.contains(key);
}

/*!
    Return the value of this \a key
*/

QVariant QProcessInfo::value(const QString &key) const
{
    return m_info.value(key);
}

/*!
    Set \a key to \a value and emit appropriate change signals
*/

void QProcessInfo::setValue(const QString &key, const QVariant &value)
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
    Sets the data provided by this QProcessInfo object to \a data.

    Overwrites all existing values.
*/
void QProcessInfo::setData(const QVariantMap &data)
{
    m_info = data;
}

/*!
    Applies data provided by \a data on top of the existing data.

    Overwrites existing values if \a data contains the same keys as the
    original data. Leaves existing values with keys not contained by \a data
    untouched.
*/
void QProcessInfo::insert(const QVariantMap &data)
{
    QStringList keys = data.keys();
    foreach (const QString &key, keys) {
        m_info.insert(key, data.value(key));
    }
}

/*!
    Return the QProcessInfo object as a QVariantMap
*/
QVariantMap QProcessInfo::toMap() const
{
    return m_info;
}

/*!
  \internal
*/

void QProcessInfo::emitChangeSignal(const QString &key)
{
    if (key == QProcessInfoConstants::Identifier) {
        emit identifierChanged();
    } else if (key == QProcessInfoConstants::Program) {
        emit programChanged();
    } else if (key == QProcessInfoConstants::Arguments) {
        emit argumentsChanged();
    } else if (key == QProcessInfoConstants::Environment) {
        emit environmentChanged();
    } else if (key == QProcessInfoConstants::WorkingDirectory) {
        emit workingDirectoryChanged();
    } else if (key == QProcessInfoConstants::Uid) {
        emit uidChanged();
    } else if (key == QProcessInfoConstants::Gid) {
        emit gidChanged();
    } else if (key == QProcessInfoConstants::Umask) {
        emit umaskChanged();
    } else if (key == QProcessInfoConstants::DropCapabilities) {
        emit dropCapabilitiesChanged();
    } else if (key == QProcessInfoConstants::Priority) {
        emit priorityChanged();
    } else if (key == QProcessInfoConstants::OomAdjustment) {
        emit oomAdjustmentChanged();
    } else if (key == QProcessInfoConstants::StartOutputPattern) {
        emit startOutputPatternChanged();
    }
}

/*!
  Add \a newEnvironment to the current environment settings.
*/

void QProcessInfo::insertEnvironment(const QVariantMap &newEnvironment)
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
    \fn void QProcessInfo::identifierChanged()
    This signal is emitted when the process Identifier has been changed
*/
/*!
    \fn void QProcessInfo::programChanged()
    This signal is emitted when the process Program has been changed
*/
/*!
    \fn void QProcessInfo::argumentsChanged()
    This signal is emitted when the process Arguments has been changed
*/
/*!
    \fn void QProcessInfo::environmentChanged()
    This signal is emitted when the process Environment has been changed
*/
/*!
    \fn void QProcessInfo::workingDirectoryChanged()
    This signal is emitted when the process WorkingDirectory has been changed
*/
/*!
    \fn void QProcessInfo::uidChanged()
    This signal is emitted when the process UID has been changed
*/
/*!
    \fn void QProcessInfo::gidChanged()
    This signal is emitted when the process GID has been changed
*/
/*!
    \fn void QProcessInfo::priorityChanged()
    This signal is emitted when the process Priority has been changed
*/
/*!
    \fn void QProcessInfo::oomAdjustmentChanged()
    This signal is emitted when the process OOM adjustment has been changed
*/
/*!
    \fn void QProcessInfo::startOutputPatternChanged()
    This signal is emitted when the startOutputPattern has been changed.
*/

#include "moc_qprocessinfo.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

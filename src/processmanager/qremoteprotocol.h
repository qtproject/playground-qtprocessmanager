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

#ifndef _REMOTE_PROTOCOL_H
#define _REMOTE_PROTOCOL_H

#include <QString>

#include "qprocessmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class QRemoteProtocol {
public:
    static inline const QString command() { return QStringLiteral("command"); }
    static inline const QString data() { return QStringLiteral("data"); }
    static inline const QString error() { return QStringLiteral("error"); }
    static inline const QString errorString() { return QStringLiteral("errorString"); }
    static inline const QString event() { return QStringLiteral("event"); }
    static inline const QString processError() { return QStringLiteral("processError"); }
    static inline const QString exitCode() { return QStringLiteral("exitCode"); }
    static inline const QString exitStatus() { return QStringLiteral("exitStatus"); }
    static inline const QString finished() { return QStringLiteral("finished"); }
    static inline const QString halt() { return QStringLiteral("halt"); }
    static inline const QString id() { return QStringLiteral("id"); }
    static inline const QString idlecpurequested() { return QStringLiteral("idlecpurequested"); }
    static inline const QString idlecpuavailable() { return QStringLiteral("idlecpuavailable"); }
    static inline const QString info() { return QStringLiteral("info"); }
    static inline const QString internalprocesses() { return QStringLiteral("internalprocesses"); }
    static inline const QString internalprocesserror() { return QStringLiteral("internalprocesserror"); }
    static inline const QString key() { return QStringLiteral("key"); }
    static inline const QString memory() { return QStringLiteral("memory"); }
    static inline const QString oomAdjustment() { return QStringLiteral("oomAdjustment"); }
    static inline const QString output() { return QStringLiteral("output"); }
    static inline const QString pid() { return QStringLiteral("pid"); }
    static inline const QString priority() { return QStringLiteral("priority"); }
    static inline const QString processes() { return QStringLiteral("processes"); }
    static inline const QString remote() { return QStringLiteral("remote"); }
    static inline const QString restricted() { return QStringLiteral("restricted"); }
    static inline const QString request() { return QStringLiteral("request"); }
    static inline const QString set() { return QStringLiteral("set"); }
    static inline const QString signal() { return QStringLiteral("signal"); }
    static inline const QString start() { return QStringLiteral("start"); }
    static inline const QString started() { return QStringLiteral("started"); }
    static inline const QString stateChanged() { return QStringLiteral("stateChanged"); }
    // Under Bionic, stderr & stdout are macros, so they can't be used as function names
    static inline const QString standarderror() { return QStringLiteral("stderr"); }
    static inline const QString standardout() { return QStringLiteral("stdout"); }
    static inline const QString stop() { return QStringLiteral("stop"); }
    static inline const QString timeout() { return QStringLiteral("timeout"); }
    static inline const QString value() { return QStringLiteral("value"); }
    static inline const QString write() { return QStringLiteral("write"); }
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // _REMOTE_PROTOCOL_H

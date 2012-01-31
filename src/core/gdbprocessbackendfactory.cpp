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

#include "gdbprocessbackendfactory.h"
#include "processinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class GdbProcessBackendFactory
  \brief The GdbProcessBackendFactory class creates UnixProcessBackend objects

  This is a simple example class showing how to create a custom factory.
  The GdbProcessBackendFactory matches ProcessInfo records with a "gdb"
  attribute of "true" (the string, not the boolean value).  It rewrites the
  process arguments to launch gdb with the passed application.

  In the future this class will be replaced with a general "rewriting"
  frontend to modify ProcessInfo arguments.
*/

/*!
  Construct a GdbProcessBackendFactory with optional \a parent
*/

GdbProcessBackendFactory::GdbProcessBackendFactory(QObject *parent)
    : StandardProcessBackendFactory(parent)
{
}

/*!
  GdbProcessBackendFactory will match ProcessInfo \a info objects
  with a "gdb" attribute containing the string "true".
*/

bool GdbProcessBackendFactory::canCreate(const ProcessInfo& info) const
{
    return (info.value("gdb").toString() == "true");
}

/*!
  Construct a UnixProcessBackend from a ProcessInfo \a info record and \a parent,
  but change the program name to "gdb" and pass the original
  program as an argument.
*/

ProcessBackend * GdbProcessBackendFactory::create(const ProcessInfo& info, QObject *parent)
{
    ProcessInfo i = info;
    QStringList args = i.arguments();
    args.prepend(i.program());
    args.prepend("--");
    i.setProgram("gdb");
    return StandardProcessBackendFactory::create(i, parent);
}

#include "moc_gdbprocessbackendfactory.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

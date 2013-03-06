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

#include <QStringList>
#include "qgdbrewritedelegate.h"
#include "qprocessinfo.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class QGdbRewriteDelegate
  \brief The QGdbRewriteDelegate class adds gdb to the process arguments.
  \inmodule QtProcessManager
*/

/*!
    Construct a QGdbRewriteDelegate with an optional \a parent.
*/

QGdbRewriteDelegate::QGdbRewriteDelegate(QObject *parent)
    : QRewriteDelegate(parent)
{
}

/*!
    \fn void QGdbRewriteDelegate::rewrite(QProcessInfo& info)

    Rewrites the QProcessInfo \a info structure to launch gdb.
    For example, "prog arg1 arg2 ..." becomes "gdb -- prog arg1 arg2..."
*/

void QGdbRewriteDelegate::rewrite(QProcessInfo& info)
{
    QStringList args = info.arguments();
    args.prepend(info.program());
    args.prepend(QStringLiteral("--"));
    info.setProgram(QStringLiteral("gdb"));
    info.setArguments(args);
}

#include "moc_qgdbrewritedelegate.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

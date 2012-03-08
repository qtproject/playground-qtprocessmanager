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

#include "declarativesocketlauncher.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass Launcher DeclarativeSocketLauncher
  \brief The Launcher class encapsulates ways of creating and tracking processes
         suitable for QtQml programs.

   The Launcher class is used to create a standalone program for launching
   and tracking child processes.  Other programs can connect to the Launcher using
   a JsonStream-formatted socket connection.

   Here is an example of a simple launcher instantiation:

   \qml
   import QtQuick 2.0
   import Test 1.0

   Launcher {
       id: socket_launcher

       factories: [
           StandardProcessBackendFactory {
               id: gdbFactory
               matchDelegate: KeyMatchDelegate { key: "gdb" }
               rewriteDelegate: GdbRewriteDelegate {}
           },
           StandardProcessBackendFactory {
               id: standardFactory
           }
       ]

       JsonUIDRangeAuthority {
           id: rangeAuthority
           minimum: 0
           maximum: 1000
       }

       Component.onCompleted: {
           socket_launcher.listen("/tmp/socket_launcher", rangeAuthority);
       }
   }
   \endqml

   Note that the Launcher object has been configured with two factories, the first
   of which rewrites any ProcessInfo objects containing a "gdb" attribute to
   use the gdb server.  The Launcher object also has a JsonUIDRangeAuthority object
   which is assigned to the internal JsonServer.
*/

/*!
  \qmlproperty list<ProcessBackendFactory> Launcher::factories
  List of ProcessBackendFactory objects.

  The order of the list is important - when launching a new
  process each factory in turn will be given a chance.
 */

/*!
  \qmlproperty list<Object> Launcher::children
  Generic child objects of this Launcher - allows extra objects like Authorities
  to be instantiated under this class.
 */

DeclarativeSocketLauncher::DeclarativeSocketLauncher(QObject *parent)
    : SocketLauncher(parent)
{
}

void DeclarativeSocketLauncher::classBegin()
{
}

void DeclarativeSocketLauncher::componentComplete()
{
}

void DeclarativeSocketLauncher::append_factory(QQmlListProperty<ProcessBackendFactory> *list,
                                               ProcessBackendFactory *factory)
{
    DeclarativeSocketLauncher *launcher = static_cast<DeclarativeSocketLauncher *>(list->object);
    if (factory && launcher)
        launcher->addFactory(factory);
}

QQmlListProperty<ProcessBackendFactory> DeclarativeSocketLauncher::factories()
{
    return QQmlListProperty<ProcessBackendFactory>(this, NULL, append_factory);
}

QQmlListProperty<QObject> DeclarativeSocketLauncher::children()
{
    return QQmlListProperty<QObject>(this, m_children);
}

#include "moc_declarativesocketlauncher.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

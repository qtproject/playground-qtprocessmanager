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

#include <QDebug>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlProperty>

#include <qjsonuidauthority.h>
#include <qjsonuidrangeauthority.h>

#include "qdeclarativesocketlauncher.h"
#include "qdeclarativeprocessmanager.h"
#include "qstandardprocessbackendfactory.h"
#include "qkeymatchdelegate.h"
#include "qgdbrewritedelegate.h"

QT_USE_NAMESPACE_PROCESSMANAGER

QString progname;

static void usage()
{
    qWarning("Usage: %s [ARGS] FILENAME\n"
             "\n"
             "The FILENAME should be a QML file to load\n"
             , qPrintable(progname));
    exit(1);
}

static void registerQmlTypes()
{
    const char *uri = "Test";
    QDeclarativeProcessManager::registerTypes(uri);
    qmlRegisterUncreatableType<QtAddOn::QtJsonStream::QJsonAuthority>(uri, 1, 0, "JsonAuthority", "Abstract class");
    qmlRegisterType<QtAddOn::QtJsonStream::QJsonUIDRangeAuthority>(uri, 1, 0, "JsonUIDRangeAuthority");
    qmlRegisterType<QtAddOn::QtJsonStream::QJsonUIDAuthority>(uri, 1, 0, "JsonUIDAuthority");
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QStringList args = QGuiApplication::arguments();
    progname = args.takeFirst();
    while (args.size()) {
        QString arg = args.at(0);
        if (!arg.startsWith('-'))
            break;
        args.removeFirst();
        if (arg == QLatin1String("-help"))
            usage();
        else {
            qWarning("Unexpected argument '%s'", qPrintable(arg));
            usage();
        }
    }

    if (args.size() != 1)
        usage();

    registerQmlTypes();

    QQmlEngine    engine;
    QQmlComponent component(&engine, QUrl::fromLocalFile(args.takeFirst()));
    if (component.isError())
        qWarning() << component.errors();
    component.create();
    return app.exec();
}

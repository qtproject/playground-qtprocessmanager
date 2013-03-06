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

#include <QCoreApplication>
#include <QDebug>
#include "qpipelauncher.h"
#include "qstandardprocessbackendfactory.h"
#include "qprelaunchprocessbackendfactory.h"
#include "qprocessinfo.h"

QT_USE_NAMESPACE_PROCESSMANAGER

QString progname;

void usage()
{
    qWarning("Usage: %s [ARGS] [OPTS]\n"
             "\n"
             "Accepts JSON-formatted commands over STDIN and sends results over STDOUT.\n"
             "\n"
             "Valid arguments:\n"
             "   -prelaunch PROGRAM    Create prelaunch launcher instead of standard\n"
             , qPrintable(progname));
    exit(1);
}


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    progname = args.takeFirst();
    QString prelaunch_program;

    while (args.size()) {
        QString arg = args.at(0);
        if (!arg.startsWith('-'))
            break;
        args.removeFirst();
        if (arg == QStringLiteral("-help"))
            usage();
        else if (arg == QStringLiteral("-prelaunch")) {
            if (!args.size())
                usage();
            prelaunch_program = args.takeFirst();
        }
        else {
            qWarning("Unexpected argument '%s'", qPrintable(arg));
            usage();
        }
    }

    if (args.size())
        usage();

    QPipeLauncher launcher;
    if (!prelaunch_program.isEmpty()) {
        QProcessInfo info;
        info.setValue("program", prelaunch_program);
        QPrelaunchProcessBackendFactory *factory = new QPrelaunchProcessBackendFactory;
        factory->setProcessInfo(info);
        launcher.addFactory(factory);
    }
    else
        launcher.addFactory(new QStandardProcessBackendFactory);
    return app.exec();
}

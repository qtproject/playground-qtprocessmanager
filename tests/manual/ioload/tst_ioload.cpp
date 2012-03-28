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
#include <QStringList>
#include <QDebug>

#include <ioidledelegate.h>
#include <iostream>

QT_USE_NAMESPACE_PROCESSMANAGER

QString progname;

class Target : public QObject {
    Q_OBJECT

public slots:
    void loadUpdate(double value) {
        std::cout << value << std::endl;
    }
    void idleCpuAvailable() {
        std::cout << "idle ";
    }
};

static void usage()
{
    qWarning("Usage: %s [ARGS] DEVICE\n", qPrintable(progname));
    exit(1);
}

int
main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    progname = args.takeFirst();
    while (args.size()) {
        QString arg = args.at(0);
        if (!arg.startsWith('-'))
            break;
        args.removeFirst();
        if (arg == QLatin1String("-help"))
            usage();
    }

    if (args.size() != 1)
        usage();

    IoIdleDelegate io;
    Target t;
    io.setDevice(args.at(0));
    QObject::connect(&io, SIGNAL(loadUpdate(double)), &t, SLOT(loadUpdate(double)));
    QObject::connect(&io, SIGNAL(idleCpuAvailable()), &t, SLOT(idleCpuAvailable()));

    io.requestIdleCpu(true);
    return app.exec();
}

#include "tst_ioload.moc"

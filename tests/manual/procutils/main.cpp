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
#include <QStringList>

#include <procutils.h>
//#include <stdio.h>
//#include <string.h>
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

QT_USE_NAMESPACE_PROCESSMANAGER

QString progname;

static void usage()
{
    qWarning("Usage: %s [ARGS] [PID+]\n"
             "\n"
             "Arguments are only useful if don't specify a PID value to read\n"
             "   -uid UID    Set user id\n"
             "   -gid GID    Set group id\n"
             "   -euid UID   Set effective user id\n"
             "   -egid GID   Set effective group id\n"
             "\n"
             "If you don't pass any PID values on the command line, this program's pid will be used\n"
             , qPrintable(progname));
    exit(1);
}

static void dumpInfo(pid_t pid)
{
    ExecutingProcessInfo p(pid);
    if (p.exists()) {
        std::cout << "PID: " << p.pid() << std::endl
                  << "PPID: " << p.ppid() << std::endl
                  << "PGRP: " << p.pgrp() << std::endl
                  << "SID: " << p.sid() << std::endl
                  << "UID: " << p.uid() << " (real) " << p.euid()
                  << " (effective) " << p.suid() << " (saved) " << std::endl
                  << "GID: " << p.gid() << " (real) " << p.egid()
                  << " (effective) " << p.sgid() << " (saved) " << std::endl
                  << "Priority: " << p.priority() << std::endl
                  << "Nice: " << p.nice() << std::endl
                  << "Groups:";
        foreach (gid_t group, p.groups())
            std::cout << " " << group;
        std::cout << std::endl
                  << "Name: " << qPrintable(p.name()) << std::endl;
    }
    else
        std::cout << "Doesn't exist" << pid << std::endl;
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
        else if (arg == QLatin1String("-uid")) {
            if (!args.size())
                usage();
            if (::setuid(args.takeFirst().toInt()) < 0)
                qWarning("Unable to setuid: %s", strerror(errno));
        }
        else if (arg == QLatin1String("-gid")) {
            if (!args.size())
                usage();
            if (::setgid(args.takeFirst().toInt()) < 0)
                qWarning("Unable to setgid: %s", strerror(errno));
        }
        else if (arg == QLatin1String("-euid")) {
            if (!args.size())
                usage();
            if (::seteuid(args.takeFirst().toInt()) < 0)
                qWarning("Unable to seteuid: %s", strerror(errno));
        }
        else if (arg == QLatin1String("-egid")) {
            if (!args.size())
                usage();
            if (::setegid(args.takeFirst().toInt()) < 0)
                qWarning("Unable to setegid: %s", strerror(errno));
        }
    }
    if (args.size()) {
        while (args.size())
            dumpInfo(args.takeFirst().toLongLong());
    } else {
        dumpInfo(::getpid());
    }
    // return app.exec();
}

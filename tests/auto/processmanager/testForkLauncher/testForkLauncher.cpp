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
#include <QSocketNotifier>
#include <QTimer>
#include <QDebug>
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>

#include "qforklauncher.h"

#if defined(Q_OS_LINUX)
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#endif
#include <pwd.h>
#include <signal.h>

QT_USE_NAMESPACE_PROCESSMANAGER

class Container : public QObject
{
    Q_OBJECT

public:
    Container() {
        m_in  = new QSocketNotifier( STDIN_FILENO, QSocketNotifier::Read, this );
        connect(m_in, SIGNAL(activated(int)), SLOT(inReady(int)));
        m_in->setEnabled(true);
        m_out = new QSocketNotifier( STDOUT_FILENO, QSocketNotifier::Write, this );
        connect(m_out, SIGNAL(activated(int)), SLOT(outReady(int)));
        m_out->setEnabled(false);
    }

    void handleMessage(const QString& cmd) {
        if (cmd == QLatin1String("stop")) {
            qDebug() << "Stopping";
            exit(0);
        }
        else if (cmd == QLatin1String("crash")) {
            qDebug() << "Crashing";
            exit(2);
        }
        else {
            m_outbuf.append(cmd.toLatin1());
            m_outbuf.append('\n');
            m_out->setEnabled(true);
        }
    }

public slots:
    void inReady(int fd) {
        m_in->setEnabled(false);
        const int bufsize = 1024;
        uint oldSize = m_inbuf.size();
        m_inbuf.resize(oldSize + bufsize);
        int n = ::read(fd, m_inbuf.data()+oldSize, bufsize);
        if (n > 0)
            m_inbuf.resize(oldSize+n);
        else
            m_inbuf.resize(oldSize);

        int offset;
        while ((offset=m_inbuf.indexOf('\n')) != -1) {
            QByteArray msg = m_inbuf.left(offset);
            m_inbuf = m_inbuf.mid(offset + 1);
            if (msg.size() > 0)
                handleMessage(QString::fromLocal8Bit(msg));
        }
        m_in->setEnabled(true);
    }

    void outReady(int fd) {
        m_out->setEnabled(false);
        if (m_outbuf.size()) {
            int n = ::write(fd, m_outbuf.data(), m_outbuf.size());
            if (n == -1) {
                qDebug() << "Failed to write to stdout";
                exit(-1);
            }
            if (n < m_outbuf.size())
                m_outbuf = m_outbuf.mid(n);
            else
                m_outbuf.clear();
        }
        if (m_outbuf.size())
            m_out->setEnabled(true);
    }

private:
    QSocketNotifier *m_in, *m_out;
    QByteArray       m_inbuf, m_outbuf;
};

extern "C" Q_DECL_EXPORT int
main(int argc, char **argv)
{
    qForkLauncher(&argc, &argv);
    QCoreApplication app(argc, argv);

    for (int i = 1 ; i < argc ; i++) {
        if (!strcmp(argv[i], "-noterm")) {
            struct sigaction action;
            ::memset(&action, 0, sizeof(action));
            action.sa_handler=SIG_IGN;
            if (::sigaction(SIGTERM, &action, NULL) < 0) {
                ::perror("Unable ignore SIGTERM");
                return 1;
            }
            qDebug() << "Running in tough mode";
        }
    }

    Container c;
    return app.exec();
}

#include "testForkLauncher.moc"

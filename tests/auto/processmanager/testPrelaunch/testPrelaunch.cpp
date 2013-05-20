/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
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
#include "qprocessinfo.h"
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

#if defined(Q_OS_LINUX)
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#endif
#include <pwd.h>

QT_USE_NAMESPACE_PROCESSMANAGER

class Container : public QObject
{
    Q_OBJECT

public:
    Container() : count(0) {
        m_in  = new QSocketNotifier( STDIN_FILENO, QSocketNotifier::Read, this );
        connect(m_in, SIGNAL(activated(int)), SLOT(inReady(int)));
        m_in->setEnabled(true);
        m_out = new QSocketNotifier( STDOUT_FILENO, QSocketNotifier::Write, this );
        connect(m_out, SIGNAL(activated(int)), SLOT(outReady(int)));
        m_out->setEnabled(false);
    }

    void handleMessage(const QJsonObject& object) {
        if (!count) {
            QProcessInfo info(object.toVariantMap());
            // qDebug() << "Received process info" << info.toMap();
            qint64 uid = (info.contains(QProcessInfoConstants::Uid) ? info.uid() : -1);
            qint64 gid = (info.contains(QProcessInfoConstants::Gid) ? info.gid() : -1);
            if (gid >= 0)
                ::setgid(gid);
            if (uid >= 0)
                ::setuid(uid);
#if !defined(Q_OS_LINUX_ANDROID)
            // Bionic does not have a getpwent() function defined
            struct passwd * pw = getpwent();
            if (pw)
                ::initgroups(pw->pw_name, pw->pw_gid);
            else {
                qWarning() << "Unable to find UID" << ::getuid() << "to set groups";
                ::setgroups(0,0);
            }
#else
            ::setgroups(0,0);
#endif
        }
        else {
            QString cmd = object.value("command").toString();
            // qDebug() << "Received command" << cmd;
            if (cmd == QLatin1String("stop")) {
                // qDebug() << "Stopping";
                exit(0);
            }
            else if (cmd == QLatin1String("crash")) {
                // qDebug() << "Crashing";
                exit(2);
            }
            else {
                m_outbuf.append(cmd.toLatin1());
                m_outbuf.append('\n');
                m_out->setEnabled(true);
            }
        }
        count++;
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
        // Could check for an error here
        // Check for a complete JSON object
        while (m_inbuf.size() >= 12) {
            qint32 message_size = qFromLittleEndian(((qint32 *)m_inbuf.data())[2]) + 8;
            if (m_inbuf.size() < message_size)
                break;
            QByteArray msg = m_inbuf.left(message_size);
            m_inbuf = m_inbuf.mid(message_size);
            handleMessage(QJsonDocument::fromBinaryData(msg).object());
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
    int              count;
};

const int kNumThreads = 4;
void * work(void *)
{
    while (1)
        sleep(1);
    pthread_exit((void *)0);
    return 0;
}

int
main(int argc, char **argv)
{
    pthread_t thread[kNumThreads];

    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    QString progname = args.takeFirst();
    while (args.size()) {
        QString arg = args.at(0);
        if (!arg.startsWith('-'))
            break;
        args.removeFirst();
        if (arg == QStringLiteral("-noterm")) {
            struct sigaction action;
            memset(&action, 0, sizeof(action));
            action.sa_handler=SIG_IGN;
            if (sigaction(SIGTERM, &action, NULL) < 0) {
                perror("Unable ignore SIGTERM");
                return 1;
            }
            qDebug() << "tough";
        }
        else if (arg == QStringLiteral("-threads")) {
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            for (int j = 0 ; j < kNumThreads ; j++) {
                int result = pthread_create(&thread[j], &attr, work, (void *)j);
                if (result) {
                    printf("Error in pthread_created: %d\n", result);
                    return 3;
                }
            }
            pthread_attr_destroy(&attr);
        }
    }

    Container c;
    return app.exec();
}

#include "testPrelaunch.moc"

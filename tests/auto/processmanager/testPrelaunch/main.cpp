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
#include "qjsondocument.h"
#include "processinfo.h"

QT_USE_NAMESPACE_PROCESSMANAGER

class Container : public QObject
{
    Q_OBJECT

public:
    Container() : count(0) {
        notifier = new QSocketNotifier( STDIN_FILENO, QSocketNotifier::Read, this );
        connect(notifier, SIGNAL(activated(int)), SLOT(dataReady()));
        notifier->setEnabled(true);
    }

    void handleMessage(const QVariantMap& map) {
        if (!count) {
            ProcessInfo info(map);
            qDebug() << "Received process info" << info.toMap();
        }
        else {
            QString cmd = map.value("command").toString();
            qDebug() << "Received command" << cmd;
            if (cmd == "stop") {
                qDebug() << "Stopping";
                exit(0);
            }
        }
        count++;
    }

public slots:
    void dataReady() {
        qDebug() << Q_FUNC_INFO;
        notifier->setEnabled(false);
        const int bufsize = 1024;
        uint oldSize = buffer.size();
        buffer.resize(oldSize + bufsize);
        int n = ::read( STDIN_FILENO, buffer.data()+oldSize, bufsize);
        if (n > 0)
            buffer.resize(oldSize+n);
        else
            buffer.resize(oldSize);
        // Could check for an error here
        // Check for a complete JSON object
        while (buffer.size() >= 12) {
            qint32 message_size = ((qint32 *)buffer.data())[2] + 8;  // Should use 'sizeof(Header)'
            if (buffer.size() < message_size)
                break;
            QByteArray msg = buffer.left(message_size);
            buffer = buffer.mid(message_size);
            handleMessage(QJsonDocument::fromBinaryData(msg).toVariant().toMap());
        }
        notifier->setEnabled(true);
    }

private:
    QSocketNotifier *notifier;
    QByteArray       buffer;
    int              count;
};

int
main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Container c;
    qDebug() << "testPrelaunch running";
    return app.exec();
}

#include "main.moc"

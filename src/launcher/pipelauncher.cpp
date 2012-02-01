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

#include <signal.h>
#include <QDebug>
#include <QtEndian>
#include <QJsonDocument>

#include "pipelauncher.h"
#include "launcherclient.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \class PipeLauncher
  \brief The PipeLauncher class accepts input from STDIN and writes data to STDOUT
 */

/*!
  Construct a PipeLauncher with optional \a parent.
  The PipeLauncher reads JSON-formatted messages from stdin and
  writes replies on stdout.
*/

PipeLauncher::PipeLauncher(QObject *parent)
    : ProcessBackendManager(parent)
{
    m_in = new QSocketNotifier( STDIN_FILENO, QSocketNotifier::Read, this );
    connect(m_in, SIGNAL(activated(int)), SLOT(inReady(int)));
    m_in->setEnabled(true);
    m_out = new QSocketNotifier( STDOUT_FILENO, QSocketNotifier::Write, this );
    connect(m_out, SIGNAL(activated(int)), SLOT(outReady(int)));
    m_out->setEnabled(false);

    m_client = new LauncherClient(this);
    connect(m_client, SIGNAL(send(const QJsonObject&)),
            SLOT(send(const QJsonObject&)));
}

/*!
  \internal
 */
void PipeLauncher::inReady(int fd)
{
//    qDebug() << Q_FUNC_INFO;
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
        QJsonObject message = QJsonDocument::fromBinaryData(msg).object();
        if (message.value("remote").toString() == "stop")
            deleteLater();
        m_client->receive(message);
    }
    m_in->setEnabled(true);
}

/*!
  \internal
 */
void PipeLauncher::outReady(int fd)
{
//    qDebug() << Q_FUNC_INFO;
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

/*!
  \internal
 */
void PipeLauncher::send(const QJsonObject& object)
{
    QByteArray data = QJsonDocument(object).toBinaryData();
    m_outbuf.append(data);
    m_out->setEnabled(true);
}

#include "moc_pipelauncher.cpp"

QT_END_NAMESPACE_PROCESSMANAGER

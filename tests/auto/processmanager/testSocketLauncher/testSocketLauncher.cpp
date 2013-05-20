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
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <qjsonserver.h>
#include <qjsonschemavalidator.h>

#include "qsocketlauncher.h"
#include "qstandardprocessbackendfactory.h"
#include "qprelaunchprocessbackendfactory.h"
#include "qprocessinfo.h"

QT_USE_NAMESPACE_PROCESSMANAGER

QString progname;

static void usage()
{
    qWarning("Usage: %s [ARGS] <socketname>\n"
             "\n"
             "The socketname is the name of the Unix local socket to listen on\n"
             "\n"
             "Valid arguments:\n"
             "   -prelaunch PROGRAM       Create prelaunch launcher instead of standard\n"
             "   -validate-inbound PATH   Directory where inbound schema are stored\n"
             "   -validate-outbound PATH  Directory where outbound schema are stored\n"
             "   -warn                    Warn on invalid messages\n"
             "   -drop                    Drop invalid messages\n"
             , qPrintable(progname));
    exit(1);
}

class ValidateMessage : public QObject {
    Q_OBJECT
public:
    ValidateMessage(QObject *parent=0) : QObject(parent) {}
public slots:
    void failed(const QJsonObject& message, const QtAddOn::QtJsonStream::QJsonSchemaError& error) {
        qDebug() << Q_FUNC_INFO << "Message failed to validate" << message << error;
    }
};

static void loadSchemasFromDirectory(QtAddOn::QtJsonStream::QJsonSchemaValidator *validator, const QString& path)
{
    int count = 0;
    QDir dir(path);
    if (!dir.exists())
        qFatal("Schema directory '%s' does not exist", qPrintable(path));

    dir.setNameFilters( QStringList() << "*.json" );
    foreach (QString filename, dir.entryList(QDir::Files | QDir::Readable)) {
        if (!validator->loadFromFile(dir.filePath(filename))) {
            QtAddOn::QtJsonStream::QJsonSchemaError err = validator->getLastError();
            qFatal("Error loading schema file '%s', [%d] %s",
                   qPrintable(dir.filePath(filename)), err.errorCode(), qPrintable(err.errorString()));
        }
        count += 1;
    }

    if (count == 0)
        qFatal("Unable to find any schema files in directory '%s'", qPrintable(path));
    qDebug() << progname << ": loaded" << count << "schemas from" << path;
}

int main(int argc, char **argv)
{
    QtAddOn::QtJsonStream::QJsonServer::ValidatorFlags flags(QtAddOn::QtJsonStream::QJsonServer::NoValidation);
    QString indir, outdir;

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
        else if (arg == QStringLiteral("-validate-inbound")) {
            if (!args.size())
                usage();
            indir = args.takeFirst();
            QFileInfo fi(indir);
            if (!fi.exists() || !fi.isDir()) {
                qWarning("Invalid inbound validation directory '%s'", qPrintable(indir));
                exit(1);
            }
        }
        else if (arg == QStringLiteral("-validate-outbound")) {
            if (!args.size())
                usage();
            outdir = args.takeFirst();
            QFileInfo fi(outdir);
            if (!fi.exists() || !fi.isDir()) {
                qWarning("Invalid outbound validation directory '%s'", qPrintable(outdir));
                exit(1);
            }
        }
        else if (arg == QStringLiteral("-warn"))
            flags |= QtAddOn::QtJsonStream::QJsonServer::WarnIfInvalid;
        else if (arg == QStringLiteral("-drop"))
            flags |= QtAddOn::QtJsonStream::QJsonServer::DropIfInvalid;
        else {
            qWarning("Unexpected argument '%s'", qPrintable(arg));
            usage();
        }
    }

    if (args.size() != 1)
        usage();

    QSocketLauncher launcher;
    if (!prelaunch_program.isEmpty()) {
        QProcessInfo info;
        info.setValue("program", prelaunch_program);
        QPrelaunchProcessBackendFactory *factory = new QPrelaunchProcessBackendFactory;
        factory->setProcessInfo(info);
        launcher.addFactory(factory);
    }
    else
        launcher.addFactory(new QStandardProcessBackendFactory);

    if (!indir.isEmpty())
        loadSchemasFromDirectory(launcher.server()->inboundValidator(), indir);
    if (!outdir.isEmpty())
        loadSchemasFromDirectory(launcher.server()->outboundValidator(), outdir);
    launcher.server()->setValidatorFlags(flags);

    ValidateMessage *vtrap = new ValidateMessage;

    QObject::connect(launcher.server(),
                     SIGNAL(inboundMessageValidationFailed(const QJsonObject&, const QtAddOn::JsonStream::SchemaError&)),
                     vtrap, SLOT(failed(const QJsonObject&, const QtAddOn::JsonStream::SchemaError&)));
    QObject::connect(launcher.server(),
                     SIGNAL(outboundMessageValidationFailed(const QJsonObject&, const QtAddOn::JsonStream::SchemaError&)),
                     vtrap, SLOT(failed(const QJsonObject&, const QtAddOn::JsonStream::SchemaError&)));

    launcher.listen(args[0]);
    return app.exec();
}

#include "testSocketLauncher.moc"

/*
 * Copyright (C) 2011 Nokia Corporation
 */

#ifndef DECLARATIVE_PROCESS_H
#define DECLARATIVE_PROCESS_H

#include "processfrontend.h"

class Q_ADDON_PROCESSMANAGER_EXPORT DeclarativeProcess : public ProcessFrontend
{
    Q_OBJECT

    Q_PROPERTY(QString identifier READ identifier CONSTANT)
    Q_PROPERTY(QString program READ program CONSTANT)
    Q_PROPERTY(QStringList arguments READ arguments CONSTANT)
    Q_PROPERTY(QVariantMap environment READ environment CONSTANT)
    Q_PROPERTY(QString workingDirectory READ workingDirectory CONSTANT)
    Q_PROPERTY(qint64 uid READ uid CONSTANT)
    Q_PROPERTY(qint64 gid READ gid CONSTANT)

    Q_PROPERTY(qint64 pid READ pid NOTIFY started)
    Q_PROPERTY(qint64 startTime READ startTime NOTIFY started)

    Q_PROPERTY(int priority READ priority WRITE setPriority NOTIFY priorityChanged)
#if defined(Q_OS_LINUX)
    Q_PROPERTY(int oomAdjustment READ oomAdjustment WRITE setOomAdjustment NOTIFY oomAdjustmentChanged)
#endif

public:
    DeclarativeProcess(ProcessBackend *process, QObject *parent=0);
    virtual ~DeclarativeProcess();
};

Q_DECLARE_METATYPE(DeclarativeProcess*)

#endif // DECLARATIVE_PROCESS_H

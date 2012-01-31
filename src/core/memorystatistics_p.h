#ifndef MEMORYMANAGER_P_H
#define MEMORYMANAGER_P_H

#include <QString>

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class MappedRegion {
public:
    quint64 addressStart;
    quint64 addressEnd;
    quint64 rssSize;
    quint64 pssSize;
    quint64 privateSize;
    QString pathname;

    static QString libBaseName(const QString &pathName);
    QString libBaseName() const;
};

// proc/pid/stat
class stat {
public:
    stat()
        : ppid(0)
        , pgrp(0)
        , session(0)
        , it_real_value(0)
        , start_time(0)
        , vsize(0)
        , rss(0) {}
    QByteArray comm;
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tty_pgrp;
    ulong flags;
    ulong min_flt;
    ulong cmin_flt;
    ulong maj_flt;
    ulong cmaj_flt;
    ulong tms_utime;
    ulong tms_stime;
    long tms_cutime;
    long tms_cstime;
    long priority;
    long nice;

    long it_real_value;
    ulong start_time;
    ulong vsize;
    long rss; /* you might want to shift this left 3 */
    ulong rlim;
    ulong start_code;
    ulong end_code;
    ulong start_stack;
    ulong esp;
    ulong eip;

    ulong wchan;
    ulong nswap;
    ulong cnswap;
    int exit_signal;
    int processor;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // MEMORYMANAGER_P_H

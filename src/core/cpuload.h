#ifndef CPULOAD_H
#define CPULOAD_H

#include <QList>
#include <QByteArray>

class CPULoad
{
public:

    CPULoad();

    void init();
    void update();
    int  cpuLoad() const;

private:
    typedef QList<QByteArray> TimeList;

    CPULoad(const CPULoad &);
    CPULoad & operator= (const CPULoad &);
    TimeList  readTimeList();

private:
    TimeList   m_timeList;
    int        m_load;
    static int m_forcedLoad;
};

#endif // CPULOAD_H

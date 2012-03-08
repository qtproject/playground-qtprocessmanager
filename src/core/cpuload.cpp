#include "cpuload.h"
#include <QFile>
#include <QDebug>

/*!
 * \class CPULoad
 * \brief CPULoad calculates the current CPU load.
 */
int CPULoad::m_forcedLoad = -1;

//! Constructor
CPULoad::CPULoad() :
    m_load(-1)
{}

/*!
 * Reads /proc/stat file
 * ToDo: use sscanf if it faster
 */
CPULoad::TimeList CPULoad::readTimeList()
{
#ifdef Q_OS_LINUX
    QFile fin(QStringLiteral("/proc/stat"));
    if (fin.open(QIODevice::ReadOnly | QIODevice::Text))
        return fin.readLine().split(' ');
#endif
    return TimeList();
}

/*!
 * Initialize. Currently only sets the value to -1.
 */
void CPULoad::init()
{
    m_load = -1;
}

/*!
 * Update the value.
 * This must be called periodically (e.g. 1 - 5 seconds) in order to update the measurement.
 * The data is obtained from first line of /proc/stat file
 */
void CPULoad::update()
{
    // Read timing from the file
    TimeList v(readTimeList());

    if (m_timeList.length()) {
        // Take delta with the previous timing
        int sum  = 0;
        int idle = 0;
        int iowait = 0;
        int cpu = 0;

        for (int i = 2; i < 7 && i < v.length(); i++)
            sum += v.at(i).toInt() - m_timeList.at(i).toInt();

        idle = v.at(5).toInt() - m_timeList.at(5).toInt();
        iowait = v.at(6).toInt() - m_timeList.at(6).toInt();
        cpu = sum - idle - iowait;

        // Calculate load
        m_load = 100.0 - (100.0 * idle / sum);
    } else {
        m_load = -1;
    }

    // Store the current timing
    m_timeList = v;
}

/*!
 * Returns The current value of the load in the range of 0 to 100
 * or -1 if nothing measured.
 */
int CPULoad::cpuLoad() const
{
    return m_forcedLoad > -1 ? m_forcedLoad : m_load;
}

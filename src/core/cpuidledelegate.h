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

#ifndef CPU_IDLE_DELEGATE_H
#define CPU_IDLE_DELEGATE_H

#include <QTimer>
#include "idledelegate.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessInfo;

class Q_ADDON_PROCESSMANAGER_EXPORT CpuIdleDelegate : public IdleDelegate
{
    Q_OBJECT
    Q_PROPERTY(int idleInterval READ idleInterval WRITE setIdleInterval NOTIFY idleIntervalChanged)
    Q_PROPERTY(int loadThreshold READ loadThreshold WRITE setLoadThreshold NOTIFY loadThresholdChanged)

public:
    explicit CpuIdleDelegate(QObject *parent = 0);

    int     idleInterval() const;
    void    setIdleInterval(int interval);

    double  loadThreshold() const;
    void    setLoadThreshold(double threshold);

signals:
    void idleCpuAvailable();
    void idleIntervalChanged();
    void loadThresholdChanged();
    void loadUpdate(double);

protected:
    virtual void handleStateChange(bool state);

private slots:
    void timeout();

private:
    double updateStats(bool);

private:
    Q_DISABLE_COPY(CpuIdleDelegate);
    QTimer m_timer;
    double m_load, m_loadThreshold;
    int    m_total, m_idle;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // CPU_IDLE_DELEGATE_H

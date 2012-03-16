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

#ifndef PREFORK_H
#define PREFORK_H

#include "processmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

struct Q_ADDON_PROCESSMANAGER_EXPORT PreforkChildData {
    int stdin;      // Child stdin (write to this)
    int stdout;     // Child stdout (read from this)
    int pid;        // Child process ID
};

class Q_ADDON_PROCESSMANAGER_EXPORT Prefork {
public:
    static Prefork *instance();
    void execute(int *argc_ptr, char ***argv_ptr);
    void checkChildDied(pid_t pid);

    int  size() const;
    const PreforkChildData *at(int i) const;

private:
    Prefork();

    int  nextMarker(int index);
    void launch(int start, int end);
    int  makeChild(int start);

private:
    int    m_argc;       // Original number of arguments
    char **m_argv;       // Original pointer to argument
    size_t m_argv_size;  // Length of vector allocated to original list
    int    m_count;      // Number of child processes forked
    PreforkChildData *m_children;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PREFORK_H

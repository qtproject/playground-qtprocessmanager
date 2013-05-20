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

#ifndef PREFORK_H
#define PREFORK_H

#include "qprocessmanager-global.h"

QT_BEGIN_NAMESPACE_PROCESSMANAGER

struct Q_ADDON_PROCESSMANAGER_EXPORT QPreforkChildData {
    int in;      // Child stdin (write to this)
    int out;     // Child stdout (read from this)
    int pid;     // Child process ID
};

class Q_ADDON_PROCESSMANAGER_EXPORT QPrefork {
public:
    static QPrefork *instance();
    void execute(int *argc_ptr, char ***argv_ptr);
    void checkChildDied(pid_t pid);

    int  size() const;
    const QPreforkChildData *at(int i) const;

private:
    QPrefork();

    int  nextMarker(int index);
    void launch(int start, int end);
    int  makeChild(int start);

private:
    int    m_argc;       // Original number of arguments
    char **m_argv;       // Original pointer to argument
    size_t m_argv_size;  // Length of vector allocated to original list
    int    m_count;      // Number of child processes forked
    QPreforkChildData *m_children;
};

QT_END_NAMESPACE_PROCESSMANAGER

#endif // PREFORK_H

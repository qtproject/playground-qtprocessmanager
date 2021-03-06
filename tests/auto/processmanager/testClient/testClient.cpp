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

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

const int kBufSize = 100;

ssize_t writeline(char *buffer, int len)
{
    char *b = buffer;
    while ( len > 0 ) {
        ssize_t r = write(STDOUT_FILENO, b, len);
        if (r < 0)
            return r;
        b += r;
        len -= r;
    }
    return 0;
}

ssize_t readline(char *buffer, int max_len)
{
    int len = 0;
    char c = 0;
    while (len < max_len && c != '\r' && c != '\n') {
        ssize_t count = read(STDIN_FILENO, &c, 1);
        if (count <= 0)
            return count;
        *buffer++ = c;
        len++;
    }
    *buffer = 0;
    return len;
}

void * work(void *)
{
    while (1)
        sleep(1);
    pthread_exit((void *)0);
    return 0;
}

static char tough[] = "tough\n";
const int kNumThreads = 4;

int
main(int argc, char **argv)
{
    pthread_t thread[kNumThreads];

    for (int i = 1 ; i < argc ; i++) {
        if (!strcmp(argv[i], "-noterm")) {
            struct sigaction action;
            memset(&action, 0, sizeof(action));
            action.sa_handler=SIG_IGN;
            if (sigaction(SIGTERM, &action, NULL) < 0) {
                perror("Unable ignore SIGTERM");
                return 1;
            }
            if (writeline(tough, strlen(tough)) < 0)
                return 3;
        }
        else if (!strcmp(argv[i], "-threads")) {
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            for (int j = 0 ; j < kNumThreads ; j++) {
                int result = pthread_create(&thread[j], &attr, work, (void *)j);
                if (result) {
                    printf("Error in pthread_created: %d\n", result);
                    return 3;
                }
            }
            pthread_attr_destroy(&attr);
        }
    }

    char buffer[kBufSize+1];
    while (1) {
        ssize_t count = readline(buffer, kBufSize);
        if (count < 0)
            return 1;
        if (count == 0)
            return 0;

        if (strncmp("stop", buffer, 4) == 0)
            return 0;
        if (strncmp("crash", buffer, 5) == 0)
            return 2;

        ssize_t result = writeline(buffer, count);
        if (result < 0)
            return 2;
    }

    pthread_exit(NULL);
}

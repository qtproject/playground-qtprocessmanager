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

#include <QtTest>

#include "matchinfo.h"

QT_USE_NAMESPACE_PROCESSMANAGER

/******************************************************************************/

class TestMatcher : public QObject
{
    Q_OBJECT
public:
    TestMatcher(QObject *parent=0);

private Q_SLOTS:
    void matchProgram();
    void matchEnvironment();
};

TestMatcher::TestMatcher(QObject *parent)
  : QObject(parent)
{
}

void TestMatcher::matchProgram()
{
    ProcessInfo match_info;
    match_info.setProgram("/usr/bin/abc");

    MatchInfo matcher;
    matcher.setInfo(match_info);

    ProcessInfo info;
    info.setProgram("/usr/bin/abc");
    QVERIFY(matcher.matches(info));  // Exactly the same value

    info.setProgram("abc");
    QVERIFY(!matcher.matches(info));  // Short name shouldn't match
}

static void _makeEnvironmentInfo(ProcessInfo& info,
                                 const char *key1 = 0,
                                 const char *value1 = 0,
                                 const char *key2 = 0,
                                 const char *value2 = 0)
{
    QProcessEnvironment test_env;
    if (key1)
        test_env.insert(QLatin1String(key1), QLatin1String(value1));
    if (key2)
        test_env.insert(QLatin1String(key2), QLatin1String(value2));
    info.setEnvironment(test_env);
}

static void _testEnvironment(MatchInfo& matcher,
                             bool answer,
                             const char *key1 = 0,
                             const char *value1 = 0,
                             const char *key2 = 0,
                             const char *value2 = 0)
{
    ProcessInfo test_info;
    _makeEnvironmentInfo(test_info, key1, value1, key2, value2);
    QCOMPARE(matcher.matches(test_info), answer);
}

void TestMatcher::matchEnvironment()
{
    MatchInfo matcher;
    ProcessInfo match_info;
    _makeEnvironmentInfo(match_info, "test", "value");
    matcher.setInfo(match_info);

    _testEnvironment(matcher, false);  // Missing environment
    _testEnvironment(matcher, false, "random", "222");  // Wrong attribute
    _testEnvironment(matcher, false, "test", "222");  // Wrong value
    _testEnvironment(matcher, true, "test", "value");  // Exact match
    _testEnvironment(matcher, true, "random", "222", "test", "value"); // Extra field

    _makeEnvironmentInfo(match_info, "test", "value", "test2", "value2");
    matcher.setInfo(match_info);

    _testEnvironment(matcher, false);  // Both missing
    _testEnvironment(matcher, false, "random", "222");  // Random data
    _testEnvironment(matcher, false, "test", "value");  // Only one match
    _testEnvironment(matcher, false, "test2", "value2"); // Only one match
    _testEnvironment(matcher, true, "test2", "value2", "test", "value");  // Correct
    _testEnvironment(matcher, true, "test", "value", "test2", "value2");  // Different order
}

QTEST_MAIN(TestMatcher)
#include "tst_matcher.moc"

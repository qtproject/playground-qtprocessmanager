/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtAddOn.JsonStream module of the Qt.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest>

#include "qinfomatchdelegate.h"
#include "qkeymatchdelegate.h"

QT_USE_NAMESPACE_PROCESSMANAGER

/******************************************************************************/

static void _testKeyMatch(QProcessInfo& info,
                          bool answer,
                          const char *key=0,
                          const QVariant& value=QVariant())
{
    QKeyMatchDelegate matcher;
    if (key)
        matcher.setKey(key);
    if (!value.isNull())
        matcher.setValue(value);
    QCOMPARE(matcher.matches(info), answer);
}

static void _makeEnvironmentInfo(QProcessInfo& info,
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

static void _testEnvironment(QInfoMatchDelegate& matcher,
                             bool answer,
                             const char *key1 = 0,
                             const char *value1 = 0,
                             const char *key2 = 0,
                             const char *value2 = 0)
{
    QProcessInfo test_info;
    _makeEnvironmentInfo(test_info, key1, value1, key2, value2);
    QCOMPARE(matcher.matches(test_info), answer);
}

/******************************************************************************/

class TestMatcher : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void matchKey();
    void matchProgram();
    void matchEnvironment();
};

void TestMatcher::matchKey()
{
    QProcessInfo info;
    info.setProgram("/usr/bin/abc");
    info.setValue("prelaunch", "true");

    _testKeyMatch(info, false);
    _testKeyMatch(info, true, "prelaunch");
    _testKeyMatch(info, false, "prelunch");
    _testKeyMatch(info, true, "prelaunch", "true");
    _testKeyMatch(info, false, "prelaunch", "false");
}

void TestMatcher::matchProgram()
{
    QProcessInfo match_info;
    match_info.setProgram("/usr/bin/abc");

    QInfoMatchDelegate matcher;
    matcher.setInfo(match_info);

    QProcessInfo info;
    info.setProgram("/usr/bin/abc");
    QVERIFY(matcher.matches(info));  // Exactly the same value

    info.setProgram("abc");
    QVERIFY(!matcher.matches(info));  // Short name shouldn't match
}

void TestMatcher::matchEnvironment()
{
    QInfoMatchDelegate matcher;
    QProcessInfo match_info;
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

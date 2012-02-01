%modules = ( # path to module name map
    "QtAddOnProcessManager" => "$basedir/src/core",
    "QtAddOnProcessManagerDeclarative" => "$basedir/src/declarative",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
    "qtaddonprocessmanagerversion.h" => "QtAddOnProcessManagerVersion",
);
%mastercontent = (
    "core" => "#include <QtCore/QtCore>\n",
    "network" => "#include <QtNetwork/QtNetwork>\n",
);
%modulepris = (
    "QtAddOnProcessManager" => "$basedir/modules/qt_processmanager.pri",
    "QtAddOnProcessManagerDeclarative" => "$basedir/modules/qt_processmanagerdeclarative.pri",
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
        "qtbase" => "refs/heads/master",
        "qtdeclarative" => "refs/heads/master",
        "qtjsbackend" => "refs/heads/master",
);

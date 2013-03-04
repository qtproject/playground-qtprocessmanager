%modules = ( # path to module name map
    "QtAddOnProcessManager" => "$basedir/src/processmanager",
    "QtAddOnProcessManagerDeclarative" => "$basedir/src/processmanager-declarative",
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
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
        "qtbase" => "refs/heads/stable",
        "qtdeclarative" => "refs/heads/stable",
        "qtjsbackend" => "refs/heads/stable",
);

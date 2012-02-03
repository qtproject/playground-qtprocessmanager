#include "processinfotemplate.h"
#include "declarativeprocessmanager.h"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeExtensionPlugin>


QT_BEGIN_NAMESPACE_PROCESSMANAGER

/*!
  \qmlclass Manager DeclarativeProcessManager
  \inqmlmodule ProcessManager 1
  \brief The Manager element controls running processes.

  Only a single DeclarativeProcessManager class should be loaded at one time.

  Typical use of the ProcessManager class is as follows:

  \code
  import QtQuick 2.0
  import ProcessManager 1.0

  Manager {
     id: myProcessManager

     factories: [
        GdbProcessBackendFactory {},
        StandardProcessBackendFactory {}
     ]
  }
  \endcode
*/

/*!
  \qmlproperty list ProcessManager::Manager::factories
  \brief The factories assigned to this process manager

   The factories property is an ordered list of ProcessBackendFactory objects.
*/


class ProcessManagerPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    void registerTypes(const char *uri) {
        qmlRegisterType<ProcessInfoTemplate>(uri, 1, 0, "ProcessInfoTemplate");
        qmlRegisterType<DeclarativeProcessManager>(uri, 1, 0, "Manager");
    }

    void initializeEngine(QDeclarativeEngine *engine, const char *uri) {
        Q_UNUSED(engine);
        Q_UNUSED(uri);
    }
};

#include "plugin.moc"

Q_EXPORT_PLUGIN2(processmanagertemplateplugin, ProcessManagerPlugin)

QT_END_NAMESPACE_PROCESSMANAGER

#include "processinfotemplate.h"
#include "declarativeprocessmanager.h"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessManagerPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    void registerTypes(const char *uri) {
        qmlRegisterType<ProcessInfoTemplate>(uri, 1, 0, "ProcessInfoTemplate");
        qmlRegisterType<DeclarativeProcessManager>(uri, 1, 0, "ProcessManager");
    }

    void initializeEngine(QDeclarativeEngine *engine, const char *uri) {
        Q_UNUSED(engine);
        Q_UNUSED(uri);
    }
};

#include "plugin.moc"

Q_EXPORT_PLUGIN2(processmanagertemplateplugin, ProcessManagerPlugin)

QT_END_NAMESPACE_PROCESSMANAGER

#include "processinfotemplate.h"
#include "declarativeprocessmanager.h"

#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>


QT_BEGIN_NAMESPACE_PROCESSMANAGER

class ProcessManagerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
public:
    void registerTypes(const char *uri) {
        DeclarativeProcessManager::registerTypes(uri);
    }

    void initializeEngine(QQmlEngine *engine, const char *uri) {
        Q_UNUSED(engine);
        Q_UNUSED(uri);
    }
};

#include "plugin.moc"

Q_EXPORT_PLUGIN2(processmanagertemplateplugin, ProcessManagerPlugin)

QT_END_NAMESPACE_PROCESSMANAGER

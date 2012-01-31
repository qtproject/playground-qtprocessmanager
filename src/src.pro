TEMPLATE = subdirs

module_processmanager_core.subdir = core
module_processmanager_core.target = module-processmanager-core

module_processmanager_declarative.subdir = declarative
module_processmanager_declarative.target = module-processmanager-declarative
module_processmanager_declarative.depends += module-processmanager-core

module_processmanager_launcher.subdir = launcher
module_processmanager_launcher.target = module-processmanager-launcher
module_processmanager_launcher.depends += module-processmanager-core

SUBDIRS += \
  module_processmanager_core \
  module_processmanager_declarative \
  module_processmanager_launcher

import QtQuick 2.0
import Test 1.0

DeclarativeSocketLauncher {
  id: socket_launcher

  factories: [
    StandardProcessBackendFactory {
      id: gdbFactory
      matchDelegate: KeyMatchDelegate { key: "gdb" }
      rewriteDelegate: GdbRewriteDelegate {}
    },
    StandardProcessBackendFactory {
      id: standardFactory
    }
  ]

  Component.onCompleted: {
    socket_launcher.listen("/tmp/socket_launcher");
    console.log("Listening on socket");
  }
}

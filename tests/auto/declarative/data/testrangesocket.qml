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

  JsonUIDRangeAuthority {
      id: rangeAuthority
      minimum: 0
      maximum: 1000000000
  }

  Component.onCompleted: {
    console.log("Range authority : " +rangeAuthority);
    socket_launcher.listen("/tmp/socket_launcher", rangeAuthority);
    console.log("Listening on socket");
  }
}

import QtQuick 2.0
import Test 1.0

PmManager {
  id: foo

  factories: [
    SocketProcessBackendFactory {
      socketName: "/tmp/socket_launcher"
    }
  ]

    onProcessStarted: console.log("Process started "+name)
    onProcessStateChanged: {
      function stateToString(s) {
        if (s == Process.NotRunning) return "Not running";
        if (s == Process.Running) return "Running";
        if (s == Process.Starting) return "Starting";
        return "Unknown";
      }
      console.log("state changed for "+name+" to "+stateToString(state));
    }

    function makeProcess() {
        var a = create({"program": "testDeclarative/testDeclarative","name": "test-client"});
        return a.name;
    }

    function startProcess(name) {
        var p = processForName(name);
        console.log("Starting process '"+name+"' ="+p);
        p.start();
    }

    function stopProcess(name) {
        var p = processForName(name);
        p.stop();
    }

  Component.onCompleted: {
    console.log("Running socket-based process manager");
  }
}

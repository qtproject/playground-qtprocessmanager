import QtQuick 2.0
import Test 1.0

TestManager {
    id: foo
    magic: "fuzzy"

    function makeProcess() {
        var a = create({"program": "testClient/testClient","identifier": "a"});
        console.log("Type="+typeof(a));
        var p = processForIdentifier("a");
        console.log("Process: "+p);
        p.onStarted.connect(foo.itStarted);
    }

    function itStarted() {
        console.log("It started");
    }
}

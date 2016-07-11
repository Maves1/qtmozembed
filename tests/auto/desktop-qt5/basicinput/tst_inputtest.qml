import QtTest 1.0
import QtQuick 2.0
import Qt5Mozilla 1.0
import "../../shared/componentCreation.js" as MyScript
import "../../shared/sharedTests.js" as SharedTests

Item {
    id: appWindow
    width: 480
    height: 800

    property bool mozViewInitialized
    property string inputContent
    property int inputState: -1
    property bool changed
    property int focusChange: -1
    property int cause: -1
    property string inputType

    function isState(state, focus, cause)
    {
        return appWindow.changed === true && appWindow.inputState === state && appWindow.focusChange === focus && appWindow.cause === cause
    }

    QmlMozContext {
        id: mozContext
    }
    Connections {
        target: mozContext.instance
        onOnInitialized: {
            mozContext.instance.addComponentManifest(mozContext.getenv("QTTESTSROOT") + "/components/TestHelpers.manifest")
        }
    }

    QmlMozView {
        id: webViewport
        clip: false
        visible: true
        focus: true
        active: true

        anchors.fill: parent
        onViewInitialized: {
            webViewport.loadFrameScript("chrome://tests/content/testHelper.js")
            webViewport.addMessageListener("testembed:elementpropvalue")
            appWindow.mozViewInitialized = true
        }
        onHandleSingleTap: {
            print("HandleSingleTap: [",point.x,",",point.y,"]")
        }
        onRecvAsyncMessage: {
            // print("onRecvAsyncMessage:" + message + ", data:" + data)
            switch (message) {
            case "testembed:elementpropvalue": {
                // print("testembed:elementpropvalue value:" + data.value)
                appWindow.inputContent = data.value
                break
            }
            default:
                break
            }
        }
        onImeNotification: {
            print("onImeNotification: state:" + state + ", open:" + open + ", cause:" + cause + ", focChange:" + focusChange + ", type:" + type)
            appWindow.changed = true
            appWindow.inputState = state
            appWindow.cause = cause
            appWindow.focusChange = focusChange
            appWindow.inputType = type
        }
    }

    resources: TestCase {
        id: testcaseid
        name: "mozContextPage"
        when: windowShown
        parent: appWindow

        function cleanup() {
            mozContext.dumpTS("tst_inputtest cleanup")
        }

        function test_Test1LoadInputPage()
        {
            SharedTests.shared_Test1LoadInputPage()
        }

        function test_Test1LoadInputURLPage()
        {
            SharedTests.shared_Test1LoadInputURLPage()
        }
    }
}

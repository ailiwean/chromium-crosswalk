description('Tests using DeviceOrientation from multiple frames.');

var mockEvent = {alpha: 1.1, beta: 2.2, gamma: 3.3, absolute: true};
if (window.testRunner)
    testRunner.setMockDeviceOrientation(true, mockEvent.alpha, true, mockEvent.beta, true, mockEvent.gamma, true, mockEvent.absolute);
else
    debug('This test can not be run without the TestRunner');

var deviceOrientationEvent;
function checkOrientation(event) {
    deviceOrientationEvent = event;
    shouldBe('deviceOrientationEvent.alpha', 'mockEvent.alpha');
    shouldBe('deviceOrientationEvent.beta', 'mockEvent.beta');
    shouldBe('deviceOrientationEvent.gamma', 'mockEvent.gamma');
    shouldBe('deviceOrientationEvent.absolute', 'mockEvent.absolute');
}

var hasMainFrameEventFired = false;
function mainFrameListener(event) {
    checkOrientation(event);
    hasMainFrameEventFired = true;
    maybeFinishTest();
}

var hasChildFrameEventFired = false;
function childFrameListener(event) {
    checkOrientation(event);
    hasChildFrameEventFired = true;
    maybeFinishTest();
}

function maybeFinishTest() {
    if (hasMainFrameEventFired && hasChildFrameEventFired)
        finishJSTest();
}

var childFrame = document.createElement('iframe');
document.body.appendChild(childFrame);
childFrame.contentWindow.addEventListener('deviceorientation', childFrameListener);

window.addEventListener('deviceorientation', mainFrameListener);

window.jsTestIsAsync = true;

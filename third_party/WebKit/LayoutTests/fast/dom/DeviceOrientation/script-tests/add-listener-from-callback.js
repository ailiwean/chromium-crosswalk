description('Tests that adding a new event listener from a callback works as expected.');

var mockAlpha = 1.1;
var mockBeta = 2.2;
var mockGamma = 3.3;
var mockAbsolute = true;

if (window.testRunner)
    testRunner.setMockDeviceOrientation(true, mockAlpha, true, mockBeta, true, mockGamma, true, mockAbsolute);
else
    debug('This test can not be run without the TestRunner');

var deviceOrientationEvent;
function checkOrientation(event) {
    deviceOrientationEvent = event;
    shouldBe('deviceOrientationEvent.alpha', 'mockAlpha');
    shouldBe('deviceOrientationEvent.beta', 'mockBeta');
    shouldBe('deviceOrientationEvent.gamma', 'mockGamma');
    shouldBe('deviceOrientationEvent.absolute', 'mockAbsolute');
}

var firstListenerEvents = 0;
function firstListener(event) {
    checkOrientation(event);
    window.removeEventListener('deviceorientation', firstListener);
    if (++firstListenerEvents == 1)
        window.addEventListener('deviceorientation', secondListener);
}

var secondListenerEvents = 0;
function secondListener(event) {
    checkOrientation(event);
    ++secondListenerEvents;
    if (firstListenerEvents != 1 || secondListenerEvents != 1)
        testFailed('Too many events fired for first or second listener');
    finishJSTest();
}

window.addEventListener('deviceorientation', firstListener);

window.jsTestIsAsync = true;

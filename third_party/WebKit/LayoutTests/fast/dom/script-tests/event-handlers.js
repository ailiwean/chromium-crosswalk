function getObject(interface) {
    switch(interface) {
        case "Element":
            var e = document.createElementNS("http://example.com/", "example");
            assert_true(e instanceof Element);
            assert_false(e instanceof HTMLElement);
            assert_false(e instanceof SVGElement);
            return e;
        case "HTMLElement":
            var e = document.createElement("html");
            assert_true(e instanceof HTMLElement);
            return e;
        case "HTMLBodyElement":
            var e = document.createElement("body");
            assert_true(e instanceof HTMLBodyElement);
            return e;
        case "HTMLFormElement":
            var e = document.createElement("form");
            assert_true(e instanceof HTMLFormElement);
            return e;
        case "HTMLFrameSetElement":
            var e = document.createElement("frameset");
            assert_true(e instanceof HTMLFrameSetElement);
            return e;
        case "SVGElement":
            var e = document.createElementNS("http://www.w3.org/2000/svg", "rect");
            assert_true(e instanceof SVGElement);
            return e;
        case "Document":
            assert_true(document instanceof Document);
            return document;
        case "Window":
            assert_true(window instanceof Window);
            return window;
    }
    assert_unreached();
}
function testSet(interface, attribute) {
    test(function() {
        var object = getObject(interface);
        function nop() {}
        assert_equals(object[attribute], null, "Initially null");
        object[attribute] = nop;
        assert_equals(object[attribute], nop, "Return same function");
        object[attribute] = "";
        assert_equals(object[attribute], null, "Return null after setting string");
        object[attribute] = null;
        assert_equals(object[attribute], null, "Finally null");
    }, "Set " + interface + "." + attribute);
}
function testReflect(interface, attribute) {
    test(function() {
        var element = getObject(interface);
        assert_false(element.hasAttribute(attribute), "Initially missing");
        element.setAttribute(attribute, "return");
        assert_equals(element.getAttribute(attribute), "return", "Return same string");
        assert_equals(typeof element[attribute], "function", "Convert to function");
        element.removeAttribute(attribute);
    }, "Reflect " + interface + "." + attribute);
}
// Object.propertyIsEnumerable cannot be used because it doesn't
// work with properties inherited through the prototype chain.
function getEnumerable(interface) {
    var enumerable = {};
    for (var attribute in getObject(interface)) {
        enumerable[attribute] = true;
    }
    return enumerable;
}
var enumerableCache = {};
function testEnumerate(interface, attribute) {
    if (!(interface in enumerableCache)) {
        enumerableCache[interface] = getEnumerable(interface);
    }
    test(function() {
        assert_true(enumerableCache[interface][attribute]);
    }, "Enumerate " + interface + "." + attribute);
}

if (window.layoutTestController && window.eventSender) {
    layoutTestController.waitUntilDone();

    // Give DRT a chance to layout the document with its initial size,
    // before zooming the page. This also tests repainting as side-effect..
    setTimeout(function() {
        for (i = 0; i < zoomCount; ++i)
            eventSender.textZoomIn();
        layoutTestController.notifyDone();
    }, 0);
}

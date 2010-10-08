# WebKit2 - Qt4 build info

# Use a config-specific target to prevent parallel builds file clashes on Mac
mac: CONFIG(debug, debug|release): WEBKIT2_TARGET = webkit2d
else: WEBKIT2_TARGET = webkit2

# Output in WebKit2/<config>
CONFIG(debug, debug|release) : WEBKIT2_DESTDIR = debug
else: WEBKIT2_DESTDIR = release

defineTest(_addWebKit2Lib_common) {
    pathToWebKit2Output = $$ARGS/$$WEBKIT2_DESTDIR

    QMAKE_LIBDIR += $$pathToWebKit2Output

    POST_TARGETDEPS += $${pathToWebKit2Output}$${QMAKE_DIR_SEP}lib$${WEBKIT2_TARGET}.a

    # The following line is to prevent qmake from adding webkit2 to libQtWebKit's prl dependencies.
    CONFIG -= explicitlib

    export(QMAKE_LIBDIR)
    export(POST_TARGETDEPS)
    export(CONFIG)

    return(true)
}

defineTest(addWebKit2Lib) {
    _addWebKit2Lib_common($$ARGS)

    QMAKE_LFLAGS += -l$$WEBKIT2_TARGET
    export(QMAKE_LFLAGS)

    return(true)
}

defineTest(addWebKit2LibWholeArchive) {
    _addWebKit2Lib_common($$ARGS)

    # -whole-archive makes all objects, even if unreferenced, included in the linked target.
    mac:QMAKE_LFLAGS += -Wl,-all_load -l$$WEBKIT2_TARGET
    else:QMAKE_LFLAGS += -Wl,-whole-archive -l$$WEBKIT2_TARGET -Wl,-no-whole-archive
    export(QMAKE_LFLAGS)

    return(true)
}

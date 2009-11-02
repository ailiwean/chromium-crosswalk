TEMPLATE = app
TARGET = tst_qwebview
include(../../../../WebKit.pri)
SOURCES  += tst_qwebview.cpp
QT += testlib network
QMAKE_RPATHDIR = $$OUTPUT_DIR/lib $$QMAKE_RPATHDIR
RESOURCES  += tst_qwebview.qrc
DEFINES += SRCDIR=\\\"$$PWD/\\\"

symbian {
    TARGET.UID3 = 0xA000E53F
    TARGET.CAPABILITY = ReadUserData WriteUserData NetworkServices
}

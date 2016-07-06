#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T22:48:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += static
#CONFIG += console
#win32:LIBS += -static -static-libgcc -static-libstdc++
TARGET = demo2
TEMPLATE = app
  OPENSSL_INCLUDE_PATH=e:/deps/openssl-1.0.1j/include
  OPENSSL_LIB_PATH=e:/deps/openssl-1.0.1j
INCLUDEPATH +=  $$OPENSSL_INCLUDE_PATH
LIBS +=  $$join(OPENSSL_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto
# -lgdi32 has to happen after -lcrypto (see  #681)
LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32

SOURCES += \
    demo2.cpp \
    sendemail.cpp

# Location of SMTP Library

#win32:CONFIG(release, debug|release): LIBS += -L$$SMTP_LIBRARY_LOCATION/release/ -lSMTPEmail
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$SMTP_LIBRARY_LOCATION/debug/ -lSMTPEmail
#else:unix: LIBS += -L$$SMTP_LIBRARY_LOCATION -lSMTPEmail

#INCLUDEPATH += ../staticLibrary
#LIBS += -L../staticLibrary/debug -lstaticLibrary

# working on build both with 541 dynamic.
# building when lib is dynamic 541, and app is static 540 include dynam.-lib. but could not find .dll
# dynamiclib 540 building, app not building- class links bloken
# all static builds are broken links

SMTP_LIBRARY_LOCATION = $$PWD/../..
INCLUDEPATH += $$SMTP_LIBRARY_LOCATION/src
#DEPENDPATH += $$SMTP_LIBRARY_LOCATION/src
#LIBS += $$SMTP_LIBRARY_LOCATION/static/release/libSMTPEmail.a
LIBS += -L$$SMTP_LIBRARY_LOCATION/static/release  -lSMTPEmail
#LIBS += -L$$SMTP_LIBRARY_LOCATION/build/release  -lSMTPEmail

HEADERS += \
    sendemail.h

FORMS += \
    sendemail.ui

#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T20:59:25
#
#-------------------------------------------------

QT +=  core gui network

TARGET = SMTPEmail

# Build as an application
#TEMPLATE = app

# Build as a library
TEMPLATE = lib
DEFINES += SMTP_BUILD
CONFIG += staticlib
#CONFIG += static
#Q_DECL_EXPORT
#Q_DECL_IMPORT
#STATIC=all
#win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat
#win32:QMAKE_LFLAGS *= -Wl,--large-address-aware -static
#win32:QMAKE_LFLAGS +=  -static -static-libgcc -static-libstdc++
#win32:LIBS += -static -static-libgcc -static-libstdc++
#win32:CONFIG += dll
  OPENSSL_INCLUDE_PATH=e:/deps/openssl-1.0.1j/include
  OPENSSL_LIB_PATH=e:/deps/openssl-1.0.1j
INCLUDEPATH +=  $$OPENSSL_INCLUDE_PATH
LIBS +=  $$join(OPENSSL_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto
# -lgdi32 has to happen after -lcrypto (see  #681)
LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32

android: {
    #QT += androidextras
    #USE_ASM=1
}
unix {
  #target.path = /usr/lib
  #INSTALLS += target
}
#uncomment line below - for app build only
#QMAKE_CXXFLAGS += -fPIC

SOURCES += \
    src/emailaddress.cpp \
    src/mimeattachment.cpp \
    src/mimefile.cpp \
    src/mimehtml.cpp \
    src/mimeinlinefile.cpp \
    src/mimemessage.cpp \
    src/mimepart.cpp \
    src/mimetext.cpp \
    src/smtpclient.cpp \
    src/quotedprintable.cpp \
    src/mimemultipart.cpp \
    src/mimecontentformatter.cpp \

HEADERS  += \
    src/emailaddress.h \
    src/mimeattachment.h \
    src/mimefile.h \
    src/mimehtml.h \
    src/mimeinlinefile.h \
    src/mimemessage.h \
    src/mimepart.h \
    src/mimetext.h \
    src/smtpclient.h \
    src/SmtpMime \
    src/quotedprintable.h \
    src/mimemultipart.h \
    src/mimecontentformatter.h

#    src/smtpexports.h

OTHER_FILES += \
    LICENSE \
    README.md

FORMS +=

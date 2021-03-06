TEMPLATE = app
TARGET = ArtBoomerang-Qt
macx:TARGET = "ArtBoomerang-Qt"
windows:TARGET = "ArtBoomerang-Qt"
VERSION = 1.0.2.0
INCLUDEPATH += src src/json src/qt src/qt/plugins/mrichtexteditor  src/qt/plugins
QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
CONFIG += no_include_pwd
CONFIG += thread
STATIC=all

USE_QRCODE=1
USE_LEVELDB=1
USE_UPNP=1
USE_TESTLIB=0
#USE_MINISCREEN=1 #to detect small screen on mini-laptop

#use variables below for command-line build
#for QTCreator comment them - or creator lost .moc files
# http://doc.qt.io/qt-5/qmake-variable-reference.html
#OBJECTS_DIR = build
#MOC_DIR = build
#UI_DIR = build
#OBJECTS_DIR = build
MOC_DIR = release

android: {
    #USE_FULLSCREEN=1 #detect small screen on mobile
    DEFINES += USE_FULLSCREEN=1
    INCLUDEPATH += src/qt/android

    QT += androidextras
    QT += widgets

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    OTHER_FILES +=
    HEADERS +=
    SOURCES +=

    OBJECTS_DIR = build-android
    MOC_DIR = build-android
    UI_DIR = build-android
    USE_UPNP=-
    USE_QRCODE=1
    USE_IPV6=0
    USE_LEVELDB=1
    USE_ASM=1
    BOOST_THREAD_LIB_SUFFIX=-gcc-mt-s-1_57
    BOOST_LIB_SUFFIX=-gcc-mt-s-1_57
    BOOST_INCLUDE_PATH=C:/Android/deps/boost_1_57_0
    BOOST_LIB_PATH=C:/Android/deps/boost_1_57_0/stage/lib
    BDB_INCLUDE_PATH=C:/Android/deps/db-4.8.30.NC/build_unix
    BDB_LIB_PATH=C:/Android/deps/db-4.8.30.NC/build_unix
    OPENSSL_INCLUDE_PATH=C:/Android/deps/openssl-1.0.2a/include
    OPENSSL_LIB_PATH=C:/Android/deps/openssl-1.0.2a
    QRCODE_LIB_PATH=C:/Android/deps/qrencode-3.4.4/.libs
    QRCODE_INCLUDE_PATH=C:/Android/deps/qrencode-3.4.4

}


win32: {
  DEFINES += USE_MINISCREEN=1
  CONFIG += static
  BOOST_LIB_SUFFIX=-mgw49-mt-s-1_57
  BOOST_THREAD_LIB_SUFFIX=-mgw49-mt-s-1_57
  BOOST_INCLUDE_PATH=e:/deps/boost_1_57_0
  BOOST_LIB_PATH=e:/deps/boost_1_57_0/stage/lib

  BDB_INCLUDE_PATH=e:/deps/db-4.8.30.NC/build_unix
  BDB_LIB_PATH=e:/deps/db-4.8.30.NC/build_unix
  OPENSSL_INCLUDE_PATH=e:/deps/openssl-1.0.2h/include
  OPENSSL_LIB_PATH=e:/deps/openssl-1.0.2h
  MINIUPNPC_LIB_PATH=e:/deps/miniupnpc
  MINIUPNPC_INCLUDE_PATH=e:/deps
  QRCODE_LIB_PATH=e:/deps/qrencode-3.4.4/.libs
  QRCODE_INCLUDE_PATH=e:/deps/qrencode-3.4.4

  windows:LIBS += -lshlwapi -static -static-libgcc -static-libstdc++

}
macx:{
  USE_DBUS=1
  CONFIG += static
  BOOST_LIB_SUFFIX=-mt-s
  BOOST_INCLUDE_PATH=/usr/local/opt/boost_1_57_0
  BOOST_LIB_PATH=/usr/local/opt/boost_1_57_0/stage/lib
  OPENSSL_INCLUDE_PATH=/usr/local/opt/openssl/include
  OPENSSL_LIB_PATH=/usr/local/opt/openssl/lib
  MINIUPNPC_LIB_PATH=/usr/local/opt/miniupnpc/lib
  MINIUPNPC_INCLUDE_PATH=/usr/local/opt/miniupnpc/include
  QRCODE_LIB_PATH=/usr/local/opt/qrencode/lib
  QRCODE_INCLUDE_PATH=/usr/local/opt/qrencode/include
  
  CPPFLAGS="$CPPFLAGS -I/usr/local/opt/protobuf/include"
  LIBS+= -L/usr/local/opt/protobuf/lib
  CPPFLAGS="$CPPFLAGS -I/usr/local/opt/miniupnpc/include"
  LIBS+= -L/usr/local/opt/miniupnpc/lib
  CPPFLAGS="$CPPFLAGS -I/usr/local/opt/libpng/include"
  LIBS+= -L/usr/local/opt/libpng/lib
  CPPFLAGS="$CPPFLAGS -I/usr/local/opt/qrencode/include"
  LIBS+= -L/usr/local/opt/qrencode/lib
}

!win32:!macx:!android: {
  USE_DBUS=1
  CONFIG += static
  CPPFLAGS="$CPPFLAGS -I/usr/include"
  BOOST_LIB_SUFFIX=-mt-s
  BOOST_THREAD_LIB_SUFFIX=-mt-s
  BOOST_LIB_PATH=/usr/local/boost_1_57_0/stage/lib
  BOOST_INCLUDE_PATH=/usr/local/boost_1_57_0
  BDB_INCLUDE_PATH=/usr/local/BerkeleyDB.4.8/include
  BDB_LIB_PATH=/usr/local/BerkeleyDB.4.8/lib
  OPENSSL_INCLUDE_PATH=/usr/local/openssl/include
  OPENSSL_LIB_PATH=/usr/local/openssl
  MINIUPNPC_LIB_PATH=/usr/local/miniupnpc
  MINIUPNPC_INCLUDE_PATH=/usr/local
  QRCODE_LIB_PATH=/usr/local/qrencode-3.4.4/.libs
  QRCODE_INCLUDE_PATH=/usr/local/qrencode-3.4.4
  INCLUDEPATH += /usr/local/qrencode-3.4.4
  QMAKE_LFLAGS *= -static-libgcc 
  QMAKE_LFLAGS *= --enable-static-nss
  LIBS += -L/usr/local/Qt-5.3.2/lib
  INCLUDEPATH += /usr/local/Qt-5.3.2/include

}



# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    windows:BOOST_LIB_SUFFIX = -mt-s
}
isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    win32:BOOST_THREAD_LIB_SUFFIX = _win32$$BOOST_LIB_SUFFIX
    else:BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /opt/local/lib/db48
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -4.8
}

isEmpty(BDB_INCLUDE_PATH) {
    macx:BDB_INCLUDE_PATH = /opt/local/include/db48
}

isEmpty(BOOST_LIB_PATH) {
    macx:BOOST_LIB_PATH = /opt/local/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    macx:BOOST_INCLUDE_PATH = /opt/local/include
}

LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX

win32:LIBS += -lws2_32 -lole32 -loleaut32 -luuid -lgdi32

# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively


# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for maximum compatibility (10.5, 32-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.5 -arch x86_64 -isysroot /Developer/SDKs/MacOSX10.5.sdk

    !windows:!macx {
        # Linux: static link
        LIBS += -Wl,-Bstatic
    }
}

!win32 {
# for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
QMAKE_CXXFLAGS *= -fstack-protector-all
QMAKE_LFLAGS *= -fstack-protector-all
# We need to exclude this for Windows cross compile with MinGW 4.2.x, as it will result in a non-working executable!
# This can be enabled for Windows, when we switch to MinGW >= 4.4.x.
}
# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat
win32:QMAKE_LFLAGS *= -Wl,--large-address-aware -static 
win32:QMAKE_LFLAGS += -static-libgcc -static-libstdc++

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    LIBS += $$join(QRCODE_LIB_PATH,,-L,) -lqrencode
    INCLUDEPATH += $$QRCODE_INCLUDE_PATH
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    DEFINES += USE_UPNP=$$USE_UPNP STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
}

# use: qmake "USE_DBUS=1" or qmake "USE_DBUS=0"
linux:count(USE_DBUS, 0) {
    USE_DBUS=1
}
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

contains(BITCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += BITCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}
android: {
  INCLUDEPATH += src/leveldb-android/include src/leveldb-android/helpers
  LIBS += $$PWD/src/leveldb-android/libleveldb.a $$PWD/src/leveldb-android/libmemenv.a
}
else {
  INCLUDEPATH += src/leveldb/include src/leveldb/helpers
  LIBS += $$PWD/src/leveldb/libleveldb.a $$PWD/src/leveldb/libmemenv.a
}

contains(USE_TESTLIB, 1) {
    message(Building with testlib)
    QT += testlib
    SOURCES += src/qt/test/test_main.cpp  src/qt/test/uritests.cpp
    HEADERS += src/qt/test/uritests.h
}


SOURCES += src/txdb-leveldb.cpp


!win32 {
    # we use QMAKE_CXXFLAGS_RELEASE even without RELEASE=1 because we use RELEASE to indicate linking preferences not -O preferences
    android: {
    genleveldb.commands = cd $$PWD/src/leveldb-android && CC=$$QMAKE_CC CXX=$$QMAKE_CXX $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a
    } else {
    genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a
    }
} else {
    # make an educated guess about what the ranlib command is called
    isEmpty(QMAKE_RANLIB) {
        QMAKE_RANLIB = $$replace(QMAKE_STRIP, strip, ranlib)
    }
    LIBS += -lshlwapi
    #genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX TARGET_OS=OS_WINDOWS_CROSSCOMPILE $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libleveldb.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libmemenv.a
}
android: {
  genleveldb.target = $$PWD/src/leveldb-android/libleveldb.a
  genleveldb.depends = FORCE
  PRE_TARGETDEPS += $$PWD/src/leveldb-android/libleveldb.a
  QMAKE_EXTRA_TARGETS += genleveldb
  QMAKE_CLEAN += $$PWD/src/leveldb-android/libleveldb.a; cd $$PWD/src/leveldb-android; chmod 0755 build_detect_platform; $(MAKE) clean
} else {
  genleveldb.target = $$PWD/src/leveldb/libleveldb.a
  genleveldb.depends = FORCE
  PRE_TARGETDEPS += $$PWD/src/leveldb/libleveldb.a
  QMAKE_EXTRA_TARGETS += genleveldb
  # Gross ugly hack that depends on qmake internals, unfortunately there is no other way to do it.
  #QMAKE_CLEAN += $$PWD/src/leveldb/libleveldb.a; cd $$PWD/src/leveldb; chmod 0755 build_detect_platform; $(MAKE) clean
  #do not delete leveldb existed libs
  #QMAKE_CLEAN += $$PWD/src/leveldb/libleveldb.a $$PWD/src/leveldb/libmemenv.a $(MAKE) clean
}
# regenerate src/build.h
!android {
  !windows|contains(USE_BUILD_INFO, 1) {
      genbuild.depends = FORCE
      genbuild.commands = cd $$PWD;  chmod 0755 share/genbuild.sh; /bin/sh share/genbuild.sh $$OUT_PWD/build/build.h
      genbuild.target = $$OUT_PWD/build/build.h
      PRE_TARGETDEPS += $$OUT_PWD/build/build.h
      QMAKE_EXTRA_TARGETS += genbuild
      DEFINES += HAVE_BUILD_INFO
  }
}

contains(USE_O3, 1) {
    message(Building O3 optimization flag)
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS += -O3
    QMAKE_CFLAGS += -O3
}

#*-g++-32 {
#    message("32 platform, adding -msse2 flag")
#    QMAKE_CXXFLAGS += -msse2
#    QMAKE_CFLAGS += -msse2
#}

QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wno-format -Wno-unused-parameter -Wstack-protector

# Input
DEPENDPATH += src src/json src/qt src/qt/plugins

android: {
    HEADERS += src/android/ifaddrs.h  src/qt/plugins/tabslider/tabSlider.h
}
HEADERS +=  src/qt/plugins/actionbar/menustyle.h src/qt/plugins/actionbar/actionbar.h

HEADERS += src/qt/bitcoingui.h \
    src/qt/transactiontablemodel.h \
    src/qt/addresstablemodel.h \
    src/qt/optionsdialog.h \
    src/qt/coincontroldialog.h \
    src/qt/coincontroltreewidget.h \
    src/qt/sendcoinsdialog.h \
    src/qt/donatedialog.h \
    src/qt/addressbookpage.h \
    src/qt/signverifymessagedialog.h \
    src/qt/aboutdialog.h \
    src/qt/editaddressdialog.h \
    src/qt/bitcoinaddressvalidator.h \
    src/alert.h \
    src/addrman.h \
    src/base58.h \
    src/bignum.h \
    src/checkpoints.h \
    src/compat.h \
    src/coincontrol.h \
    src/sync.h \
    src/util.h \
    src/uint256.h \
    src/kernel.h \
    src/scrypt.h \
    src/pbkdf2.h \
    src/serialize.h \
    src/strlcpy.h \
    src/smessage.h \
    src/main.h \
    src/miner.h \
    src/net.h \
    src/key.h \
    src/db.h \
    src/txdb.h \
    src/walletdb.h \
    src/script.h \
    src/stealth.h \
    src/init.h \
    src/irc.h \
    src/mruset.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/clientmodel.h \
    src/qt/guiutil.h \
    src/qt/transactionrecord.h \
    src/qt/guiconstants.h \
    src/qt/optionsmodel.h \
    src/qt/monitoreddatamapper.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/bitcoinamountfield.h \
    src/wallet.h \
    src/keystore.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/bitcoinrpc.h \
    src/qt/overviewpage.h \
    src/qt/csvmodelwriter.h \
    src/crypter.h \
    src/qt/sendcoinsentry.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/bitcoinunits.h \
    src/qt/qvaluecombobox.h \
    src/qt/askpassphrasedialog.h \
    src/protocol.h \
    src/qt/notificator.h \
    src/qt/qtipcserver.h \
    src/allocators.h \
    src/ui_interface.h \
    src/qt/rpcconsole.h \
    src/qt/trafficgraphwidget.h \
    src/version.h \
    src/netbase.h \
    src/clientversion.h \
    src/threadsafety.h \
    src/qt/plugins/mrichtexteditor/mrichtextedit.h \
    src/qt/qvalidatedtextedit.h \
    src/qt/dialogwindowflags.h \
    src/qt/requestpaymentdialog.h \
    src/qt/plugins/smtp/smtp.h

android: {
    SOURCES += src/android/ifaddrs.c src/qt/plugins/tabslider/tabSlider.cpp
}
SOURCES += src/qt/plugins/actionbar/menustyle.cpp  src/qt/plugins/actionbar/actionbar.cpp

SOURCES += src/qt/bitcoin.cpp src/qt/bitcoingui.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/donatedialog.cpp \
    src/qt/coincontroldialog.cpp \
    src/qt/coincontroltreewidget.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/signverifymessagedialog.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/bitcoinaddressvalidator.cpp \
    src/alert.cpp \
    src/version.cpp \
    src/sync.cpp \
    src/smessage.cpp \
    src/util.cpp \
    src/netbase.cpp \
    src/key.cpp \
    src/script.cpp \
    src/main.cpp \
    src/miner.cpp \
    src/init.cpp \
    src/net.cpp \
    src/irc.cpp \
    src/checkpoints.cpp \
    src/addrman.cpp \
    src/db.cpp \
    src/walletdb.cpp \
    src/qt/clientmodel.cpp \
    src/qt/guiutil.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/bitcoinstrings.cpp \
    src/qt/bitcoinamountfield.cpp \
    src/wallet.cpp \
    src/keystore.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/bitcoinrpc.cpp \
    src/rpcdump.cpp \
    src/rpcnet.cpp \
    src/rpcmining.cpp \
    src/rpcwallet.cpp \
    src/rpcblockchain.cpp \
    src/rpcrawtransaction.cpp \
    src/rpcsmessage.cpp \
    src/qt/overviewpage.cpp \
    src/qt/csvmodelwriter.cpp \
    src/crypter.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/bitcoinunits.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/protocol.cpp \
    src/qt/notificator.cpp \
    src/qt/qtipcserver.cpp \
    src/qt/rpcconsole.cpp \
    src/qt/trafficgraphwidget.cpp \
    src/qt/qvalidatedtextedit.cpp \
    src/qt/plugins/mrichtexteditor/mrichtextedit.cpp \
    src/qt/requestpaymentdialog.cpp \
    src/qt/plugins/smtp/smtp.cpp \
    src/noui.cpp \
    src/kernel.cpp \
    src/scrypt-arm.S \
    src/scrypt-x86.S \
    src/scrypt-x86_64.S \
    src/scrypt.cpp \
    src/pbkdf2.cpp \
    src/stealth.cpp 


RESOURCES += \
    src/qt/bitcoin.qrc \
    src/qt/res/themes/qdarkstyle/style.qrc \
    src/qt/res/themes/qlightstyle/qlightstyle.qrc \
    src/qt/android/res/icons.qrc

android: {
  RESOURCES += \
      src/qt/res/themes/androidLight/style.qrc

}
FORMS += \
    src/qt/forms/coincontroldialog.ui \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/signverifymessagedialog.ui \ 
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui \
    src/qt/forms/optionsdialog.ui \
    src/qt/plugins/mrichtexteditor/mrichtextedit.ui \
    src/qt/forms/requestpaymentdialog.ui \
    src/qt/forms/donatedialog.ui

contains(USE_QRCODE, 1) {
  HEADERS += src/qt/qrcodedialog.h
  SOURCES += src/qt/qrcodedialog.cpp
  FORMS += src/qt/forms/qrcodedialog.ui
}

CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/bitcoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/bitcoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM

# "Other files" to show in Qt Creator
OTHER_FILES += \
    doc/*.rst doc/*.txt doc/README README.md res/bitcoin-qt.rc \
    src/Tumler.py


windows:DEFINES += WIN32
windows:RC_FILE = src/qt/res/bitcoin-qt.rc

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!windows:!macx {
    DEFINES += LINUX
}
android: {
    DEFINES += ANDROID
}
!win32:!macx:!android {
    LIBS += -lrt -ldl
    # _FILE_OFFSET_BITS=64 lets 32-bit fopen transparently support large files.
    DEFINES += _FILE_OFFSET_BITS=64
}

macx:HEADERS += src/qt/macdockiconhandler.h \
                src/qt/macnotificationhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm \
                          src/qt/macnotificationhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0
macx:ICON = src/qt/res/icons/artboomerang.icns
macx:QMAKE_INFO_PLIST = share/qt/Info.plist

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX
# -lgdi32 has to happen after -lcrypto (see  #681)
windows:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32
android:{
    LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread_pthread$$BOOST_THREAD_LIB_SUFFIX
}
else{
    LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX
}

windows:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX

contains(RELEASE, 1) {
    !windows:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic
    }
}
!windows:!macx:!android: {
  LIBS += -L/usr/lib/x86_64-linux-gnu
  LIBS += -lm -ljpeg -L/usr/lib  -lstdc++  -ldl
}

system($$QMAKE_LRELEASE -silent $$_PRO_FILE_)

android: {
  DISTFILES += \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat

  DISTFILES += \
    android/res/values/colors.xml \
    android/res/values/dimens.xml \
    android/res/values/styles.xml \
    android/res/values-large/dimens.xml \
    android/res/values-small/dimens.xml \
    src/qt/res/themes/androidLight/style.qss \
    android/res/values/styles.xml

}


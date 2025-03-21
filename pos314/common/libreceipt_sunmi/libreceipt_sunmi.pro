QT += core network
QT -= gui
TEMPLATE = lib
CONFIG  += c++11
CONFIG  += sdk_no_version_check
DEFINES += _CYLOG
#DEFINES += _CYSSH
DEFINES += _CYHMAC
DEFINES += _CYMYSQL
DEFINES += _CYMEMCURL
DEFINES += _CYSTORESERVER
DEFINES += _POSIX_THREAD_SAFE_FUNCTIONS
DEFINES += QT_DEPRECATED_WARNINGS
#QMAKE_CXXFLAGS += "-Wno-deprecated-copy"
QMAKE_CXXFLAGS += "-Wno-unused-parameter"
QMAKE_CXXFLAGS += "-Wno-deprecated-declarations"

INCLUDEPATH += /Users/cyware/brewery
INCLUDEPATH += /Users/cyware/brewery/library
INCLUDEPATH += /Users/cyware/brewery/library/include
INCLUDEPATH += /Users/cyware/brewery/library/include/restsvc
INCLUDEPATH += /Users/cyware/brewery/library/include/restsvc/storesales

unix:!mac {
    QMAKE_CXXFLAGS += "-Wno-stringop-truncation"

    INCLUDEPATH += /usr/include/mysql
    INCLUDEPATH += /usr/include/postgresql

    LIBS += -ldl -lpthread -lssh -lssl -lcrypto -lmysqlclient
} mac {
    INCLUDEPATH += /opt/homebrew/opt/libssh/include
    INCLUDEPATH += /opt/homebrew/opt/openssl/include
    INCLUDEPATH += /opt/homebrew/opt/mysql-client/include/mysql

    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/liblegacy.dylib
} windows  {
    LIBS += /Users/cyware/brewery/liblegacy.a
    LIBS += /Users/cyware/brewery/libssl.lib
    LIBS += /Users/cyware/brewery/libcrypto.lib
}

TARGET  = receiptsunmi
OBJECTS_DIR = /tmp/receipt_sunmi/

SOURCES += \
    cyreceiptsunmi.cpp  \
    cyreceiptsunmi_account.cpp \
    cyreceiptsunmi_detail.cpp \
    cyreceiptsunmi_footer.cpp \
    cyreceiptsunmi_header.cpp \
    cyreceiptsunmi_payment.cpp \
    cyreceiptsunmi_signature.cpp \
    cyreceiptsunmi_totals.cpp \
    cyreceiptsunmi_vat.cpp

HEADERS += \
    /Users/cyware/brewery/library/include/cgic.h \
    /Users/cyware/brewery/library/include/col/column.h \
    /Users/cyware/brewery/library/include/col/column_array.h \
    /Users/cyware/brewery/library/include/col/column_array_base.h \
    /Users/cyware/brewery/library/include/col/column_enum.h \
    /Users/cyware/brewery/library/include/cydb/cydbsql.h \
    /Users/cyware/brewery/library/include/cydb/cysqlite.h \
    /Users/cyware/brewery/library/include/cydb/cyw_db.h \
    /Users/cyware/brewery/library/include/cydb/cymysql.h \
    /Users/cyware/brewery/library/include/cyw_aes.h \
    /Users/cyware/brewery/library/include/cyw_cgi.h \
    /Users/cyware/brewery/library/include/cyw_colbind.h \
    /Users/cyware/brewery/library/include/cyw_form.h \
    /Users/cyware/brewery/library/include/cyw_html.h \
    /Users/cyware/brewery/library/include/cyw_ini.h \
    /Users/cyware/brewery/library/include/cyw_log.h \
    /Users/cyware/brewery/library/include/cyw_mem.h \
    /Users/cyware/brewery/library/include/cyw_restglobal.h \
    /Users/cyware/brewery/library/include/cyw_util.h \
    /Users/cyware/brewery/library/include/cyw_util_sha.h \
    /Users/cyware/brewery/library/include/db/sqlite3.h \
    /Users/cyware/brewery/library/include/db/sqlite3ext.h \
    /Users/cyware/brewery/library/include/json/json-forwards.h \
    /Users/cyware/brewery/library/include/json/json.h \
    /Users/cyware/brewery/library/include/sha256/WjCryptLib_Sha256.h \
    /Users/cyware/brewery/library/include/sha256/hmac_sha256.h \
    /Users/cyware/brewery/library/include/sha256/sha1.h \
    /Users/cyware/brewery/library/include/osxport/SerialPort.h \
    /Users/cyware/brewery/library/include/restsvc/storesales/cyrestcommon.h \
    /Users/cyware/brewery/library/include/restsvc/storesales/cyrestreceipt.h \
    /Users/cyware/brewery/library/include/restsvc/storesales/cyresteod.h \
    /Users/cyware/brewery/library/include/restsvc/storesales/cyreceiptsunmi.h




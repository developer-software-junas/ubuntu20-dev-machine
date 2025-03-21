TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  -= qt
CONFIG  += console
CONFIG  += c++11
CONFIG  += sdk_no_version_check
DEFINES += _CYLOG
DEFINES += _CYSSH
DEFINES += _CYHMAC
DEFINES += _CYMYSQL
DEFINES += _APPLE_
DEFINES += _CYMEMCURL
DEFINES += _CYSTORESERVER
DEFINES += OPENSSL_API_3_0
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CFLAGS += "-Wno-deprecated-copy"
QMAKE_CFLAGS += "-Wno-deprecated-declarations"
QMAKE_CXXFLAGS += "-Wno-deprecated-copy"
QMAKE_CXXFLAGS += "-Wno-unused-result"
QMAKE_CXXFLAGS += "-Wno-deprecated-declarations"

INCLUDEPATH += /Users/cyware/Qt/6.4.0/macos/include
INCLUDEPATH += /Users/cyware/brewery/library
INCLUDEPATH += /Users/cyware/brewery/library/include
INCLUDEPATH += /Users/cyware/brewery/library/include/restsvc
INCLUDEPATH += /Users/cyware/brewery/library/include/restsvc/storesales
unix:!mac {
    QMAKE_CXXFLAGS += "-Wno-stringop-truncation"

    INCLUDEPATH += /usr/include/mysql
    INCLUDEPATH += /usr/include/postgresql

    LIBS += -ldl -llegacy -lreceipt -lreceiptrcc -lreceiptjoel -lreceiptelena
} mac {


    INCLUDEPATH += /opt/homebrew/opt/libssh/include
    INCLUDEPATH += /opt/homebrew/opt/openssl/include
    INCLUDEPATH += /opt/homebrew/opt/mysql-client/include/mysql

    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/liblegacy.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceipt.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptrcc.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptjoel.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptelena.dylib
}

TARGET  = mysqlpublish.cgi
OBJECTS_DIR = /tmp/mysqlpublish

SOURCES +=  \
    cycmd.cpp \
    cypublish.cpp \
    cypublish_masterfile.cpp \
    cypublish_salesimport.cpp \
    cypublish_sqlite.cpp \
    cypublish_utility.cpp \
    cytable.cpp \
    main.cpp

HEADERS += \
    /Users/cyware/storm/library/include/shopifyapp/cyw_global_app.h \
    /Users/cyware/storm/library/include/sha256/sha1.h \
    /Users/cyware/storm/library/include/sha256/hmac_sha256.h \
    /Users/cyware/storm/library/include/sha256/WjCryptLib_Sha256.h \
    /Users/cyware/storm/library/include/col/column.h \
    /Users/cyware/storm/library/include/col/column_array.h \
    /Users/cyware/storm/library/include/col/column_enum.h \
    /Users/cyware/storm/library/include/col/column_utility.h \
    /Users/cyware/storm/library/include/cyw_aes.h \
    /Users/cyware/storm/library/include/cyw_cgi.h \
    /Users/cyware/storm/library/include/cyw_colbind.h \
    /Users/cyware/storm/library/include/cyw_db.h \
    /Users/cyware/storm/library/include/cyw_form.h \
    /Users/cyware/storm/library/include/cyw_html.h \
    /Users/cyware/storm/library/include/cyw_ini.h \
    /Users/cyware/storm/library/include/cyw_js.h \
    /Users/cyware/storm/library/include/cyw_js_web.h \
    /Users/cyware/storm/library/include/cyw_log.h \
    /Users/cyware/storm/library/include/cyw_mem.h \
    /Users/cyware/storm/library/include/cyw_qrencode.h \
    /Users/cyware/storm/library/include/cyw_schema_setup.h \
    /Users/cyware/storm/library/include/cyw_util.h \
    /Users/cyware/storm/library/include/cyw_util_sha.h \
    /Users/cyware/storm/library/include/db/cydbsql.h \
    /Users/cyware/storm/library/include/db/cymysql.h \
    /Users/cyware/storm/library/include/db/cysqlite.h \
    /Users/cyware/storm/library/include/json/json.h \
    /Users/cyware/storm/library/include/logic/cyw_branch.h \
    /Users/cyware/storm/library/include/logic/cyw_currency.h \
    /Users/cyware/storm/library/include/logic/cyw_customer.h \
    /Users/cyware/storm/library/include/logic/cyw_movement.h \
    /Users/cyware/storm/library/include/logic/cyw_order.h \
    /Users/cyware/storm/library/include/logic/cyw_po.h \
    /Users/cyware/storm/library/include/logic/cyw_product.h \
    /Users/cyware/storm/library/include/logic/cyw_transform.h \
    /Users/cyware/storm/library/include/logic/cyw_uom.h \
    /Users/cyware/storm/library/include/logic/cyw_user.h \
    /Users/cyware/storm/library/include/logic/cyw_vendor.h \
    /Users/cyware/storm/library/include/logic/cyw_user.h \
    /Users/cyware/storm/library/include/shopify/cyw_shopify.h \
    /Users/cyware/storm/library/include/brewery_json.h \
    /Users/cyware/storm/library/include/brewery_rest_argument.h  \
    cycmd.h \
    cypublish.h \
    cytable.h

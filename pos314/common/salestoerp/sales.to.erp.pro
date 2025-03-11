QT += core network gui
#QT -= gui
TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  += console
CONFIG  += c++11
CONFIG  += sdk_no_version_check
DEFINES += _CYLOG
DEFINES += _CYSSH
DEFINES += _CYHMAC
DEFINES += _CYMYSQL
DEFINES += _CYMEMCURL
DEFINES += _CYSTORESERVER
DEFINES += OPENSSL_API_3_0
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CFLAGS += "-Wno-deprecated-declarations"
QMAKE_CXXFLAGS += "-Wno-deprecated-copy"
QMAKE_CXXFLAGS += "-Wno-unused-result"
QMAKE_CXXFLAGS += "-Wno-unused-parameter"
QMAKE_CXXFLAGS += "-Wno-stringop-truncation"
QMAKE_CXXFLAGS += "-Wno-deprecated-declarations"


INCLUDEPATH += /Users/pos314/include
INCLUDEPATH += /Users/pos314/include/json
INCLUDEPATH += /Users/pos314/include/restsvc
INCLUDEPATH += /Users/pos314/common/endofday
INCLUDEPATH += /Users/pos314/common/mysqlpublish
INCLUDEPATH += /Users/pos314/include/restlogic
INCLUDEPATH += /Users/pos314/include/restsvc/erpsales
INCLUDEPATH += /Users/pos314/include/restsvc/storesales

unix:!mac {
    QMAKE_CXXFLAGS += "-Wno-stringop-truncation"

    INCLUDEPATH += /usr/include/mysql
    INCLUDEPATH += /usr/include/postgresql

    LIBS += -ldl -lpthread -lssh -lssl -lcrypto -lmysqlclient -llegacy
} mac {


    INCLUDEPATH += /opt/homebrew/opt/libssh/include
    INCLUDEPATH += /opt/homebrew/opt/libressl/include
    INCLUDEPATH += /opt/homebrew/opt/mysql-client/include/mysql

    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/liblegacy.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceipt.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptrcc.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptjoel.dylib
    LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptelena.dylib
    #LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceipt.dylib
    #LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptrcc.dylib
    #LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptjoel.dylib
    #LIBS += /Library/RCC-POS/1.0.0/web/cgi-bin/libreceiptelena.dylib
}

TARGET  = salestoerp.cgi
OBJECTS_DIR = /tmp/sales2erp
UI_DIR = /tmp/sales2erp
MOC_DIR = /tmp/sales2erp


SOURCES += \
    /Users/pos314/common/mysqlpublish/cytable.cpp \
    /Users/pos314/common/liblegacy/cyw_mem_curl.cpp \
    /Users/pos314/common/restpos/restlogic/salesfilter.cpp \
    /Users/pos314/common/restpos/restlogic/salesprocess.cpp \
    /Users/pos314/common/restpos/restlogic/tablemigrate.cpp \
    /Users/pos314/common/restpos/restlogic/cysaleshost.cpp \
    /Users/pos314/common/restpos/restlogic/remote_suspend.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_xread.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_utility.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_retrieve.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_payment.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_headeraccount.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_header.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_detail.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/detail_calculate_regular.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/detail_calculate_senior.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/detail_calculate_zerorated.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_fspurl.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_fast.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_cancel.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_json.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_refund.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_sales.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_suspend.cpp \
    /Users/pos314/common/restpos/restsvc/storesales/cyrestsales_receipt_eod_void.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_batch.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_creditlimit.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_detail.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_export.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_header.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_payment.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_detail_senior_tax.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_detail_svccharge.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_detail_tax.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_item_senior.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_item_regular.cpp \
    /Users/pos314/common/liblegacy/restsvc/erpsales/erpsales_item_diplomat.cpp \
    cgimain.cpp


HEADERS += \
    ../../include/restsvc/erpsales/erpsales.h \
    /Users/pos314/include/cgic.h \
    /Users/pos314/include/col/column.h \
    /Users/pos314/include/col/column_array.h \
    /Users/pos314/include/col/column_array_base.h \
    /Users/pos314/include/col/column_enum.h \
    /Users/pos314/include/cydb/cydbsql.h \
    /Users/pos314/include/cydb/cysqlite.h \
    /Users/pos314/include/cydb/cyw_db.h \
    /Users/pos314/include/cydb/cymysql.h \
    /Users/pos314/include/cyw_aes.h \
    /Users/pos314/include/cyw_cgi.h \
    /Users/pos314/include/cyw_colbind.h \
    /Users/pos314/include/cyw_form.h \
    /Users/pos314/include/cyw_html.h \
    /Users/pos314/include/cyw_ini.h \
    /Users/pos314/include/cyw_log.h \
    /Users/pos314/include/cyw_mem.h \
    /Users/pos314/include/cyw_util.h \
    /Users/pos314/include/cyw_util_sha.h \
    /Users/pos314/include/db/sqlite3.h \
    /Users/pos314/include/db/sqlite3ext.h \
    /Users/pos314/include/json/json-forwards.h \
    /Users/pos314/include/json/json.h \
    /Users/pos314/include/sha256/WjCryptLib_Sha256.h \
    /Users/pos314/include/sha256/hmac_sha256.h \
    /Users/pos314/include/sha256/sha1.h \
    /Users/pos314/common/mysqlpublish/cytable.h \
    /Users/pos314/include/restsvc/storesales/cyrestcommon.h \
    /Users/pos314/include/restsvc/storesales/cyrestreceipt.h \
    /Users/pos314/include/restlogic/cysaleshost.h

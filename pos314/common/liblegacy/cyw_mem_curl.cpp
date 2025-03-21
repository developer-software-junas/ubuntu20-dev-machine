/*******************************************************************
          FILE:         cyw_mem.cpp
   DESCRIPTION:         Memory routines
 *******************************************************************/
//////////////////////////////////////////
//  Header file(s)
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#ifdef _CYMEMCURL
#include "cyw_mem.h"
#include <string>

#include <QUrl>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
/*******************************************************************
      FUNCTION:        cymem_https_get
   DESCRIPTION:        Get the http content
     ARGUMENTS:        szUrl - the site to access
                       szResult - the content
       RETURNS:        true on success
 *******************************************************************/
bool                    cymem_https_get (int port,
                                         int timeout,
                                         const char* host,
                                         const char* args,
                                         struct_cymem* mem)
{
    std::string url;
    (void)timeout;
    cymem_reset(mem);
    if (port == 443)
        url = "https://";
    else
        url = "http://";
    url += host;
    //url += "?";
    url += args;

    QEventLoop synchronous;
    QNetworkReply* reply;
    QNetworkRequest request;
    QNetworkAccessManager *networkManager = new QNetworkAccessManager();
    QObject::connect(networkManager, SIGNAL(finished(QNetworkReply*)), &synchronous, SLOT(quit()));

    request.setUrl(QUrl(url.c_str ()));
    reply = networkManager->get(request);
    synchronous.exec();
    reply->deleteLater();
    QByteArray qba = reply->readAll();

    url = qba.toStdString();
    cymem_append (mem, url.c_str ());
    return true;
}
/*******************************************************************
      FUNCTION:        cymem_https_post
   DESCRIPTION:        Get the http content
     ARGUMENTS:        szUrl - the site to access
                       szResult - the content
       RETURNS:        true on success
 *******************************************************************/
bool                    cymem_https_post(int port,
                                         int timeout,
                                         const char* host,
                                         const char* args,
                                         struct_cymem* mem)
{
    (void)port;
    (void)timeout;

    cymem_reset(mem);
    std::string url = host;
    //"\"Access-Control-Allow-Methods\": \"GET,PUT,PATCH,POST,DELETE\", "
    //"\"Access-Control-Allow-Headers\": \"Origin, X-Requested-With, Content-Type, Accept\", "
    const char hdr [] = "\"Access-Control-Allow-Origin\": \"*\", "
                        "'Content-Type': 'text/html', "
                        "'Accept': '*/*' ";

    QEventLoop synchronous;
    QNetworkReply* reply;
    QNetworkRequest request;
    QNetworkAccessManager *networkManager = new QNetworkAccessManager();
    request.setHeader(QNetworkRequest::ContentTypeHeader,hdr);
    QObject::connect(networkManager, SIGNAL(finished(QNetworkReply*)), &synchronous, SLOT(quit()));

    request.setUrl(QUrl(url.c_str ()));
    QByteArray postData = args;
    reply = networkManager->post(request,postData);

    synchronous.exec();    
    reply->deleteLater();
    QByteArray qba = reply->readAll();

    url = qba.toStdString();
    cymem_append (mem, url.c_str ());
    return true;
}
/*******************************************************************
      FUNCTION:        cymem_http_file
   DESCRIPTION:        Get the http content and save to a file
     ARGUMENTS:        szUrl - the site to access
                       szResult - the content
       RETURNS:        true on success
 *******************************************************************/
bool                    cymem_http_file (int port,
                                         const char* host,
                                         const char* args,
                                         const char* file,
                                         struct_cymem* mem)
{
    (void)port; (void)host; (void)args; (void)file; (void)mem;
    return true;
/*
    cymem_reset(mem);
    //////////////////////////////////////
    //  Initialize the library
    mg_init_library(MG_FEATURES_TLS);
    if (mg_check_feature(MG_FEATURES_TLS) != MG_FEATURES_TLS) {
        cymem_append(mem, "TLS is not available\n");
        return false;
    }
    //////////////////////////////////////
    //  Client connection
    char errbuf[256] = {0};
    struct mg_client_options opt = {nullptr,0,nullptr,nullptr,nullptr};
    opt.host = host;           // Host name from command line
    opt.port = port;            // Default HTTPS port
    opt.client_cert = nullptr; // Client certificate, if required
    opt.server_cert = nullptr; // Server certificate to verify
    opt.host_name = NULL;//opt.host;  // Host name for SNI
    struct mg_connection *cli = mg_connect_client (host,port,0,errbuf, sizeof(errbuf));
    //struct mg_connection *cli = mg_connect_client_secure(&opt, errbuf, sizeof(errbuf));
    //////////////////////////////////////
    //  Check return value
    if (nullptr == cli) {
        cymem_append(mem, "Cannot connect client: ");
        cymem_append(mem, errbuf);
        return false;
    }
    //////////////////////////////////////
    //  Append the HTTP argument(s)
    mg_printf(cli, "GET %s HTTP/1.1\r\n", args);
    mg_printf(cli, "Host: %s\r\n", opt.host);
    mg_printf(cli, "Connection: close\r\n\r\n");
    //////////////////////////////////////
    //  Do it...
    int ret = mg_get_response(cli, errbuf, sizeof(errbuf), 10000);
    if (ret < 0) {
        cymem_append(mem, "Get request failed: ");
        cymem_append(mem, errbuf);
        mg_close_connection(cli);
        return false;
    }
    const struct mg_response_info *ri = mg_get_response_info(cli);
    if (ri == NULL) {
        cymem_append(mem, "Invalid response information: ");
        cymem_append(mem, errbuf);
        mg_close_connection(cli);
        return false;
    }
    //////////////////////////////////////
    //  Content length
    size_t len = 0;
    char buf [2048];
    std::string tmp;
    bool bexit = false;
    FILE* f = fopen (file,"w");
    if (nullptr == f)  {
        cymem_append(mem, "Unable to open the output file: ");
        cymem_append(mem, file);
        mg_close_connection(cli);
        return false;
    }
    while (!bexit)  {
        memset (buf, 0x00, 2048);
        len = mg_read(cli,buf,2000);
        if (len < 1)
            bexit = true;
        else
            fwrite (buf,1,len,f);
    }
    fclose (f);
    mg_exit_library();
    mg_close_connection(cli);
    return true;
*/
}
#endif

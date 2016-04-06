/*
   Copyright 2016 Wang Haomiao<et.tw@163.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */
#include <QObject>
#include <QTextStream>
#include <QEventLoop>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "SeimiServerHandler.h"
#include "SeimiWebPage.h"
#include "pillowcore/HttpServer.h"
#include "pillowcore/HttpHandler.h"
#include "pillowcore/HttpConnection.h"

SeimiServerHandler::SeimiServerHandler(QObject *parent):Pillow::HttpHandler(parent),
    renderTimeP("renderTime"),
    urlP("url"),
    proxyP("proxy"),
    scriptP("script"),
    useCookieP("useCookie"),
    postParamP("postParam")
{

}

bool SeimiServerHandler::handleRequest(Pillow::HttpConnection *connection){
    QEventLoop eventLoop;
    SeimiPage *seimiPage=new SeimiPage(this);
    QTextStream out(stdout);
    QString url = connection->requestParamValue(urlP);
    int renderTime = connection->requestParamValue(renderTimeP).toInt();
    QString proxyStr = connection->requestParamValue(proxyP);
    if(!proxyStr.isEmpty()){
        QRegularExpression re("(?<protocol>http|https|socket)://(?:(?<user>\\w*):(?<password>\\w*)@)?(?<host>[\\w.]+)(:(?<port>\\d+))?");
        QRegularExpressionMatch match = re.match(proxyStr);
        if(match.hasMatch()){
            QNetworkProxy proxy;
            if(match.captured("protocol") == "socket"){
                proxy.setType(QNetworkProxy::Socks5Proxy);
            }else{
                proxy.setType(QNetworkProxy::HttpProxy);
            }
            proxy.setHostName(match.captured("host"));
            proxy.setPort(match.captured("port").toInt()==0?80:match.captured("port").toInt());
            proxy.setUser(match.captured("user"));
            proxy.setPassword(match.captured("password"));

            seimiPage->setProxy(proxy);
        }else {
            out<< "[SEIMI] proxy pattern error, proxy = "<< proxyStr << endl;
        }
    }
    QString jscript = connection->requestParamValue(scriptP);
    QString postParamJson = connection->requestParamValue(postParamP);
    seimiPage->setScript(jscript);
    seimiPage->setPostParam(postParamJson);
    out << "[seimi] targetUrl:" << url<< " renderTime(ms):"<< renderTime <<endl;
    int useCookieFlag = connection->requestParamValue(useCookieP).toInt();
    seimiPage->setUseCookie(useCookieFlag==1);
    QObject::connect(seimiPage,SIGNAL(loadOver()),&eventLoop,SLOT(quit()));
    seimiPage->toLoad(url,renderTime);
    eventLoop.exec();
    connection->writeResponse(200, Pillow::HttpHeaderCollection(),seimiPage->getContent().toUtf8());
    seimiPage->deleteLater();
    return true;
}


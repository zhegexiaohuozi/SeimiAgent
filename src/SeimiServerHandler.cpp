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
#include <QEventLoop>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QBuffer>
#include "SeimiServerHandler.h"
#include "SeimiWebPage.h"


SeimiServerHandler::SeimiServerHandler(QObject *parent):HttpRequestHandler(parent),
    renderTimeP("renderTime"),
    urlP("url"),
    proxyP("proxy"),
    scriptP("script"),
    useCookieP("useCookie"),
    postParamP("postParam"),
    contentTypeP("contentType"),
    outImgSizeP("outImgSize"),
    uaP("ua"),
    resourceTimeoutP("resourceTimeout")
{

}

void SeimiServerHandler::service(HttpRequest& request, HttpResponse& response){
    QByteArray method = request.getMethod();
    QByteArray path = request.getPath();
    if(method == "GET"){
        response.setStatus(405,"Method 'GET' is not supprot,please use 'POST'");
        response.write("Method 'GET' is not supprot,please use 'POST'");
        return;
    }
    if(path != "/doload"){
        return;
    }
    QString url = request.getParameter(urlP.toUtf8());
    int renderTime = request.getParameter(renderTimeP.toUtf8()).toInt();
    QString proxyStr = request.getParameter(proxyP.toUtf8());
    QString contentType = request.getParameter(contentTypeP.toUtf8());
    QString outImgSizeStr = request.getParameter(outImgSizeP.toUtf8());
    QString ua = request.getParameter(uaP.toUtf8());
    QString jscript = request.getParameter(scriptP.toUtf8());
    QString postParamJson = request.getParameter(postParamP.toUtf8());
    int resourceTimeout = request.getParameter(resourceTimeoutP.toUtf8()).toInt();
    response.setHeader("Pragma", "no-cache");
    response.setHeader("Expires", "-1");
    response.setHeader("Cache-Control", "no-cache");
    try{
        QEventLoop eventLoop;
        SeimiPage *seimiPage=new SeimiPage(this);
        if(!proxyStr.isEmpty()){
            QRegularExpression reProxy("(?<protocol>http|https|socket)://(?:(?<user>\\w*):(?<password>\\w*)@)?(?<host>[\\w.]+)(:(?<port>\\d+))?");
            QRegularExpressionMatch matchProxy = reProxy.match(proxyStr);
            if(matchProxy.hasMatch()){
                QNetworkProxy proxy;
                if(matchProxy.captured("protocol") == "socket"){
                    proxy.setType(QNetworkProxy::Socks5Proxy);
                }else{
                    proxy.setType(QNetworkProxy::HttpProxy);
                }
                proxy.setHostName(matchProxy.captured("host"));
                proxy.setPort(matchProxy.captured("port").toInt()==0?80:matchProxy.captured("port").toInt());
                proxy.setUser(matchProxy.captured("user"));
                proxy.setPassword(matchProxy.captured("password"));

                seimiPage->setProxy(proxy);
            }else {
                qWarning("[seimi] proxy pattern error, proxy = %s",proxyStr.toUtf8().constData());
            }
        }
        seimiPage->setScript(jscript);
        seimiPage->setPostParam(postParamJson);
        qInfo("[seimi] TargetUrl:%s ,RenderTime(ms):%d",url.toUtf8().constData(),renderTime);
        int useCookieFlag = request.getParameter(useCookieP.toUtf8()).toInt();
        seimiPage->setUseCookie(useCookieFlag==1);
        QObject::connect(seimiPage,SIGNAL(loadOver()),&eventLoop,SLOT(quit()));
        seimiPage->toLoad(url,renderTime,ua,resourceTimeout);
        eventLoop.exec();

        if(contentType == "pdf"){
            response.setHeader("Content-Type", "application/pdf");
            QByteArray pdfContent = seimiPage->generatePdf();
            QCryptographicHash md5sum(QCryptographicHash::Md5);
            md5sum.addData(pdfContent);
            QByteArray etag = md5sum.result().toHex();
            response.setHeader("ETag", etag);
            response.write(pdfContent);
        }else if(contentType == "img"){
            response.setHeader("Content-Type", "image/png");
            QSize targetSize;
            if(!outImgSizeStr.isEmpty()){
                QRegularExpression reImgSize("(?<xSize>\\d+)(?:x|X)(?<ySize>\\d+)");
                QRegularExpressionMatch matchImgSize = reImgSize.match(outImgSizeStr);
                if(matchImgSize.hasMatch()){
                    targetSize.setWidth(matchImgSize.captured("xSize").toInt());
                    targetSize.setHeight(matchImgSize.captured("ySize").toInt());
                }
            }
            QByteArray imgContent = seimiPage->generateImg(targetSize);
            QCryptographicHash md5sum(QCryptographicHash::Md5);
            md5sum.addData(imgContent);
            QByteArray etag = md5sum.result().toHex();
            response.setHeader("ETag", etag);
            response.write(imgContent);
        }else{
            response.setHeader("Content-Type", "text/html;charset=utf-8");
            QString defBody = "<html>null</html>";
            response.write(seimiPage->getContent().isEmpty()?defBody.toUtf8():seimiPage->getContent().toUtf8());
        }
        seimiPage->deleteLater();
    }catch (std::exception& e) {
        response.setHeader("Content-Type", "text/html;charset=utf-8");
        QString errMsg = "<html>server error,please try again.</html>";
        qInfo("[seimi error] Page error, url: %s, errorMsg: %s", url.toUtf8().constData(), QString(QLatin1String(e.what())).toUtf8().constData());
        response.setStatus(500,"server error");
        response.write(errMsg.toUtf8());
    }catch (...) {
        qInfo() << "server error!";
        response.setHeader("Content-Type", "text/html;charset=utf-8");
        QString errMsg = "<html>server error,please try again.</html>";
        response.setStatus(500,"server error");
        response.write(errMsg.toUtf8());
    }
}


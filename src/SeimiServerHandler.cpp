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
#include "SeimiAgent.h"


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
    connect(this,SIGNAL(reciveReq(HttpRequest&,HttpResponse&)),SeimiAgent::instance(),SLOT(requestProcess(HttpRequest&,HttpResponse&)));
}

void SeimiServerHandler::service(HttpRequest& request, HttpResponse& response){
    emit reciveReq(request,response);
}


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
#ifndef SEIMISERVERHANDLER_H
#define SEIMISERVERHANDLER_H
#include "pillowcore/HttpHandler.h"
#include "pillowcore/HttpConnection.h"
#include "SeimiWebPage.h"

class SeimiServerHandler : public Pillow::HttpHandler
{
public:
    SeimiServerHandler(QObject* parent = 0);
    bool handleRequest(Pillow::HttpConnection *connection);
private:
    QString renderTimeP;
    QString urlP;
    QString proxyP;
    QString scriptP;
    QString useCookieP;
    QString postParamP;
    QString contentTypeP;
    QString outImgSizeP;
    QString uaP;
    QString resourceTimeoutP;
};

#endif // SEIMISERVERHANDLER_H

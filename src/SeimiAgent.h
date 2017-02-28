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
#ifndef SEIMIAGENT_H
#define SEIMIAGENT_H
#include <QObject>
#include <QEventLoop>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QBuffer>
#include <cookiejar.h>
#include "httplistener.h"
#include "httprequest.h"
#include "httpresponse.h"

using namespace stefanfrings;

class SeimiAgent: public QObject
{
    Q_OBJECT
private:
    SeimiAgent(QObject *parent = 0);    
public slots:
    void requestProcess(HttpRequest& request, HttpResponse& response);
public:
    static SeimiAgent* instance();
    int run(int argc, char *argv[]);


};

#endif // SEIMIAGENT_H

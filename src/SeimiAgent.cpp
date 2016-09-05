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
#include <QApplication>
#include <QCommandLineParser>
#include "pillowcore/HttpServer.h"
#include "pillowcore/HttpHandler.h"
#include "pillowcore/HttpConnection.h"
#include "SeimiAgent.h"
#include "SeimiWebPage.h"
#include "SeimiServerHandler.h"

static SeimiAgent* seimiAgentInstance = NULL;

SeimiAgent::SeimiAgent(QObject *parent) : QObject(parent)
{

}

SeimiAgent* SeimiAgent::instance(){
    if(NULL == seimiAgentInstance){
        seimiAgentInstance = new SeimiAgent();
    }
    return seimiAgentInstance;
}


int SeimiAgent::run(int argc, char *argv[]){
    QApplication a(argc, argv);
    a.setApplicationVersion("1.3.1");
    a.setApplicationName("SeimiAgent");
    a.setApplicationDisplayName("A headless,standalone webkit server which make grabing dynamic web page easier.");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption p(QStringList() << "p" << "port", "The port for seimiAgent to listening,default:8000.", "8000");

    parser.addOption(p);
    parser.process(a);

    int portN = parser.value("p").toInt();
    if (portN == 0){
        portN = 8000;
    }
    Pillow::HttpServer server(QHostAddress("0.0.0.0"), portN);
    if (!server.isListening())
        exit(1);
    qInfo() << "[seimi] SeimiAgent started,listening on :"<<portN;
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    Pillow::HttpHandler* handler = new Pillow::HttpHandlerStack(&server);
        new Pillow::HttpHandlerLog(handler);
        new SeimiServerHandler(handler);
        new Pillow::HttpHandler404(handler);
    QObject::connect(&server, SIGNAL(requestReady(Pillow::HttpConnection*)), handler, SLOT(handleRequest(Pillow::HttpConnection*)));
    return a.exec();
}

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
#include "SeimiAgent.h"
#include "SeimiWebPage.h"
#include "SeimiServerHandler.h"

using namespace stefanfrings;

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
    QSettings* settings = new QSettings("seimiagent.org","SeimiAgent");
    settings->setValue("port",portN);
    settings->setValue("minThreads",5);
    settings->setValue("maxThreads",100);
    settings->setValue("cleanupInterval",60000);
    settings->setValue("readTimeout",60000);
    settings->setValue("maxRequestSize",10240000);
    settings->setValue("maxMultiPartSize",10000000);

    qInfo() << "[seimi] SeimiAgent started";
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    new HttpListener(settings,new SeimiServerHandler(&a),&a);
    return a.exec();
}

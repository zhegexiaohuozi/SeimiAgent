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
#include "SeimiWebPage.h"

#include <QTimer>
#include <QUrl>
#include <QString>
#include <QNetworkProxy>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include "NetworkAccessManager.h"
#include "SeimiAgent.h"

SeimiPage::SeimiPage(QObject *parent) : QObject(parent)
{
    _sWebPage = new QWebPage(this);
    _isContentSet = false;
    _isProxyHasBeenSet = false;
    _renderTime = 0;
    _useCookie = false;

    connect(_sWebPage,SIGNAL(loadFinished(bool)),SLOT(loadAllFinished(bool)));
    connect(_sWebPage,SIGNAL(loadProgress(int)),SLOT(processLog(int)));
}

void SeimiPage::loadAllFinished(bool){
    qInfo("[Seimi] All load finished.");
    QTimer::singleShot(_renderTime,this,SLOT(renderFinalHtml()));
}

void SeimiPage::renderFinalHtml(){
    if(!_script.isEmpty()){
        QVariant res = _sWebPage->mainFrame()->evaluateJavaScript(_script);
        qInfo("[Seimi] EvaluateJavaScript done. result=%s , js=%s",res.toString().toUtf8().constData(),_script.toUtf8().constData());
    }
    qInfo("[Seimi] Document render out over.");
    _content = _sWebPage->mainFrame()->toHtml();
    _isContentSet = true;
    emit loadOver();
}

QString SeimiPage::getContent(){
    return _content;
}

bool SeimiPage::isOver(){
    return _isContentSet;
}

void SeimiPage::setProxy(QNetworkProxy &proxy){
    _proxy = proxy;
    _isProxyHasBeenSet = true;
}

void SeimiPage::setScript(QString &script){
    _script = script;
}

bool SeimiPage::isProxySet(){
    return _isProxyHasBeenSet;
}

void SeimiPage::processLog(int p){
    qInfo("[Seimi] TargetUrl[%s] process:%d%",_url.toUtf8().constData(),p);
}

void SeimiPage::toLoad(const QString &url,int renderTime){
    this->_url = url;
    this->_renderTime = renderTime;
    NetworkAccessManager *m_networkAccessManager = new NetworkAccessManager(this);
    m_networkAccessManager->setCurrentUrl(url);
    if(isProxySet()){
        m_networkAccessManager->setProxy(_proxy);
    }
    if(_useCookie){
        m_networkAccessManager->setCookieJar(SeimiAgent::instance()->getCookieJar());
    }
    _sWebPage->setNetworkAccessManager(m_networkAccessManager);
    if(_postParamStr.isEmpty()){
        _sWebPage->mainFrame()->load(QUrl(url));
    }else{
        QJsonParseError parseErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(_postParamStr.toUtf8(),&parseErr);
        if(parseErr.error == QJsonParseError::NoError&&!(jsonDoc.isNull()||jsonDoc.isEmpty())&&jsonDoc.isObject()){
            QJsonObject obj = jsonDoc.object();
            QByteArray targetParams;
            foreach (QString k, obj.keys()) {
                targetParams.append(k+"="+obj.value(k).toString()+"&");
            }
            _sWebPage->mainFrame()->load(QNetworkRequest(QUrl(url)),QNetworkAccessManager::PostOperation,targetParams);

        }else{
            qWarning("[Seimi] postParam is invalid");
        }
    }

}

void SeimiPage::setUseCookie(bool useCoookie){
    _useCookie = useCoookie;
}

void SeimiPage::setPostParam(QString &jsonStr){
    _postParamStr = jsonStr;
}

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

#include "NetworkAccessManager.h"
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QStandardPaths>
#include <QString>
#include <QDebug>

NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent),
    requestFinishedCount(0), requestFinishedFromCacheCount(0), requestFinishedPipelinedCount(0),
    requestFinishedSecureCount(0), requestFinishedDownloadBufferCount(0),_ua("Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.84 Safari/537.36"),
    _resourceTimeout(20000)
{
    connect(this, SIGNAL(finished(QNetworkReply*)),
            SLOT(requestFinished(QNetworkReply*)));
#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    QString location = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    diskCache->setCacheDirectory(location);
    setCache(diskCache);
}

RequestTimer::RequestTimer(QObject* parent)
    : QTimer(parent)
{
}

QNetworkReply* NetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
{
    QNetworkRequest request = req;
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setRawHeader("User-Agent",_ua.toUtf8());
    QNetworkReply* reply = QNetworkAccessManager::createRequest(op, request, outgoingData);

    RequestTimer* rt = new RequestTimer(reply);
    rt->reply = reply;
    rt->setInterval(_resourceTimeout);
    rt->setSingleShot(true);
    rt->start();

    connect(rt, SIGNAL(timeout()), this, SLOT(resourceTimeout()));

    return reply;
}

void NetworkAccessManager::requestFinished(QNetworkReply *reply)
{
    requestFinishedCount++;

    if (reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool() == true)
        requestFinishedFromCacheCount++;

    if (reply->attribute(QNetworkRequest::HttpPipeliningWasUsedAttribute).toBool() == true)
        requestFinishedPipelinedCount++;

    if (reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute).toBool() == true)
        requestFinishedSecureCount++;

    if (reply->attribute(QNetworkRequest::DownloadBufferAttribute).isValid() == true)
        requestFinishedDownloadBufferCount++;

    if (requestFinishedCount % 10)
        return;

    double pctCached = (double(requestFinishedFromCacheCount) * 100.0/ double(requestFinishedCount));
    double pctPipelined = (double(requestFinishedPipelinedCount) * 100.0/ double(requestFinishedCount));
    double pctSecure = (double(requestFinishedSecureCount) * 100.0/ double(requestFinishedCount));
    double pctDownloadBuffer = (double(requestFinishedDownloadBufferCount) * 100.0/ double(requestFinishedCount));
    //http://stackoverflow.com/a/27479099/3035247
    qInfo("[seimi] TargetUrl[%s] STATS [%lli requests total] [%3.2f%% from cache] [%3.2f%% pipelined] [%3.2f%% SSL/TLS] [%3.2f%% Zerocopy]",_currentMainTarget.toUtf8().constData(), requestFinishedCount, pctCached, pctPipelined, pctSecure, pctDownloadBuffer);
}

#ifndef QT_NO_OPENSSL
void NetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    // check if SSL certificate has been trusted already
    QString replyHost = reply->url().host() + QString(":%1").arg(reply->url().port());
    if(! sslTrustedHostList.contains(replyHost)) {
        QStringList errorStrings;
        for (int i = 0; i < error.count(); ++i)
            errorStrings += error.at(i).errorString();
        QString errors = errorStrings.join(QLatin1String("\n"));
        qDebug()<<"SSL_ERR "<< errors;
        reply->ignoreSslErrors();
        sslTrustedHostList.append(replyHost);
    }
}
#endif

void NetworkAccessManager::setCurrentUrl(const QString &current){
    _currentMainTarget = current;
}

void NetworkAccessManager::resourceTimeout(){
    RequestTimer* rt = qobject_cast<RequestTimer*>(sender());
    if (!rt->reply) {
        return;
    }
    // Abort the reply that we attached to the Network Timeout
    qInfo("[seimi] Resource[%s] request timeout.",rt->reply->request().url().toString().toUtf8().constData());
    rt->reply->abort();
}

void NetworkAccessManager::setUserAgent(const QString &ua){
    if(!ua.isEmpty()){
        _ua = ua;
    }
}

void NetworkAccessManager::setResourceTimeout(int resourceTimeout){
    if(resourceTimeout > 0){
        _resourceTimeout = resourceTimeout;
    }
}

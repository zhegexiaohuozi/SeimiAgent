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

#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>
class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    NetworkAccessManager(QObject *parent = 0);
    virtual QNetworkReply* createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );
    void setCurrentUrl(const QString &current);
    void setUserAgent(const QString &ua);
    void setResourceTimeout(int resourceTimeout);

private:
    QList<QString> sslTrustedHostList;
    qint64 requestFinishedCount;
    qint64 requestFinishedFromCacheCount;
    qint64 requestFinishedPipelinedCount;
    qint64 requestFinishedSecureCount;
    qint64 requestFinishedDownloadBufferCount;
    QString _currentMainTarget;
    QString _ua;
    int _resourceTimeout;
signals:
    void resourceTimeOut();
public slots:
    void requestFinished(QNetworkReply *reply);
    void resourceTimeout();

#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &error);
#endif
};

class RequestTimer : public QTimer
{
    Q_OBJECT

public:
    RequestTimer(QObject* parent = 0);
    QNetworkReply* reply;
};
#endif // NETWORKACCESSMANAGER_H

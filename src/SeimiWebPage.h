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
#ifndef SEIMIPAGE_H
#define SEIMIPAGE_H

#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QThread>
#include <QNetworkProxy>
#include <QFile>
#include <QtWebKitWidgets/QWebPage>
#include <QtWebKitWidgets/QWebFrame>
#include "cookiejar.h"

class SeimiPage : public QObject
{
    Q_OBJECT
public:
    explicit SeimiPage(QObject *parent = 0);

signals:
    void loadOver();

public slots:
    void loadAllFinished(bool);
    void renderFinalHtml();
    void processLog(int p);
    void toLoad(const QString &url, int renderTime, const QString &ua);

public:
    bool isOver();
    bool isProxySet();
    QString getContent();
    void startLoad(const QString &url);
    void setProxy(QNetworkProxy &proxy);
    void setScript(QString &script);
    void setUseCookie(bool useCoookie);
    void setPostParam(QString &jsonStr);
    QByteArray generateImg(QSize &targetSize);
    QByteArray generatePdf();
private:
    QWebFrame *_sWebFrame;
    QWebPage *_sWebPage;
    QNetworkProxy _proxy;
    bool _isProxyHasBeenSet;
    QString _content;
    bool _isContentSet;
    int _renderTime;
    QString _script;
    bool _useCookie;
    /**
     * json format
     * @brief _postParamStr
     */
    QString _postParamStr;
    QString _url;

};

#endif // SEIMIPAGE_H

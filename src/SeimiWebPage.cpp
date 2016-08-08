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
#include <QPainter>
#include <QTemporaryFile>
#include <QBuffer>
#include <QEventLoop>
#include "NetworkAccessManager.h"
#include "SeimiAgent.h"
#include <QPrinter>

SeimiPage::SeimiPage(QObject *parent) : QObject(parent)
{
    _sWebPage = new QWebPage(this);
    QWebSettings* default_settings = _sWebPage->settings();
            default_settings->setAttribute(QWebSettings::JavascriptEnabled,true);
            default_settings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled,true);
            default_settings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled,true);
            default_settings->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,true);
            default_settings->setAttribute(QWebSettings::LocalStorageEnabled,true);
            default_settings->setAttribute(QWebSettings::JavascriptCanAccessClipboard,true);
            default_settings->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);

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
        QVariant evalResult;
        evalResult = _sWebPage->mainFrame()->evaluateJavaScript(_script);
        qDebug() << "[Seimi] - evaluateJavaScript result=" << evalResult;
        qInfo()<< "[Seimi] evaluateJavaScript done. script=" << _script;
        QEventLoop eventLoop;
        QTimer::singleShot(_renderTime/2,&eventLoop,SLOT(quit()));
        eventLoop.exec();
    }
    _content = _sWebPage->mainFrame()->toHtml();
    _isContentSet = true;
    emit loadOver();
    qInfo("[Seimi] Document render out over.");
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
    qInfo("[seimi] TargetUrl[%s] process:%d%",_url.toUtf8().constData(),p);
}

void SeimiPage::toLoad(const QString &url,int renderTime,const QString &ua){
    this->_url = url;
    this->_renderTime = renderTime;
    NetworkAccessManager *m_networkAccessManager = new NetworkAccessManager(this);
    m_networkAccessManager->setCurrentUrl(url);
    m_networkAccessManager->setUserAgent(ua);
    if(isProxySet()){
        m_networkAccessManager->setProxy(_proxy);
    }
    if(_useCookie){
        m_networkAccessManager->setCookieJar(new CookieJar());
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
            qWarning("[seimi] postParam[%s] is invalid",_postParamStr.toUtf8().constData());
            renderFinalHtml();
        }
    }

}

void SeimiPage::setUseCookie(bool useCoookie){
    _useCookie = useCoookie;
}

void SeimiPage::setPostParam(QString &jsonStr){
    _postParamStr = jsonStr;
}

QByteArray SeimiPage::generateImg(QSize &targetSize){
    if(targetSize.isNull()||targetSize.width()<=0||targetSize.height()<=0){
        targetSize = _sWebPage->mainFrame()->contentsSize();
    }
    QRect tRect = QRect(QPoint(0, 0), targetSize);
    qDebug()<<"finalSize:"<<targetSize;
    QSize oriViewportSize = _sWebPage->viewportSize();
    _sWebPage->setViewportSize(targetSize);
#ifdef Q_OS_WIN
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
#else
    QImage::Format format = QImage::Format_ARGB32;
#endif
    QImage imgRes(tRect.size(), format);
    imgRes.fill(Qt::transparent);
    QPainter painter;
    int chipSize = 4096;
    int xChipNum = (imgRes.width()+chipSize -1)/chipSize;
    int yChipNum = (imgRes.height()+chipSize -1)/chipSize;
    for (int x = 0; x < xChipNum; ++x) {
        for (int y = 0; y < yChipNum; ++y) {
            QImage tmpBuffer(chipSize, chipSize, format);
            tmpBuffer.fill(Qt::transparent);
            painter.begin(&tmpBuffer);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.translate(-tRect.left(), -tRect.top());
            painter.translate(-x * chipSize, -y * chipSize);
            _sWebPage->mainFrame()->render(&painter, QRegion(tRect));
            painter.end();
            // do merge
            painter.begin(&imgRes);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawImage(x * chipSize, y * chipSize, tmpBuffer);
            painter.end();
        }
    }
    _sWebPage->setViewportSize(oriViewportSize);
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);
    imgRes.save(&buffer,"png",-1);
    return out;
}

QByteArray SeimiPage::generatePdf(){
    int contentWidth = _sWebPage->mainFrame()->contentsSize().width();
    int contentHeight = _sWebPage->mainFrame()->contentsSize().height();
    if(contentWidth <=0||contentHeight<=0){
        return QByteArray();
    }
    QPrinter printer;
    printer.setPageSize(QPrinter::A4);
	printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOrientation(QPrinter::Portrait);
    printer.setFullPage(true);
    printer.setCollateCopies(true);
    printer.setPageMargins(0, 0, 0, 0, QPrinter::Point);
	//try again and again,finally get the relationship between in contentWidth,printerWidth,dpi.
	int coefDpi = contentWidth * 100 / printer.width();
    qDebug() <<"content W:"<<contentWidth<< " A4 w:" << printer.width() << "coef:"<<coefDpi;
    printer.setResolution(coefDpi);
    QTemporaryFile pdf;
    pdf.open();
    printer.setOutputFileName(pdf.fileName());
    _sWebPage->mainFrame()->print(&printer);
    QByteArray out = pdf.readAll();
    pdf.deleteLater();
    return out;
}

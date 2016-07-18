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
#include "cookiejar.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QUrl>
#include <QSortFilterProxyModel>
#include <QNetworkCookie>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QWebSettings>

#include <QDebug>

static const unsigned int JAR_VERSION = 23;

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for(quint32 i = 0; i < count; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0) {
            qWarning() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}
QT_END_NAMESPACE

CookieJar::CookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
    , m_loaded(false)
    , m_acceptCookies(AcceptOnlyFromSitesNavigatedTo)
{
}

CookieJar::~CookieJar()
{
    if (m_keepCookies == KeepUntilExit)
        clear();
//    m_saveTimer->saveIfNeccessary();
}

void CookieJar::clear()
{
    setAllCookies(QList<QNetworkCookie>());
//    m_saveTimer->changeOccurred();
    emit cookiesChanged();
}

void CookieJar::load()
{
    if (m_loaded)
        return;
    // load cookies and exceptions
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");
    QSettings cookieSettings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    setAllCookies(qvariant_cast<QList<QNetworkCookie> >(cookieSettings.value(QLatin1String("cookies"))));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    m_exceptions_block = cookieSettings.value(QLatin1String("block")).toStringList();
    m_exceptions_allow = cookieSettings.value(QLatin1String("allow")).toStringList();
    m_exceptions_allowForSession = cookieSettings.value(QLatin1String("allowForSession")).toStringList();
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());

    loadSettings();
}

void CookieJar::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QByteArray value = settings.value(QLatin1String("acceptCookies"),
                        QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    m_acceptCookies = acceptPolicyEnum.keyToValue(value) == -1 ?
                        AcceptOnlyFromSitesNavigatedTo :
                        static_cast<AcceptPolicy>(acceptPolicyEnum.keyToValue(value));

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("KeepUntilExpire")).toByteArray();
    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    m_keepCookies = keepPolicyEnum.keyToValue(value) == -1 ?
                        KeepUntilExpire :
                        static_cast<KeepPolicy>(keepPolicyEnum.keyToValue(value));

    if (m_keepCookies == KeepUntilExit)
        setAllCookies(QList<QNetworkCookie>());

    m_loaded = true;
    emit cookiesChanged();
}

void CookieJar::save()
{
    if (!m_loaded)
        return;
    purgeOldCookies();
    QString directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (directory.isEmpty()) {
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    }
    if (!QFile::exists(directory)) {
        QDir dir;
        dir.mkpath(directory);
    }
    QSettings cookieSettings(directory + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }
    cookieSettings.setValue(QLatin1String("cookies"), QVariant::fromValue<QList<QNetworkCookie> >(cookies));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    cookieSettings.setValue(QLatin1String("block"), m_exceptions_block);
    cookieSettings.setValue(QLatin1String("allow"), m_exceptions_allow);
    cookieSettings.setValue(QLatin1String("allowForSession"), m_exceptions_allowForSession);

    // save cookie settings
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(m_acceptCookies)));

    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(m_keepCookies)));
}

void CookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
        return;
    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
            cookies.removeAt(i);
    }
    if (oldCount == cookies.count())
        return;
    setAllCookies(cookies);
    emit cookiesChanged();
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    CookieJar *that = const_cast<CookieJar*>(this);
    if (!m_loaded)
        that->load();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        QList<QNetworkCookie> noCookies;
        return noCookies;
    }

    return QNetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!m_loaded)
        load();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return false;

    QString host = url.host();
    bool eBlock = qBinaryFind(m_exceptions_block.begin(), m_exceptions_block.end(), host) != m_exceptions_block.end();
    bool eAllow = qBinaryFind(m_exceptions_allow.begin(), m_exceptions_allow.end(), host) != m_exceptions_allow.end();
    bool eAllowSession = qBinaryFind(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end(), host) != m_exceptions_allowForSession.end();

    bool addedCookies = false;
    // pass exceptions
    bool acceptInitially = (m_acceptCookies != AcceptNever);
    if ((acceptInitially && !eBlock)
        || (!acceptInitially && (eAllow || eAllowSession))) {
        // pass url domain == cookie domain
        QDateTime soon = QDateTime::currentDateTime();
        soon = soon.addDays(90);
        foreach(QNetworkCookie cookie, cookieList) {
            QList<QNetworkCookie> lst;
            if (m_keepCookies == KeepUntilTimeLimit
                && !cookie.isSessionCookie()
                && cookie.expirationDate() > soon) {
                    cookie.setExpirationDate(soon);
            }
            lst += cookie;
            if (QNetworkCookieJar::setCookiesFromUrl(lst, url)) {
                addedCookies = true;
            } else {
                // finally force it in if wanted
                if (m_acceptCookies == AcceptAlways) {
                    QList<QNetworkCookie> cookies = allCookies();
                    cookies += cookie;
                    setAllCookies(cookies);
                    addedCookies = true;
                }
#if 0
                else
                    qWarning() << "setCookiesFromUrl failed" << url << cookieList.value(0).toRawForm();
#endif
            }
        }
    }

    if (addedCookies) {
//        m_saveTimer->changeOccurred();
        emit cookiesChanged();
    }
//    qDebug()<<"set cookies";
    return addedCookies;
}

CookieJar::AcceptPolicy CookieJar::acceptPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_acceptCookies;
}

void CookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_acceptCookies)
        return;
    m_acceptCookies = policy;
//    m_saveTimer->changeOccurred();
}

CookieJar::KeepPolicy CookieJar::keepPolicy() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_keepCookies;
}

void CookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (!m_loaded)
        load();
    if (policy == m_keepCookies)
        return;
    m_keepCookies = policy;
//    m_saveTimer->changeOccurred();
}

QStringList CookieJar::blockedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_block;
}

QStringList CookieJar::allowedCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allow;
}

QStringList CookieJar::allowForSessionCookies() const
{
    if (!m_loaded)
        (const_cast<CookieJar*>(this))->load();
    return m_exceptions_allowForSession;
}

void CookieJar::setBlockedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_block = list;
    qSort(m_exceptions_block.begin(), m_exceptions_block.end());
//    m_saveTimer->changeOccurred();
}

void CookieJar::setAllowedCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allow = list;
    qSort(m_exceptions_allow.begin(), m_exceptions_allow.end());
//    m_saveTimer->changeOccurred();
}

void CookieJar::setAllowForSessionCookies(const QStringList &list)
{
    if (!m_loaded)
        load();
    m_exceptions_allowForSession = list;
    qSort(m_exceptions_allowForSession.begin(), m_exceptions_allowForSession.end());
//    m_saveTimer->changeOccurred();
}

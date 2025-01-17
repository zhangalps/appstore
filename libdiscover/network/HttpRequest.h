﻿/*
 *   SPDX-FileCopyrightText:      2021 Wang Rui <wangrui@jingos.com>
 *   SPDX-License-Identifier:     LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "HttpResponse.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QDebug>
#include "discovercommon_export.h"

//namespace AeaQt {

#ifdef QT_APP_DEBUG
#define log_debugger qDebug().noquote().nospace() \
                          << "[AeaQt::Network] Debug: -> " \
                          << "function: " << __func__ << "; " \
                          << "line: " << __LINE__ << "; "
#else
#define log_debugger QString()
#endif

#define log_warning qWarning().noquote().nospace() \
                           << "[AeaQt::Network] Warning: -> " \
                           << "function: " << __func__ << "; " \
                           << "line: " << __LINE__ << "; "

class HttpClient;

class DISCOVERCOMMON_EXPORT HttpRequest
{
public:
    enum BodyType {
        None = 0, // This request does not have a body.
        X_Www_Form_Urlencoded, // x-www-form-urlencoded
        Raw_Text_Json, // application/json
    };

    explicit HttpRequest(QNetworkAccessManager::Operation op, HttpClient *jsonHttpClient);
    virtual ~HttpRequest();

    HttpRequest &url(const QString &url);
    HttpRequest &header(const QString &key, const QVariant &value);
    HttpRequest &headers(const QMap<QString, QVariant> &headers);

    HttpRequest &queryParam(const QString &key, const QVariant &value);
    HttpRequest &queryParams(const QMap<QString, QVariant> &params);
    void insertPublicQueryParams();

    /* Mainly used for identification */
    HttpRequest &userAttribute(const QVariant &value);

    HttpRequest &body(const QVariantMap &content);
    HttpRequest &body(const QJsonObject &content);
    HttpRequest &body(const QByteArray &content);

    /*
     * @onRespone slot support type: void function(QVariantMap resultMap) OR
     *                               void function(QByteArray resultData) OR
     *                               void function(QNetworkReply* reply)
     * note: The same type is only triggered once
     */
    HttpRequest &onResponse(const QObject *receiver, const char *slot, HttpResponse::SupportMethod type = HttpResponse::AutoInfer);
    HttpRequest &onResponse(std::function<void (QNetworkReply*)> lambda);
    HttpRequest &onResponse(std::function<void (QVariantMap)> lambda);
    HttpRequest &onResponse(std::function<void (QByteArray)> lambda);
    HttpRequest &onResponse(std::function<void (qint64, qint64)> lambda);
    /*
     * @onError slot support type: void function(QNetworkReply::NetworkError error)
     *                             void function(QString errorString);
     *                             void function(QNetworkReply::NetworkError error, QNetworkReply* reply);
     *                             void function(QString errorString, QNetworkReply* reply);
     * note: The same type is only triggered once
     */
    HttpRequest &onError(const QObject *receiver, const char *slot);
    HttpRequest &onError(std::function<void (QNetworkReply::NetworkError)> lambda);
    HttpRequest &onError(std::function<void (QString)> lambda);
    HttpRequest &onError(std::function<void (QNetworkReply::NetworkError, QNetworkReply*)> lambda);
    HttpRequest &onError(std::function<void (QString, QNetworkReply*)> lambda);

    /**
     * @brief msec <= 0, disable timeout
     *        msec >  0, enable timeout
     */
    HttpRequest &timeout(const int &msec = -1);

    /**
     * @brief Block current thread, entering an event loop.
     */
    HttpRequest &block();

    HttpRequest &removePublicQueryParams();


    HttpResponse *exec();

private:
    HttpRequest();
    HttpRequest &onResponse(QVariant lambda);

private:
    QNetworkRequest                  m_networkRequest;
    QByteArray                       m_body;
    QNetworkAccessManager::Operation m_op;
    HttpClient                      *m_httpService;
    int                              m_timeout;
    bool                             m_isBlock = false;
    QMultiMap<HttpResponse::SupportMethod, QPair<QString, QVariant>> m_slotsMap;
    bool m_isInsertPublic = true;
};

//}
#endif // HTTP_REQUEST_H

﻿/*
 *   SPDX-FileCopyrightText:      2021 Wang Rui <wangrui@jingos.com>
 *   SPDX-License-Identifier:     LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <QNetworkReply>
#include <QMultiMap>
#include <functional>
#include <QTimer>
#include "discovercommon_export.h"

//namespace AeaQt {

class HttpResponseTimeout : public QObject {
    Q_OBJECT
public:
    HttpResponseTimeout(QNetworkReply *parent = NULL, const int timeout = -1) : QObject(parent) {
        if (timeout > 0)
            QTimer::singleShot(timeout, this, SLOT(onTimeout()));
    }

private Q_SLOTS:
    void onTimeout() {
        QNetworkReply *reply = static_cast<QNetworkReply*>(parent());
        if (reply->isRunning()) {
            reply->abort();
            reply->deleteLater();
        }
    };
};

class DISCOVERCOMMON_EXPORT HttpResponse : public QObject
{
    Q_OBJECT
public:
    /*
     * Support Reflex Method
     * default: AutoInfer
     * AutoInfer: Automatic derivation based on type
     */
    enum SupportMethod {
        Invalid = 0,
        AutoInfer,
        onResponse_QNetworkReply_A_Pointer,    /* method: void function(QNetworkReply* reply); Is_AutoInfer: true */
        onResponse_QByteArray,                 /* method: void function(QByteArray data); Is_AutoInfer: true */
        onResponse_QVariantMap,                /* method: void function(QVariantMap map); Is_AutoInfer: true */
        onDownloadProgress_qint64_qint64,      /* method: void function(qint64 bytesReceived, qint64 bytesTotal); Is_AutoInfer: true */
        onError_QNetworkReply_To_NetworkError, /* method: void function(QNetworkReply::NetworkError error); Is_AutoInfer: true */
        onError_QString,                       /* method: void function(QString errorString); Is_AutoInfer: true */
        onError_QNetworkReply_To_NetworkError_QNetworkReply_A_Pointer, /* method: void function(QNetworkReply::NetworkError error, QNetworkReply* reply); Is_AutoInfer: true */
        onError_QString_QNetworkReply_A_Poniter/* method: void function(QString errorString, QNetworkReply* reply); Is_AutoInfer: true */
    };

    explicit HttpResponse(QNetworkReply *networkReply,
                          const QMultiMap<SupportMethod, QPair<QString, QVariant> > &slotsMap,
                          const int &timeout,
                          bool isBlock);

    virtual ~HttpResponse();

    QNetworkReply *networkReply();

protected:
    void slotsMapOperation(QMultiMap<SupportMethod, QPair<QString, QVariant> > &slotsMap);

Q_SIGNALS:
    void finished(QNetworkReply *reply);
    void finished(QByteArray data);
    void finished(QVariantMap map);
    void error(QString errorString);
    void error(QNetworkReply::NetworkError error);
    void error(QString errorString, QNetworkReply *reply);
    void error(QNetworkReply::NetworkError error, QNetworkReply *reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private Q_SLOTS:
    void onFinished();
    void onError(QNetworkReply::NetworkError error);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    HttpResponse();

private:
    QMultiMap<SupportMethod, QPair<QString, QVariant> > m_slotsMap;
    QNetworkReply *m_networkReply;
};

//}

Q_DECLARE_METATYPE(std::function<void (QNetworkReply*)>)
Q_DECLARE_METATYPE(std::function<void (QByteArray)>)
Q_DECLARE_METATYPE(std::function<void (QVariantMap)>)

Q_DECLARE_METATYPE(std::function<void (QString)>)
Q_DECLARE_METATYPE(std::function<void (QNetworkReply::NetworkError)>)
Q_DECLARE_METATYPE(std::function<void (QNetworkReply::NetworkError, QNetworkReply *)>)
Q_DECLARE_METATYPE(std::function<void (QString, QNetworkReply *)>)

Q_DECLARE_METATYPE(std::function<void (qint64, qint64)>)

#endif // HTTP_RESPONSE_H

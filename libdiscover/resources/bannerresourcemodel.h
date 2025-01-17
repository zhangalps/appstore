/*
 *   SPDX-FileCopyrightText:      2021 Wang Rui <wangrui@jingos.com>
 *   SPDX-License-Identifier:     LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#ifndef BANNERRESOURCEMODEL_H
#define BANNERRESOURCEMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QStandardPaths>
#include <QVector>
#include <QThread>

#include "discovercommon_export.h"
#include "bannerappresource.h"

#define BANNER_URL "applist"

class LocalBannerThread :public QThread
{
    Q_OBJECT
public:
    LocalBannerThread();
    void run() override;
Q_SIGNALS:
    void loadLocalSuc(QByteArray data,bool isNetworkRequest);
public Q_SLOTS:
    void onNetworkStop(bool isSuc);
private:
    bool isNetworkStop = false;
};

class DISCOVERCOMMON_EXPORT BannerResourceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum BannerRoleTypes
    {
        BANNER_APP
    };
    explicit BannerResourceModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    static BannerResourceModel* global();
    void loadBannerData();
Q_SIGNALS:
    void networkStart();
    void networkStop(bool isSuc);
public Q_SLOTS:
    void createbannerData(QByteArray bannerData,bool isNetworkRequest);

private:
    QList<BannerAppResource*> m_banners;
    bool netWorkSuc = false;
    LocalBannerThread *localThread;
    QString currentLang = "en";


};

#endif // BANNERRESOURCEMODEL_H

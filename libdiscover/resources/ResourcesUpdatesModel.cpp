/***************************************************************************
 *   Copyright © 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "ResourcesUpdatesModel.h"
#include "ResourcesModel.h"
#include "AbstractBackendUpdater.h"
#include "AbstractResource.h"
#include <QDateTime>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>

#include <KLocalizedString>
#include <KFormat>

ResourcesUpdatesModel::ResourcesUpdatesModel(QObject* parent)
    : QStandardItemModel(parent)
    , m_resources(nullptr)
    , m_lastIsProgressing(false)
    , m_kded(nullptr)
{
    setResourcesModel(ResourcesModel::global());
}

void ResourcesUpdatesModel::setResourcesModel(ResourcesModel* model)
{
    Q_ASSERT(model);
    m_resources = model;
    m_updaters.clear();
    addNewBackends();
    connect(model, SIGNAL(backendsChanged()), SLOT(addNewBackends()));
}

void ResourcesUpdatesModel::addNewBackends()
{
    QVector<AbstractResourcesBackend*> backends = ResourcesModel::global()->backends();
    foreach(AbstractResourcesBackend* b, backends) {
        AbstractBackendUpdater* updater = b->backendUpdater();
        if(updater && !m_updaters.contains(updater)) {
            connect(updater, SIGNAL(progressChanged(qreal)), SIGNAL(progressChanged()));
            connect(updater, SIGNAL(statusMessageChanged(QString)), SIGNAL(statusMessageChanged(QString)));
            connect(updater, SIGNAL(statusMessageChanged(QString)), SLOT(message(QString)));
            connect(updater, SIGNAL(statusDetailChanged(QString)), SLOT(message(QString)));
            connect(updater, SIGNAL(statusDetailChanged(QString)), SIGNAL(statusDetailChanged(QString)));
            connect(updater, SIGNAL(remainingTimeChanged()), SIGNAL(etaChanged()));
            connect(updater, SIGNAL(downloadSpeedChanged(quint64)), SIGNAL(downloadSpeedChanged()));
            connect(updater, SIGNAL(progressingChanged(bool)), SLOT(slotProgressingChanged(bool)));
            connect(updater, SIGNAL(cancelableChanged(bool)), SIGNAL(cancelableChanged()));
            connect(updater, SIGNAL(destroyed(QObject*)), SLOT(updaterDestroyed(QObject*)));
            m_updaters += updater;
        }
    }
}

void ResourcesUpdatesModel::updaterDestroyed(QObject* obj)
{
//     TODO: use removeAll when build.kde.org doesn't complain about Qt 5.4 API usage...
    int idx = m_updaters.indexOf(qobject_cast<AbstractBackendUpdater*>(obj));
    if (idx>=0)
        m_updaters.remove(idx);
}

void ResourcesUpdatesModel::slotProgressingChanged(bool progressing)
{
    Q_UNUSED(progressing);
    const bool newProgressing = isProgressing();
    if (newProgressing != m_lastIsProgressing) {
        m_lastIsProgressing = newProgressing;
        if (!m_lastIsProgressing) {
            if (!m_kded)
                m_kded = new QDBusInterface("org.kde.kded", "/kded",
                                            "org.kde.kded", QDBusConnection::sessionBus(), this);
            QDBusReply<QStringList> lM = m_kded->call("loadedModules");
            QStringList services = lM.value();
            foreach (const QString &service, services) {
                if (!service.startsWith("muon"))
                    continue;
                
                QDBusMessage message = QDBusMessage::createMethodCall("org.kde.kded",
                                        "/modules/" + service,
                                        "org.kde.kded.AbstractKDEDModule",
                                        "recheckSystemUpdateNeeded");
                QDBusConnection::sessionBus().send(message);
            }
        }
        emit progressingChanged(newProgressing);

        if (!newProgressing) {
            Q_EMIT finished();
        }
    }
}

qreal ResourcesUpdatesModel::progress() const
{
    qreal total = 0;
    foreach(AbstractBackendUpdater* updater, m_updaters) {
        total += updater->progress();
    }
    return total / m_updaters.count();
}

void ResourcesUpdatesModel::message(const QString& msg)
{
    if(msg.isEmpty())
        return;

    appendRow(new QStandardItem(msg));
}

void ResourcesUpdatesModel::prepare()
{
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        upd->prepare();
    }
}

void ResourcesUpdatesModel::updateAll()
{
    Q_ASSERT(m_resources);
    
    if(m_updaters.isEmpty())
        emit progressingChanged(false);
    else {
        Q_FOREACH (AbstractBackendUpdater* upd, m_updaters) {
            if (upd->hasUpdates())
                QMetaObject::invokeMethod(upd, "start", Qt::QueuedConnection);
        }
    }
}


QString ResourcesUpdatesModel::remainingTime() const
{
    long unsigned int maxEta = 0;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        maxEta = qMax(maxEta, upd->remainingTime());
    }

    // Ignore ETA if it's larger than 2 days.
    if(maxEta > 2 * 24 * 60 * 60)
        return QString();
    else if(maxEta==0)
        return i18nc("@item:intext Unknown remaining time", "Updating...");
    else
        return i18nc("@item:intext Remaining time", "%1 remaining", KFormat().formatDuration(maxEta));
}

bool ResourcesUpdatesModel::hasUpdates() const
{
    bool ret = false;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        ret |= upd->hasUpdates();
    }
    return ret;
}

quint64 ResourcesUpdatesModel::downloadSpeed() const
{
    quint64 ret = 0;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        ret += upd->downloadSpeed();
    }
    return ret;
}

bool ResourcesUpdatesModel::isCancelable() const
{
    bool cancelable = false;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        cancelable |= upd->isCancelable();
    }
    return cancelable;
}

bool ResourcesUpdatesModel::isProgressing() const
{
    bool progressing = false;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        progressing |= upd->isProgressing();
    }
    return progressing;
}

bool ResourcesUpdatesModel::isAllMarked() const
{
    bool allmarked = false;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        allmarked |= upd->isAllMarked();
    }
    return allmarked;
}

QList<AbstractResource*> ResourcesUpdatesModel::toUpdate() const
{
    QList<AbstractResource*> ret;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        ret += upd->toUpdate();
    }
    return ret;
}

void ResourcesUpdatesModel::addResources(const QList<AbstractResource*>& resources)
{
    QMap<AbstractResourcesBackend*, QList<AbstractResource*> > sortedResources;
    foreach(AbstractResource* res, resources) {
        sortedResources[res->backend()] += res;
    }

    for(auto it=sortedResources.constBegin(), itEnd=sortedResources.constEnd(); it!=itEnd; ++it) {
        it.key()->backendUpdater()->addResources(*it);
    }
}

void ResourcesUpdatesModel::removeResources(const QList< AbstractResource* >& resources)
{
    QMap<AbstractResourcesBackend*, QList<AbstractResource*> > sortedResources;
    foreach(AbstractResource* res, resources) {
        sortedResources[res->backend()] += res;
    }

    for(auto it=sortedResources.constBegin(), itEnd=sortedResources.constEnd(); it!=itEnd; ++it) {
        it.key()->backendUpdater()->removeResources(*it);
    }
}

QDateTime ResourcesUpdatesModel::lastUpdate() const
{
    QDateTime ret;
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        QDateTime current = upd->lastUpdate();
        if(!ret.isValid() || (current.isValid() && current>ret)) {
            ret = current;
        }
    }
    return ret;
}

void ResourcesUpdatesModel::cancel()
{
    foreach(AbstractBackendUpdater* upd, m_updaters) {
        if(upd->isCancelable())
            upd->cancel();
        else
            qWarning() << "tried to cancel " << upd->metaObject()->className() << "which is not cancelable";
    }
}

qint64 ResourcesUpdatesModel::secsToLastUpdate() const
{
    return lastUpdate().secsTo(QDateTime::currentDateTime());
}
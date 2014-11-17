/*
 *  This file is part of qtzeroconf. (c) 2012 Johannes Hilden
 *  https://github.com/johanneshilden/qtzeroconf
 *
 *  qtzeroconf is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  qtzeroconf is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with qtzeroconf; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <QDebug>
#include <QStringBuilder>

#include <avahi-client/publish.h>

#include <avahi-common/error.h>
#include <avahi-common/alternative.h>

#include "qtzeroconf/zconfserviceclient.h"
#include "qtzeroconf/zconfservice.h"

class ZConfServicePrivate
{
public:
    static void callback(AvahiEntryGroup      * const group,
                         AvahiEntryGroupState   const state,
                         void                 * const userdata)
    {
        Q_UNUSED(group);
        if(nullptr != userdata)
        {
            const ZConfService * const serviceGroup = static_cast<ZConfService *>(userdata);
            switch (state)
            {
            case AVAHI_ENTRY_GROUP_ESTABLISHED:
                emit serviceGroup->entryGroupEstablished();
                qDebug() << (QLatin1String("Service '") % serviceGroup->d_ptr->name % QLatin1String("' successfully establised."));
                break;
            case AVAHI_ENTRY_GROUP_COLLISION:
                emit serviceGroup->entryGroupNameCollision();
                break;
            case AVAHI_ENTRY_GROUP_FAILURE:
                emit serviceGroup->entryGroupFailure();
                qDebug() << (QLatin1String("Entry group failure: ") % serviceGroup->errorString());
                break;
            case AVAHI_ENTRY_GROUP_UNCOMMITED:
                qDebug() << QLatin1String("AVAHI_ENTRY_GROUP_UNCOMMITED");
                break;
            case AVAHI_ENTRY_GROUP_REGISTERING:
                qDebug() << QLatin1String("AVAHI_ENTRY_GROUP_REGISTERING");
            } // end switch
        }
    }

    ZConfServiceClient * client = nullptr;
    AvahiEntryGroup    * group  = nullptr;
    QString              name;
    in_port_t            port;
    QString              type;
    int                  error = 0;
};

/*!
    \class ZConfService

    \brief This class provides Avahi Zeroconf service registration. It can be
    used by server applications to announce a service on the local area network.

    Typical use involves creating an instance of ZConfService and calling
    registerService() with a service name and port number.
 */

ZConfService::ZConfService(QObject *const parent)
    : QObject(parent),
      d_ptr(new ZConfServicePrivate)
{
    d_ptr->client = new ZConfServiceClient(this);
    d_ptr->client->run();
}

/*!
    Destroys the object and releases all resources associated with it.
 */
ZConfService::~ZConfService()
{
    if(nullptr != d_ptr->group)
    {
        avahi_entry_group_free(d_ptr->group);
    }
    delete d_ptr;
}

/*!
    Returns true if the service group was added and commited without error.
 */
bool ZConfService::isValid() const
{
    return (   (nullptr != d_ptr->group)
            && (0 == d_ptr->error));
}

/*!
    Returns a human readable error string with details of the last error that
    occured.
 */
QString ZConfService::errorString() const
{
    if(nullptr == d_ptr->client->client)
    {
        return QLatin1String("No client!");
    }
    return avahi_strerror(avahi_client_errno(d_ptr->client->client));
}

/*!
    Registers a Zeroconf service on the LAN. If no service type is specified,
    "_http._tcp" is assumed. Needless to say, the server should be available
    and listen on the specified port.
 */
void ZConfService::registerService(const QString &name,
                                   in_port_t const port,
                                   const QString &type,
                                   const QStringMap &txtRecords)
{
    if(   (nullptr == d_ptr->client->client)
       || (AVAHI_CLIENT_S_RUNNING != avahi_client_get_state(d_ptr->client->client)))
    {
        qDebug() << QLatin1String("ZConfService error: Client is not running.");
        return;
    }

    d_ptr->name = name;
    d_ptr->port = port;
    d_ptr->type = type;

    if(nullptr == d_ptr->group)
    {
        d_ptr->group = avahi_entry_group_new(d_ptr->client->client,
                                             ZConfServicePrivate::callback,
                                             this);
    }

    if(avahi_entry_group_is_empty(d_ptr->group))
    {
        AvahiStringList * avahiTXTRecords = nullptr;
        for(const QString & key : const_cast<QList<QString>>(txtRecords.uniqueKeys()))
        {
            static const QLatin1Char equals('=');
            qDebug() << key << txtRecords[key];
            avahiTXTRecords = avahi_string_list_add(avahiTXTRecords,
                                                   (key % equals % txtRecords[key]).toLocal8Bit().data());
        }

        d_ptr->error = avahi_entry_group_add_service_strlst(d_ptr->group,
                                                            AVAHI_IF_UNSPEC,
                                                            AVAHI_PROTO_UNSPEC,
                                                            (AvahiPublishFlags) 0,
                                                            d_ptr->name.toLocal8Bit().data(),
                                                            d_ptr->type.toLocal8Bit().data(),
                                                            nullptr,
                                                            nullptr,
                                                            d_ptr->port,
                                                            avahiTXTRecords);

        avahi_string_list_free(avahiTXTRecords);

        if(0 == d_ptr->error)
        {
            d_ptr->error = avahi_entry_group_commit(d_ptr->group);
        }

        if(0 != d_ptr->error)
        {
            qDebug() << (QLatin1String("Error creating service: ") % errorString());
        }
    }
}

/*!
    Deregisters the service associated with this object. You can reuse the same
    ZConfService object at any time to register another service on the network.
 */
void ZConfService::resetService()
{
    avahi_entry_group_reset(d_ptr->group);
}

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

#include <QHash>
#include <QStringBuilder>

#include <cassert>

#include <avahi-common/error.h>

#include "qtzeroconf/zconfserviceclient.h"
#include "qtzeroconf/zconfservicebrowser.h"

/*!
    \struct ZConfServiceEntry

    \brief This struct contains information about a particular Zeroconf service
    available on the local network.
 */

/*!
    \property QString ZConfServiceEntry::ip

    A string representation of the IPv4 or IPv6 IP address associated with this
    service.
 */

/*!
    \property QString ZConfServiceEntry::domain

    The domain associated with this service.
 */

/*!
    \property QString ZConfServiceEntry::host

    The host name associated with this service.
 */

/*!
    \property uint16_t ZConfServiceEntry::port

    The IP port number associated with this service.
 */
namespace
{
    static QString protocolStringName(AvahiProtocol protocol)
    {
        switch(protocol)
        {
        case AVAHI_PROTO_INET:  return QLatin1String("IPv4");
        case AVAHI_PROTO_INET6: return QLatin1String("IPv6");
        default:                return QLatin1String("Unspecified");
        }
    }
}

/*!
    A human-readable string representation of the network layer protocol used
    by this service. Possible values are "IPv4", "IPv6", and "Unspecified".
 */
QString ZConfServiceEntry::protocolName() const
{
    return protocolStringName(this->protocol);
}

/*!
    \fn bool ZConfServiceEntry::isCached() const

    Returns true if this response originates from the cache.
 */

/*!
    \fn bool ZConfServiceEntry::isWideArea() const

    Returns true if this response originates from wide area DNS.
 */

/*!
    \fn bool ZConfServiceEntry::isMulticast() const

    Returns true if this response originates from multicast DNS.
 */

/*!
    \fn bool ZConfServiceEntry::isLocal() const

    Returns true if this service resides on and was announced by the local host.
 */

class ZConfServiceBrowserPrivate
{
public:
    ZConfServiceBrowserPrivate(ZConfServiceClient * const in_client)
        : client(in_client)
    { }

    static void callback(AvahiServiceBrowser    * const browser,
                         AvahiIfIndex             const interface,
                         AvahiProtocol            const protocol,
                         AvahiBrowserEvent        const event,
                         const char             * const name,
                         const char             * const type,
                         const char             * const domain,
                         AvahiLookupResultFlags   const flags,
                         void                   * const userdata)
    {
        Q_UNUSED(browser);
        Q_UNUSED(flags);
        if(nullptr != userdata)
        {
            const QString in_name(name);
            const ZConfServiceBrowser * const serviceBrowser = static_cast<ZConfServiceBrowser *>(userdata);
            switch(event)
            {
            case AVAHI_BROWSER_FAILURE:
                qDebug() << (QLatin1String("Avahi browser error: ") % QString(avahi_strerror(avahi_client_errno(serviceBrowser->d_ptr->client->client))));
                break;
            case AVAHI_BROWSER_NEW:
                qDebug() << (QLatin1String("New service '") % in_name % QLatin1String("' of type ") % QString(type) % QLatin1String(" in domain ") % QString(domain) % QLatin1String(" on protocol ") % protocolStringName(protocol) % QLatin1String("."));

                // We ignore the returned resolver object. In the callback
                // function we free it. If the server is terminated before
                // the callback function is called the server will free
                // the resolver for us.
                if (!(avahi_service_resolver_new(serviceBrowser->d_ptr->client->client,
                                                 interface,
                                                 protocol,
                                                 name,
                                                 serviceBrowser->d_ptr->type.toLocal8Bit().data(),
                                                 domain,
                                                 AVAHI_PROTO_UNSPEC,
                                                 (AvahiLookupFlags) 0,
                                                 ZConfServiceBrowserPrivate::resolve,
                                                 const_cast<ZConfServiceBrowser * const>(serviceBrowser))))
                    qDebug() << (QLatin1String("Failed to resolve service '") % in_name % QLatin1String("': ") % avahi_strerror(avahi_client_errno(serviceBrowser->d_ptr->client->client)));
                break;
            case AVAHI_BROWSER_REMOVE:
                emit serviceBrowser->serviceEntryRemoved(in_name);
                serviceBrowser->d_ptr->entries.remove(in_name);
                qDebug() << QLatin1String("Service '") % in_name % QLatin1String("' removed from the network.");
                break;
            case AVAHI_BROWSER_ALL_FOR_NOW:
            case AVAHI_BROWSER_CACHE_EXHAUSTED:
                qDebug() << (AVAHI_BROWSER_ALL_FOR_NOW == event
                             ? QLatin1String("AVAHI_BROWSER_ALL_FOR_NOW")
                             : QLatin1String("AVAHI_BROWSER_CACHE_EXHAUSTED"));
            } // end switch
        }
    }

    static void resolve(AvahiServiceResolver   * const resolver,
                        AvahiIfIndex             const interface,
                        AvahiProtocol            const protocol,
                        AvahiResolverEvent       const event,
                        const char             * const name,
                        const char             * const type,
                        const char             * const domain,
                        const char             * const host_name,
                        const AvahiAddress     * const address,
                        uint16_t                 const port,
                        AvahiStringList        *       txt,
                        AvahiLookupResultFlags   const flags,
                        void                   * const userdata)
    {
        static char addr[AVAHI_ADDRESS_STR_MAX];
        if(nullptr != userdata)
        {
            const QString in_name(name);
            const ZConfServiceBrowser * const serviceBrowser = static_cast<ZConfServiceBrowser *>(userdata);
            switch (event)
            {
                case AVAHI_RESOLVER_FAILURE:
                    qDebug() << (QLatin1String("Failed to resolve service '") % in_name % QLatin1String("': ") % avahi_strerror(avahi_client_errno(serviceBrowser->d_ptr->client->client)));
                    break;
                case AVAHI_RESOLVER_FOUND:
                {

                    avahi_address_snprint(addr, sizeof(addr), address);
                    serviceBrowser->d_ptr->entries.insert(in_name, {interface,
                                                                    QString::fromLocal8Bit(addr),
                                                                    QString(domain),
                                                                    QString(type),
                                                                    QString(host_name),
                                                                    port,
                                                                    protocol,
                                                                    flags,
                                                                    ({
                                                                        QStringMap returnMap;
                                                                        while(nullptr != txt)
                                                                        {
                                                                            static const QLatin1Char equals('=');
                                                                            const QStringList & split = QString::fromLocal8Bit(reinterpret_cast<const char*>(&(txt->text[0])),
                                                                                                                               txt->size).split(equals);
                                                                            returnMap.insert(split.first(), split.last());
                                                                            txt = txt->next;
                                                                        }
                                                                        returnMap;
                                                                    })});
                    emit serviceBrowser->serviceEntryAdded(in_name);
                }
            }
            avahi_service_resolver_free(resolver);
        }
    }

    typedef QHash<QString, ZConfServiceEntry> ZConfServiceEntryTable;

    ZConfServiceClient     * const client;
    AvahiServiceBrowser    *       browser = nullptr;
    ZConfServiceEntryTable         entries;
    QString                        type;
    AvahiProtocol                  proto = AVAHI_PROTO_UNSPEC;
};

/*!
    \class ZConfServiceBrowser

    \brief AvahiServiceBrowser wrapper that lets you browse for services
    available on the local network. This class can be used to handle Zeroconf
    service discovery in a Qt-based client application.

    Instantiate a ZConfServiceBrowser object and call browse() with the desired
    service type as argument (e.g., "_http._tcp" or "_ipp._tcp").

    ZConfServiceBrowser will emit serviceEntryAdded() when a new service is
    discovered and serviceEntryRemoved() when a service is removed from the
    network.
 */

/*!
    Creates a Zeroconf service browser. Call browse() to start browsing for
    services.
 */
ZConfServiceBrowser::ZConfServiceBrowser(QObject *parent)
    : QObject(parent),
      d_ptr(new ZConfServiceBrowserPrivate(new ZConfServiceClient(this)))
{
    connect(d_ptr->client, &ZConfServiceClient::clientRunning, [this]()
    {
        if(nullptr != this->d_ptr->browser)
        {
            return;
        }
        this->d_ptr->browser = avahi_service_browser_new(d_ptr->client->client,
                                                         AVAHI_IF_UNSPEC,
                                                         d_ptr->proto,
                                                         d_ptr->type.toLocal8Bit().data(),
                                                         NULL,
                                                         (AvahiLookupFlags) 0,
                                                         ZConfServiceBrowserPrivate::callback,
                                                         this);
    });
}

/*!
    Destroys the browser object and releases all resources associated with it.
 */
ZConfServiceBrowser::~ZConfServiceBrowser()
{
    if(nullptr != d_ptr->browser)
    {
        avahi_service_browser_free(d_ptr->browser);
    }
    delete d_ptr;
}

namespace
{
    static AvahiProtocol convertProtocol(ZConfServiceBrowser::Protocol proto)
    {
        switch(proto)
        {
            case ZConfServiceBrowser::ZCONF_IPV4: return AVAHI_PROTO_INET;
            case ZConfServiceBrowser::ZCONF_IPV6: return AVAHI_PROTO_INET6;
            default:                              return AVAHI_PROTO_UNSPEC;
        }
    }
}

/*!
    Browses for Zeroconf services on the LAN. This is a non-blocking call.
    ZConfServiceBrowser will emit serviceEntryAdded() when a new service is
    discovered and serviceEntryRemoved() when a service is removed from the
    network.
 */
void ZConfServiceBrowser::browse(const QString & serviceType, Protocol proto)
{
    d_ptr->type  = serviceType;
    d_ptr->proto = convertProtocol(proto);
    assert(nullptr != d_ptr->client);
    d_ptr->client->run();
}

/*!
    Returns a ZConfServiceEntry struct with detailed information about the
    Zeroconf service associated with the name.
 */
const ZConfServiceEntry& ZConfServiceBrowser::serviceEntry(const QString & name) const
{
    return d_ptr->entries[name];
}

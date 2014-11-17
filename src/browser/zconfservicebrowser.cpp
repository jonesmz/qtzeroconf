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

/*!
    A human-readable string representation of the network layer protocol used
    by this service. Possible values are "IPv4", "IPv6", and "Unspecified".
 */
QString ZConfServiceEntry::protocolName() const
{
    switch (protocol)
    {
    case AVAHI_PROTO_INET:  return QLatin1String("IPv4");
    case AVAHI_PROTO_INET6: return QLatin1String("IPv6");
    default:                return QLatin1String("Unspecified");
    }
}

static QStringMap avahiStrlstToQMap(const AvahiStringList * txt)
{
    QStringMap returnMap;
    while(nullptr != txt)
    {
        const QStringList & split = QString::fromLocal8Bit(reinterpret_cast<const char*>(&(txt->text[0])),
                                                                                         txt->size).split('=');
        returnMap.insert(split.first(), split.last());
        txt = txt->next;
    }
    return returnMap;
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
    ZConfServiceBrowserPrivate(ZConfServiceClient *in_client)
        : client(in_client), browser(0)
    {
    }

    static void callback(AvahiServiceBrowser     *browser,
                         AvahiIfIndex             interface,
                         AvahiProtocol            protocol,
                         AvahiBrowserEvent        event,
                         const char              *name,
                         const char              *type,
                         const char              *domain,
                         AvahiLookupResultFlags   flags,
                         void                    *userdata)
    {
        Q_UNUSED(browser);
        Q_UNUSED(flags);

        ZConfServiceBrowser *serviceBrowser = static_cast<ZConfServiceBrowser *>(userdata);
        if (serviceBrowser)
        {
            switch (event)
            {
            case AVAHI_BROWSER_FAILURE:
                qDebug() << (QLatin1String("Avahi browser error: ") % QString(avahi_strerror(avahi_client_errno(serviceBrowser->d_ptr->client->client))));
                break;
            case AVAHI_BROWSER_NEW:
                qDebug() << (QLatin1String("New service '") % QString(name) % QLatin1String("' of type ") % QString(type) % QLatin1String(" in domain ") % QString(domain) % QLatin1String("."));

                // We ignore the returned resolver object. In the callback
                // function we free it. If the server is terminated before
                // the callback function is called the server will free
                // the resolver for us.
                if (!(avahi_service_resolver_new(serviceBrowser->d_ptr->client->client,
                                                 interface,
                                                 protocol,
                                                 name,
                                                 serviceBrowser->d_ptr->type.toLatin1().data(),
                                                 domain,
                                                 AVAHI_PROTO_UNSPEC,
                                                 (AvahiLookupFlags) 0,
                                                 ZConfServiceBrowserPrivate::resolve,
                                                 serviceBrowser)))
                    qDebug() << (QLatin1String("Failed to resolve service '") % QString(name) % QLatin1String("': ") % avahi_strerror(avahi_client_errno(serviceBrowser->d_ptr->client->client)));
                break;
            case AVAHI_BROWSER_REMOVE:
                emit serviceBrowser->serviceEntryRemoved(name);
                serviceBrowser->d_ptr->entries.remove(name);
                qDebug() << QLatin1String("Service '") % QString(name) % QLatin1String("' removed from the network.");
                break;
            case AVAHI_BROWSER_ALL_FOR_NOW:
            case AVAHI_BROWSER_CACHE_EXHAUSTED:
                qDebug() << (AVAHI_BROWSER_ALL_FOR_NOW == event
                             ? QLatin1String("AVAHI_BROWSER_ALL_FOR_NOW")
                             : QLatin1String("AVAHI_BROWSER_CACHE_EXHAUSTED"));
            } // end switch
        }
    }

    static void resolve(AvahiServiceResolver   *resolver,
                        AvahiIfIndex            interface,
                        AvahiProtocol           protocol,
                        AvahiResolverEvent      event,
                        const char             *name,
                        const char             *type,
                        const char             *domain,
                        const char             *host_name,
                        const AvahiAddress     *address,
                        uint16_t                port,
                        AvahiStringList        *txt,
                        AvahiLookupResultFlags  flags,
                        void                   *userdata)
    {
        Q_UNUSED(txt);

        ZConfServiceBrowser *serviceBrowser = static_cast<ZConfServiceBrowser *>(userdata);
        if (serviceBrowser)
        {
            switch (event)
            {
                case AVAHI_RESOLVER_FAILURE:
                    qDebug() << (QLatin1String("Failed to resolve service '") % QString(name) % QLatin1String("': ") % avahi_strerror(avahi_client_errno(serviceBrowser->d_ptr->client->client)));
                    break;
                case AVAHI_RESOLVER_FOUND:
                {
                    char a[AVAHI_ADDRESS_STR_MAX];
                    avahi_address_snprint(a, sizeof(a), address);
                    ZConfServiceEntry entry;
                    entry.interface  = interface;
                    entry.ip         = QString(a);
                    entry.type       = QString(type);
                    entry.domain     = domain;
                    entry.host       = host_name;
                    entry.port       = port;
                    entry.protocol   = protocol;
                    entry.flags      = flags;
                    entry.TXTRecords = avahiStrlstToQMap(txt);
                    serviceBrowser->d_ptr->entries.insert(name, entry);
                    emit serviceBrowser->serviceEntryAdded(name);
                }
            }
            avahi_service_resolver_free(resolver);
        }
    }

    typedef QHash<QString, ZConfServiceEntry> ZConfServiceEntryTable;

    ZConfServiceClient      *const client;
    AvahiServiceBrowser     *browser;
    ZConfServiceEntryTable   entries;
    QString                  type;
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
        if (this->d_ptr->browser)
        {
            return;
        }
        this->d_ptr->browser = avahi_service_browser_new(d_ptr->client->client,
                                                   AVAHI_IF_UNSPEC,
                                                   AVAHI_PROTO_UNSPEC,
                                                   d_ptr->type.toLatin1().data(),
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
    if (d_ptr->browser)
    {
        avahi_service_browser_free(d_ptr->browser);
    }
    delete d_ptr;
}

/*!
    Browses for Zeroconf services on the LAN. This is a non-blocking call.
    ZConfServiceBrowser will emit serviceEntryAdded() when a new service is
    discovered and serviceEntryRemoved() when a service is removed from the
    network.
 */
void ZConfServiceBrowser::browse(QString serviceType)
{
    d_ptr->type = serviceType;
    assert(d_ptr->client);
    d_ptr->client->run();
}

/*!
    Returns a ZConfServiceEntry struct with detailed information about the
    Zeroconf service associated with the name.
 */
ZConfServiceEntry ZConfServiceBrowser::serviceEntry(QString name)
{
    return d_ptr->entries.value(name);
}

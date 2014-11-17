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

#ifndef ZCONFSERVICEBROWSER_H
#define ZCONFSERVICEBROWSER_H

#include <stdint.h>
#include <avahi-client/lookup.h>

#include <QMap>
#include <QObject>

typedef QMap<QString, QString> QStringMap;

struct ZConfServiceEntry
{
    AvahiIfIndex           interface;
    QString                ip;
    QString                domain;
    QString                type;
    QString                host;
    uint16_t               port;
    AvahiProtocol          protocol;
    AvahiLookupResultFlags flags;
    QStringMap             TXTRecords;

    QString protocolName()    const;
    inline bool isValid()     const { return !(ip.isEmpty() && host.isEmpty()); }
    inline bool isLocal()     const { return flags & AVAHI_LOOKUP_RESULT_LOCAL; }
    inline bool isCached()    const { return flags & AVAHI_LOOKUP_RESULT_CACHED; }
    inline bool isWideArea()  const { return flags & AVAHI_LOOKUP_RESULT_WIDE_AREA; }
    inline bool isMulticast() const { return flags & AVAHI_LOOKUP_RESULT_MULTICAST; }

};

class ZConfServiceBrowserPrivate;
class ZConfServiceBrowser : public QObject
{
    Q_OBJECT

public:
    enum Protocol
    {
        ZCONF_IPV4,
        ZCONF_IPV6,
        ZCONF_UNSPEC
    };

    explicit ZConfServiceBrowser(QObject *parent = 0);
    ~ZConfServiceBrowser();

    void browse(const QString & serviceType = QLatin1String("_http._tcp"), Protocol proto = ZCONF_UNSPEC);
    const ZConfServiceEntry& serviceEntry(const QString & name) const;

signals:
    void serviceEntryAdded(const QString &) const;
    void serviceEntryRemoved(const QString &) const;

protected:
    ZConfServiceBrowserPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(ZConfServiceBrowser);
};

#endif // ZCONFSERVICEBROWSER_H

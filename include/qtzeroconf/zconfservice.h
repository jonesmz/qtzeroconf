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

#ifndef ZCONFSERVICE_H
#define ZCONFSERVICE_H

#include <arpa/inet.h>

#include <QMap>
#include <QObject>

typedef QMap<QString, QString> QStringMap;

class ZConfServicePrivate;
class ZConfService : public QObject
{
    Q_OBJECT

public:
    explicit ZConfService(QObject * parent = nullptr);
    ~ZConfService();

    bool isValid() const;
    QString errorString() const;

signals:
    void entryGroupFailure()       const;
    void entryGroupEstablished()   const;
    void entryGroupNameCollision() const;

public slots:
    void registerService(const QString & name,
                         in_port_t port,
                         const QString & type = QLatin1String("_http._tcp"),
                         const QStringMap & txtRecords = QStringMap());
    void resetService();

protected:
    ZConfServicePrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(ZConfService)
};

#endif // ZCONFSERVICE_H

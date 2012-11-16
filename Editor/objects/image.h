/* Copyright (C) 2012 Carlos Pais 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <QFile>
#include <QPixmap>

#include "image.h"
#include "object.h"


class Image : public Object
{
    Q_OBJECT

    QPixmap *mImage;

public:
    explicit Image(QPixmap* image=0, QObject *parent = 0);
    Image(const QString&path, QObject* parent=0);
    Image(const QVariantMap&, QObject*);
    virtual ~Image();
    virtual void paint(QPainter &);
    virtual void setImage(const QString&, bool deletePrevious=true);
    virtual void setImage(QPixmap*, bool deletePrevious=true);
    QPixmap* image() const;
    virtual QPixmap* image(const QString&) const;
    virtual QVariantMap toJsonObject();

signals:

public slots:

private:
    void init();

};

#endif // IMAGE_H

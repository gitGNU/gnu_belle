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

#ifndef GROUP_WIDGET
#define GROUP_WIDGET

#include <QFrame>
#include <QWidget>
#include <QLayout>
#include <QSizePolicy>
#include <QHash>

//#include "boxlayout.h"

class WidgetGroup : public QFrame
{
    QBoxLayout *mMainLayout;
    QBoxLayout *mCurrentLayout;
    Qt::Orientation mCurrentOrientation;
    QList<QBoxLayout*> mLayouts;
    QList<QBoxLayout*> mUsedLayouts;
    QList<QBoxLayout*> mUsedMainLayouts;
    QList<Qt::Orientation> mOrientations;
    QList<Qt::Orientation> mUsedOrientations;
    Qt::Orientation mOrientation;
    //QHash<QBoxLayout*, QList<QWidget*> > mWidgetsInLayouts;

public:
    WidgetGroup(Qt::Orientation orientation, QWidget* parent=0);
    ~WidgetGroup();
    void addWidget(QWidget* widget, bool usePassedSizePolicy = false, QSizePolicy::Policy hPolicy = QSizePolicy::Fixed, QSizePolicy::Policy vPolicy = QSizePolicy::Fixed);
    Qt::Orientation orientation();
    void beginLayout(Qt::Orientation);
    void endLayout();
    void addWidgets(const QList<QWidget*>& widgets, Qt::Orientation orientation);
    void removeLastMainLayout();
    void removeWidgetsFromLastLayout();
    void setLastLayoutAsCurrent();
    void deleteAllWidgets(QBoxLayout*);
    void removeLayoutWithWidget(QWidget* widget);
protected:
    QBoxLayout *layoutForOrientation(Qt::Orientation);
    QList<QWidget*> widgetsInLayout(QBoxLayout *);
};



#endif

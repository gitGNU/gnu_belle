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

#include "widget_group.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QDebug>

WidgetGroup::WidgetGroup(Qt::Orientation orientation, QWidget* parent) :
    QFrame(parent)
{
    mMainLayout = layoutForOrientation(orientation);
    this->setLayout(mMainLayout);
    mCurrentLayout = mMainLayout;
    mOrientation = orientation;
    mCurrentOrientation = mOrientation;

    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    this->setFrameShadow(QFrame::Plain);
}

WidgetGroup::~WidgetGroup()
{
}

QBoxLayout* WidgetGroup::layoutForOrientation(Qt::Orientation orientation)
{
    QBoxLayout *layout;

    if ( orientation == Qt::Horizontal )
        layout = new QHBoxLayout();
    else
        layout = new QVBoxLayout();

    return  layout;
}

void WidgetGroup::addWidget(QWidget* widget, bool usePassedSizePolicy, QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy)
{
    if(! widget )
        return;

    if (usePassedSizePolicy) {
        widget->setSizePolicy(hPolicy, vPolicy);
    }
    else {
        if (mCurrentOrientation == Qt::Vertical)
            widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        else
            widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    }

    mCurrentLayout->addWidget(widget);
    //if (! mWidgetsInLayouts.contains(mCurrentLayout))
    //    mWidgetsInLayouts[mCurrentLayout] = QList<QWidget*>();
    //mWidgetsInLayouts[mCurrentLayout].append(widget);
}

void WidgetGroup::beginLayout(Qt::Orientation orientation)
{

    QBoxLayout *prevLayout;
    if (! mLayouts.isEmpty())
        prevLayout = mLayouts.last();
    else
        prevLayout = mMainLayout;

    mCurrentLayout = layoutForOrientation(orientation);
    prevLayout->addLayout(mCurrentLayout);

    if (prevLayout == mMainLayout)
        mUsedMainLayouts.append(mCurrentLayout);

    mLayouts.append(mCurrentLayout);
    mUsedLayouts.append(mCurrentLayout);
    mOrientations.append(orientation);
    mUsedOrientations.append(orientation);
}

void WidgetGroup::endLayout()
{
    if (! mLayouts.isEmpty()) {
        mLayouts.removeLast();
        mOrientations.removeLast();
    }

    if (! mLayouts.isEmpty()) {
        mCurrentLayout = mLayouts.last();
        mCurrentOrientation = mOrientations.last();
    }
    else {
         mCurrentLayout = mMainLayout;
         mCurrentOrientation = mOrientation;
    }

}

void WidgetGroup::addWidgets(const QList<QWidget*>& widgets, Qt::Orientation orientation)
{
    beginLayout(orientation);
    for(int i=0; i < widgets.size(); i++)
        addWidget(widgets[i]);
    endLayout();
}

Qt::Orientation WidgetGroup::orientation()
{
    return mOrientation;
}

void WidgetGroup::removeWidgetsFromLastLayout()
{
    if (mUsedLayouts.isEmpty())
        return;

    QBoxLayout * layout= mUsedLayouts.last();

    for(int i=layout->count()-1; i >= 0; --i) {
        //delete layout->itemAt(i);
        layout->removeItem(layout->itemAt(i));
    }

}

void WidgetGroup::removeLastMainLayout()
{
    if (mMainLayout->count() > 0) {

        if (! mUsedMainLayouts.isEmpty())
            deleteAllWidgets(mUsedMainLayouts.takeLast());

        QLayoutItem * item = mMainLayout->itemAt(mMainLayout->count()-1);
        mMainLayout->removeItem(item);
        delete item;
    }
}

QList<QWidget*> WidgetGroup::widgetsInLayout(QBoxLayout *layout)
{
    QList<QWidget*> widgets;
    QLayoutItem * item;
    for(int i=layout->count()-1; i >= 0; i-- ){

        item = layout->takeAt(i);
        if (item && item->widget())
            widgets.append(item->widget());
    }

    return widgets;

}

void WidgetGroup::setLastLayoutAsCurrent()
{
    if (! mUsedLayouts.isEmpty()) {
        mCurrentLayout = mUsedLayouts.last();
        mCurrentOrientation = mUsedOrientations.last();
    }
}

void WidgetGroup::removeLayoutWithWidget(QWidget* widget)
{
    for(int i=0; i < mUsedMainLayouts.size(); i++) {
        if(mUsedMainLayouts[i]->indexOf(widget) != -1) {
            deleteAllWidgets(mUsedMainLayouts[i]);
            delete mUsedMainLayouts.takeAt(i);
            break;
        }
    }
}

void WidgetGroup::deleteAllWidgets(QBoxLayout* layout)
{
    QLayoutItem *item;
    for(int i=layout->count()-1; i >= 0; i-- ){
        item = layout->takeAt(i);
        if (item ) {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
    }
}

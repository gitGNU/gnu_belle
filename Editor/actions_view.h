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

#ifndef ACTIONS_VIEW_H
#define ACTIONS_VIEW_H

#include <QListView>
#include <QStyledItemDelegate>

#include "action.h"

#define BORDER 2
#define ITEM_SELECTED_LINE_WIDTH 4

class ActionsViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ActionsViewDelegate(QObject* parent = 0);
    virtual void paint (QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class ActionsView : public QListView
{
    Q_OBJECT
public:
    explicit ActionsView(QWidget *parent = 0);
    void appendAction(Action*);


protected:
    virtual void dropEvent(QDropEvent *);

signals:

private slots:
    void onContextMenuRequested(const QPoint&);
    void onDeleteAction();
    void onItemClicked(const QModelIndex&);

private:
    QAction* mDeleteAction;
    QAction* mCopyAction;
    QAction* mCutAction;
    QAction* mPasteAction;
};

#endif // ACTIONS_VIEW_H

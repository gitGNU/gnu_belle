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

#ifndef SHOW_MENU_H
#define SHOW_MENU_H

#include "action.h"
#include "show_menu_editorwidget.h"

class ShowMenuEditorWidget;

class ShowMenu : public Action
{
    Q_OBJECT
public:
    static ActionInfo Info;

public:
    explicit ShowMenu(QObject *parent = 0);
     ShowMenu(const QVariantMap&, QObject *parent = 0);

     static ShowMenuEditorWidget* showMenuEditorWidget();
     static void setShowMenuEditorWidget(ShowMenuEditorWidget*);
     virtual ActionEditorWidget* editorWidget();
     virtual QVariantMap toJsonObject();

     void focusIn();
     void focusOut();

signals:

public slots:

private:
    void init();

};

#endif // SHOW_MENU_H

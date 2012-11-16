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

#ifndef HIDE_H
#define HIDE_H

#include "change_visibility.h"
#include "hide_editor_widget.h"

class Hide : public ChangeVisibility
{
    Q_OBJECT

public:
    static ActionInfo Info;

public:
    explicit Hide(QObject *parent = 0);
    Hide(const QVariantMap& data, QObject *parent = 0);
    static HideEditorWidget* hideEditorWidget();
    static void setHideEditorWidget(HideEditorWidget*);
    virtual ActionEditorWidget* editorWidget();
    virtual QString displayText() const;

private:
    void init();

};


#endif // HIDE_H

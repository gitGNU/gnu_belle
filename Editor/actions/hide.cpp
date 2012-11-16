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

#include "hide.h"

static HideEditorWidget* mHideEditorWidget = 0;
ActionInfo Hide::Info;

Hide::Hide(QObject *parent):
    ChangeVisibility(false, parent)
{
    init();
}

Hide::Hide(const QVariantMap &data, QObject *parent):
    ChangeVisibility(data, parent)
{
    init();
}

void Hide::init()
{
    setType(Info.type);
    setName(Info.name);
    setIcon(Info.icon);
}

void Hide::setHideEditorWidget(HideEditorWidget * widget)
{
   mHideEditorWidget = widget;
}

HideEditorWidget* Hide::hideEditorWidget()
{
   return mHideEditorWidget;
}

ActionEditorWidget* Hide::editorWidget()
{
    return mHideEditorWidget;
}

QString Hide::displayText() const
{
    if (sceneObject())
        return sceneObject()->objectName();

    return "";
}

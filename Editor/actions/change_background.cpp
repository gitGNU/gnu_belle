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

#include "change_background.h"
#include "scene_manager.h"

#include <QFileInfo>
#include <QtDebug>

static ChangeBackgroundEditorWidget* mEditorWidget = 0;
ActionInfo ChangeBackground::Info;

ChangeBackground::ChangeBackground(QObject *parent) :
    Action(parent)
{
    init();
}

ChangeBackground::ChangeBackground(const QVariantMap & data, QObject *parent):
    Action(data, parent)
{
    init();
    if (data.contains("background") && data.value("background").type() == QVariant::String) {
        setBackground(data.value("background").toString());
    }
}

void ChangeBackground::init()
{
    setIcon(Info.icon);
    setName(Info.name);
    setType(Info.type);

    mBackground  = "";
    mCurrentSceneBackground = 0;
}

void ChangeBackground::setChangeBackgroundEditorWidget(ChangeBackgroundEditorWidget * widget)
{
   mEditorWidget = widget;
}

ChangeBackgroundEditorWidget* ChangeBackground::changeBackgroundEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* ChangeBackground::editorWidget()
{
    return mEditorWidget;
}

void ChangeBackground::setBackground(const QString & background)
{
    mBackground = background;
    QFileInfo info(mBackground);
    setDisplayText(info.fileName());
    if (SceneManager::currentScene()) {
        mCurrentSceneBackground = SceneManager::currentScene()->backgroundImage();
        SceneManager::currentScene()->setBackground(mBackground);
    }
    emit dataChanged();
}

QString ChangeBackground::background()
{
    return mBackground;
}

void ChangeBackground::focusIn()
{
    if (SceneManager::currentScene()) {
        mCurrentSceneBackground = SceneManager::currentScene()->backgroundImage();
        SceneManager::currentScene()->setBackground(mBackground);
    }
}

void ChangeBackground::focusOut()
{
    if (SceneManager::currentScene())
        SceneManager::currentScene()->setBackgroundImage(mCurrentSceneBackground);

    mCurrentSceneBackground = 0;
}

QVariantMap ChangeBackground::toJsonObject()
{
    QVariantMap data = Action::toJsonObject();

    if (! mBackground.isEmpty())
        data.insert("background", mBackground);

    return data;
}

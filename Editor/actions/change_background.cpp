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
#include "resource_manager.h"
#include "utils.h"

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
    if (data.contains("backgroundImage") && data.value("backgroundImage").type() == QVariant::String)
        setBackgroundImage(data.value("backgroundImage").toString());

    if (data.contains("backgroundColor") && data.value("backgroundColor").type() == QVariant::List)
        setBackgroundColor(Utils::listToColor(data.value("backgroundColor").toList()));

}

void ChangeBackground::init()
{
    setIcon(Info.icon);
    setName(Info.name);
    setType(Info.type);

    mCurrentSceneBackgroundImage = 0;
    mBackgroundImage = 0;
    mBackgroundImageChanged = false;
    mBackgroundColorChanged = false;
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

void ChangeBackground::setBackgroundImage(const QString & background)
{
    ResourceManager::decrementReference(mBackgroundImage);
    mBackgroundImage = ResourceManager::newImage(background);
    QFileInfo info(background);
    setDisplayText(info.fileName());
    emit dataChanged();
}

QString ChangeBackground::backgroundPath()
{
    return ResourceManager::imagePath(mBackgroundImage);
}

void ChangeBackground::setBackgroundColor(const QColor& color)
{
    mBackgroundColor = color;
    setDisplayText(color.name());
    emit dataChanged();
}

QColor ChangeBackground::backgroundColor()
{
    return mBackgroundColor;
}

void ChangeBackground::focusIn()
{
    Action::focusIn();
    Scene *scene = this->scene();

    if (scene) {
        if (mBackgroundImage || mBackgroundColor.isValid()) {
            mCurrentSceneBackgroundImage = scene->backgroundImage();
            scene->setBackgroundImage(mBackgroundImage);
            mBackgroundImageChanged = true;
        }

        if (mBackgroundColor.isValid()) {
            mBackgroundColorChanged = true;
            mCurrentSceneBackgroundColor = scene->backgroundColor();
            scene->setBackgroundColor(mBackgroundColor);
        }
    }
}

void ChangeBackground::focusOut()
{
    Action::focusOut();
    Scene* scene = this->scene();

    if (scene) {
        if (mBackgroundImageChanged)
            scene->setBackgroundImage(mCurrentSceneBackgroundImage);
        if (mBackgroundColorChanged)
            scene->setBackgroundColor(mCurrentSceneBackgroundColor);
    }

    mCurrentSceneBackgroundColor = QColor();
    mCurrentSceneBackgroundImage = 0;
    mBackgroundImageChanged = false;
    mBackgroundColorChanged = false;
}

QVariantMap ChangeBackground::toJsonObject()
{
    QVariantMap data = Action::toJsonObject();
    QString path = ResourceManager::imagePath(mBackgroundImage);

    if (! path.isEmpty())
        data.insert("backgroundImage", QFileInfo(path).fileName());
    if (mBackgroundColor.isValid())
        data.insert("backgroundColor", Utils::colorToList(mBackgroundColor));

    return data;
}

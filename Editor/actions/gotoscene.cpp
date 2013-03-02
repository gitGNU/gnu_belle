/* Copyright (C) 2012, 2013 Carlos Pais 
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

#include "gotoscene.h"

#include <QDebug>

#include "scene_manager.h"

static GoToSceneEditorWidget* mEditorWidget = 0;
ActionInfo GoToScene::Info;

GoToScene::GoToScene(QObject *parent) :
    Action(parent)
{
    init();
}

GoToScene::GoToScene(const QVariantMap& data, QObject *parent) :
    Action(data, parent)
{
    init();

    if (data.contains("scene") && data.value("scene").type() == QVariant::String) {
        mTargetSceneName = data.value("scene").toString();
        setDisplayText(mTargetSceneName);
    }
}

void GoToScene::init()
{
    setType(Info.type);
    setName(Info.name);
    setIcon(Info.icon);
    setSupportedEvents(Interaction::MousePress | Interaction::MouseRelease);
    mTargetSceneName = "";
}

void GoToScene::setGoToSceneEditorWidget(GoToSceneEditorWidget * widget)
{
   mEditorWidget = widget;
}

GoToSceneEditorWidget* GoToScene::goToSceneEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* GoToScene::editorWidget()
{
    return mEditorWidget;
}

void GoToScene::setTargetSceneName(const QString & name)
{
    QList<Scene*> scenes = SceneManager::scenes();
    for(int i=0; i < scenes.size(); i++) {
        if (scenes[i] && scenes[i]->objectName() == name) {
            mTargetSceneName = name;
            setDisplayText(name);
            break;
        }
    }
}


QString GoToScene::targetSceneName()
{
    return mTargetSceneName;
}

QVariantMap GoToScene::toJsonObject()
{
    QVariantMap action = Action::toJsonObject();
    action.insert("scene", mTargetSceneName);

    return action;
}

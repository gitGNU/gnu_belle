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

#include "change_visibility_editor_widget.h"

#include <QHash>
#include <QHashIterator>
#include <QDebug>

#include "scene.h"
#include "fade.h"
#include "slide.h"

ChangeVisibilityEditorWidget::ChangeVisibilityEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    init();
}

void ChangeVisibilityEditorWidget::init()
{
    mCurrentAction = 0;
    mObjectsWidget = new QComboBox(this);
    connect(mObjectsWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onObjectChanged(int)));

    beginGroup(tr("Change Object's Visibility"));
    appendRow(tr("Object"), mObjectsWidget);
    endGroup();

    mFadeEditorWidget = new FadeEditorWidget();
    mSlideEditorWidget = new SlideEditorWidget;
    append(mFadeEditorWidget, 0, QStringList() << "Action" << "Object");
    append(mSlideEditorWidget, 0, QStringList() << "Action" << "Object");

    resizeColumnToContents(0);
}

ChangeVisibilityEditorWidget::~ChangeVisibilityEditorWidget()
{
    mFadeEditorWidget->deleteLater();
    mSlideEditorWidget->deleteLater();
}

void ChangeVisibilityEditorWidget::updateData(Action * action)
{
    mCurrentAction = qobject_cast<ChangeVisibility*>(action);
    if (! mCurrentAction)
        return;

    mFadeEditorWidget->updateData(mCurrentAction->fadeAction());
    mSlideEditorWidget->updateData(mCurrentAction->slideAction());

    setGroupName(mCurrentAction->name());
    mObjectsWidget->clear();

    mObjects = findObjects(! mCurrentAction->toShow());

    mObjectsWidget->blockSignals(true);

    for (int i=0; i < mObjects.size(); i++)
        mObjectsWidget->addItem(mObjects[i]->objectName());
    mObjectsWidget->blockSignals(false);

    mObjectsWidget->setCurrentIndex(0);
    if (! mCurrentAction->sceneObject() && mObjects.size()) {
        mCurrentAction->setSceneObject(mObjects[0]);
        //if (lastChangeVisibilityActionForObject(mObjects[0]) == mCurrentAction)
        //    mObjects[0]->setAvailable(! mObjects[0]->isAvailable());
    }

}

void ChangeVisibilityEditorWidget::onObjectChanged(int index)
{
    if (! mCurrentAction || index >= mObjects.size() || index < 0)
        return;

    /*if (mCurrentAction->sceneObject() && lastChangeVisibilityActionForObject(mCurrentAction->character()) == mCurrentAction) {
        mCurrentAction->character()->setAvailable(! mCurrentAction->character()->isAvailable());
    }*/

    mCurrentAction->setSceneObject(mObjects[index]);
    /*if (lastChangeVisibilityActionForObject(mCurrentAction->sceneObject()) == mCurrentAction) {
        mCurrentAction->character()->setAvailable(! mCurrentAction->character()->isAvailable());
    }*/

    emit objectChanged(mCurrentAction->sceneObject());
}


Action* ChangeVisibilityEditorWidget::lastChangeVisibilityActionForObject(Object * obj)
{
    if (! mCurrentAction)
        return 0;

    Scene *scene = qobject_cast<Scene*>(mCurrentAction->parent());
    if (! scene)
        return 0;

    QList<Action*> actions = scene->actions();
    Action * lastAction = 0;
    for(int i=0; i < actions.size(); i++) {
        if (actions[i]->sceneObject() == obj && qobject_cast<ChangeVisibility*>(actions[i])) {
            lastAction = actions[i];
        }
    }

    return lastAction;
}

ChangeVisibility* ChangeVisibilityEditorWidget::currentAction()
{
    return mCurrentAction;
}

QList<Object*> ChangeVisibilityEditorWidget::findObjects(bool shown)
{
    Scene* scene = 0;
    QList<Object*> objects;

    if (! mCurrentAction)
        return objects;

    scene = mCurrentAction->scene();
    if (! scene)
        return objects;

    //mObjectInitialStateWidget->clear();

    //if (mCurrentAction->sceneObject()) {
    //    objects.append(mCurrentAction->sceneObject());
        //mObjectInitialStateWidget->addItems(mCurrentAction->character()->statesToPaths().keys());
        //mObjectInitialStateWidget->setDisabled(false);
   // }
    /*else {
        mObjectInitialStateWidget->setDisabled(true);
    }*/

    objects = scene->objects();
    QHash<Object*, bool> objectsVisibility;
    Object* object = 0;
    for (int i=0; i < objects.size(); i++) {
        object = objects[i];
        if (object)
            objectsVisibility.insert(object, object->visible());
    }

    QList<Action*> actions = scene->actions();
    ChangeVisibility* changeVisibility = 0;
    for(int i=0; i < actions.size(); i++) {
        object = actions[i]->sceneObject();
        if (! object)
            continue;

        changeVisibility = qobject_cast<ChangeVisibility*>(actions[i]);
        if (! changeVisibility)
            continue;

        objectsVisibility.insert(object, changeVisibility->toShow());
        if (actions[i] == mCurrentAction)
            break;
        //if (changeVisibility->toShow() != shown)
        //    objects.removeOne(obj);
    }

    objects.clear();
    if (mCurrentAction->sceneObject())
        objects.append(mCurrentAction->sceneObject());
    QHashIterator<Object*, bool> it(objectsVisibility);
    while(it.hasNext()) {
        it.next();
        if (it.value() == shown)
            objects.append(it.key());
    }

    return objects;
}


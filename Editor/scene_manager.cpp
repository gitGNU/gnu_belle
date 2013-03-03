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

#include "scene_manager.h"

#include <QDebug>

#include "utils.h"

static QSize mSceneSize;
static int mCurrentSceneIndex = -1;
static QList<Scene*> mScenes = QList<Scene*>();
static Clipboard *mClipboard = 0;
static SceneManager* mInstance = 0;

SceneManager::SceneManager(QObject * parent) :
    QObject(parent)
{
    mSrcScene = 0;
    connect(this, SIGNAL(currentSceneChanged()), this, SLOT(onCurrentSceneChanged()));
}

SceneManager::SceneManager(int width, int height, QObject * parent) :
    QObject(parent)
{
    Scene::setWidth(width);
    Scene::setHeight(height);
    mCurrentSceneIndex = -1;
    connect(this, SIGNAL(currentSceneChanged()), this, SLOT(onCurrentSceneChanged()));
}


SceneManager::~SceneManager()
{
    for(int i=0; i < mScenesTrash.size(); i++)
        if(mScenesTrash[i])
            mScenesTrash[i]->deleteLater();
    mScenesTrash.clear();
}

void SceneManager::addScene(Scene * scene)
{
    insertScene(mScenes.size(), scene);
}

void SceneManager::insertSceneAfter(Scene * afterScene, Scene* scene)
{
    int index = mScenes.indexOf(afterScene);
    if (index != -1)
        insertScene(index, scene);
}

void SceneManager::insertScene(int index, Scene * scene)
{
    if (! scene)
        return;

    if (mScenes.contains(scene)) {
        /*if (index > mScenes.indexOf(scene))
            index--;*/
        mScenes.removeAt(mScenes.indexOf(scene));
    }
    else {
        connect(scene, SIGNAL(dataChanged()), this, SIGNAL(updateDrawingSurfaceWidget()));
        connect(scene, SIGNAL(selectionChanged(Object*)), this, SIGNAL(selectionChanged(Object*)));
    }

    mCurrentSceneIndex = index;
    mScenes.insert(index, scene);
}

Scene* SceneManager::createNewScene(const QString& name)
{
    mCurrentSceneIndex = mScenes.size();
    Scene* scene = new Scene(this, name);
    addScene(scene);
    return scene;
}

void SceneManager::removeScene(Scene* scene, bool del)
{
    removeSceneAt(mScenes.indexOf(scene), del);
}

void SceneManager::removeSceneAt(int i, bool del)
{
    if (i >= 0 && i < mScenes.size()) {
        if (mScenes.count(mScenes[i]) == 1)
            mScenes[i]->disconnect(this);
        Scene * scene = mScenes.takeAt(i);
        emit sceneRemoved(i);
        if (mScenes.size() == 0)
            mCurrentSceneIndex = -1;

        if (del && scene)
            scene->deleteLater();
    }

}

void SceneManager::removeScenes(bool del)
{
    for (int i=mScenes.size()-1; i >= 0; i--)
        removeSceneAt(i, del);

    mCurrentSceneIndex = -1;
}

int SceneManager::currentSceneIndex()
{
    return mCurrentSceneIndex;
}

void SceneManager::setCurrentSceneIndex(int index)
{
    if (index >= mScenes.size() || index < 0 || index == mCurrentSceneIndex)
        return;

    mCurrentSceneIndex = index;
    emit currentSceneChanged();
}

void SceneManager::setCurrentScene(Scene* scene)
{
    if (scene && mScenes.contains(scene))
        setCurrentSceneIndex(mScenes.indexOf(scene));
}

Scene* SceneManager::currentScene()
{
    if (mCurrentSceneIndex >= 0 && mCurrentSceneIndex < mScenes.size())
        return mScenes[mCurrentSceneIndex];
    return 0;
}

int SceneManager::size()
{
    return mScenes.size();
}

int SceneManager::count()
{
    return mScenes.size();
}

Scene* SceneManager::at(int i)
{
    return mScenes[i];
}

void SceneManager::setClipboard(Clipboard* clipboard)
{
    mClipboard = clipboard;
}

Clipboard* SceneManager::clipboard()
{
    return mClipboard;
}

void SceneManager::onCurrentSceneChanged()
{
    if (currentScene())
        currentScene()->selectObject(currentScene()->selectedObject());
}

Scene* SceneManager::scene(int i)
{
    if (i >= 0 && i < mScenes.size())
        return mScenes[i];

    return 0;
}

Scene* SceneManager::srcScene()
{
    return mSrcScene;
}

void SceneManager::setSrcScene(Scene * scene)
{
    mSrcScene = scene;
}

int SceneManager::indexOf(Scene* scene)
{
    return mScenes.indexOf(scene);
}

bool SceneManager::contains(const QString& name)
{
    foreach(Scene* scene, mScenes)
        if (scene->objectName() == name)
            return true;
    return false;
}

bool SceneManager::isValidSceneName(const QString& name)
{
    if (name.isEmpty() || name.isNull())
        return false;

    return ! contains(name);
}

QString SceneManager::validSceneName(QString name)
{
    if (name.isEmpty() || name.isNull())
        name = tr("scene");

    while(! isValidSceneName(name))
        name = Utils::incrementLastNumber(name);

    return name;
}

QList<Scene*> SceneManager::scenes()
{
    return mScenes;
}

void SceneManager::setInstance(SceneManager * instance)
{
    mInstance = instance;
}

SceneManager* SceneManager::instance()
{
    return mInstance;
}

void SceneManager::setSceneWidth(int width)
{
    Scene::setWidth(width);
    mSceneSize.setWidth(width);
    int height = Scene::height();
    for(int i=0; i < mScenes.size(); i++) {
        mScenes[i]->resize(width, height);
    }
    emit resized(QResizeEvent(QSize(width, mSceneSize.height()), mSceneSize));
}

void SceneManager::setSceneHeight(int height)
{
    Scene::setHeight(height);
    mSceneSize.setHeight(height);
    int width = Scene::width();
    for(int i=0; i < mScenes.size(); i++)
        mScenes[i]->resize(width, height);
    emit resized(QResizeEvent(QSize(mSceneSize.width(), height), mSceneSize));
}

void SceneManager::setSceneSize(const QSize& size)
{
    mSceneSize = size;
    for(int i=0; i < mScenes.size(); i++)
        mScenes[i]->resize(size.width(), size.height());
    emit resized(QResizeEvent(size, mSceneSize));
}

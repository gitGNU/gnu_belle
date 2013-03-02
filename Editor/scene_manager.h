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

#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <QList>
#include <QResizeEvent>
#include <QObject>

#include "scene.h"
#include "clipboard.h"


class SceneManager : public QObject
{
    Q_OBJECT

    QList<Scene*> mScenesTrash;
    //QList<Object*> mObjectsClipboard;
    Scene* mSrcScene;
    
    public:
        explicit SceneManager(QObject * parent=0);
        explicit SceneManager(int width, int height, QObject * parent=0);
        ~SceneManager();
        Scene* createNewScene(const QString& name="");
        void addScene(Scene*) ;

        Object* objectAt(qreal x, qreal y);


        void onResizeEvent(QResizeEvent*);

        Scene* at(int);

        Scene* srcScene();
        void setSrcScene(Scene*);
        void removeScenes(bool del);
        void setCurrentScene(Scene*);
        void setCurrentSceneIndex(int);
        void insertSceneAfter(Scene*, Scene*);
        void deleteScene(Scene*);
        void deleteSceneAt(int);
        void removeScene(Scene*, bool del=false);
        void removeSceneAt(int, bool del=false);
        void insertScene(int, Scene*);
        void setSceneWidth(int);
        void setSceneHeight(int);
        void setSceneSize(const QSize&);

        static int count();
        static Scene* scene(int);
        static int size();
        static int indexOf(Scene*);
        static void setClipboard(Clipboard*);
        static Clipboard* clipboard();
        static Scene * currentScene();
        static int currentSceneIndex();
        static QList<Scene*> scenes();
        static QString validSceneName(QString name="");
        static bool contains(const QString& name);
        static bool isValidSceneName(const QString& name);
        static void setInstance(SceneManager*);
        static SceneManager* instance();
        
    signals:
        void resized(const QResizeEvent&);
        void updateDrawingSurfaceWidget();
        void currentSceneChanged();
        void selectionChanged(Object*);
        void sceneRemoved(int);

    private slots:
        void onCurrentSceneChanged();

};


#endif

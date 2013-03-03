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

#ifndef SCENE_H
#define SCENE_H

#include <QList>
#include <QSize>
#include <QObject>
#include <QWidget>
#include <QResizeEvent>
#include <QSize>

#include "object.h"
#include "action.h"
#include "scene_editor_widget.h"
#include "animationimage.h"

class SceneEditorWidget;

class Scene : public QObject
{
    Q_OBJECT
    
    QList<Object*> mObjects;
    QList<Object*> mTemporaryObjects;
    QList<Action*> mActions;
    Object * mSelectedObject;
    Object* mHighlightedObject;
    AnimationImage *mBackgroundImage;
    AnimationImage *mTemporaryBackgroundImage;
    QColor mBackgroundColor;
    QColor mTemporaryBackgroundColor;
    
    public:
        explicit Scene(QObject *parent = 0, const QString& name="");
        Scene(const QVariantMap& data, QObject *parent = 0);
        ~Scene();
        QList<Object*> objects();
        QList<Object*> temporaryObjects();
        Object* objectAt (qreal, qreal);
        Object* object(const QString&);
        void appendObject(Object*, bool select=true, bool temporarily=false);
        void addCopyOfObject(Object*, bool select=true);

        static int width();
        static int height();
        static QSize size();
        static void setWidth(int);
        static void setHeight(int);
        static void setSize(const QSize&);
        static QPoint point();
        static void setPoint(const QPoint&);
        static void setEditorWidget(SceneEditorWidget*);
        static SceneEditorWidget* editorWidget();

        void setTemporaryBackgroundImage(AnimationImage*);
        AnimationImage* temporaryBackgroundImage();

        void setTemporaryBackgroundColor(const QColor&);
        QColor temporaryBackgroundColor();

        void selectObjectAt(int, int);
        void moveSelectedObject(int, int);
        void stopMoveSelectedObject();
        Object *selectedObject();
        void setBackgroundImage(const QString&);
        AnimationImage* backgroundImage();
        void setBackgroundColor(const QColor&);
        QColor backgroundColor();
        QString backgroundPath();
        int countTextBoxes();
        void removeSelectedObject(bool del=false);
        void removeObject(Object*, bool del=false, bool temporary=false);
        void selectObject(Object*);
        void highlightObject(Object*);
        Object * highlightedObject();
        Scene * copy();
        bool isValidObjectName(const QString&);
        QString newObjectName(QString);
        void resize(int, int);

        void insertAction(int, Action*);
        void setActions(const QList<Action*>&);
        void deleteActionAt(int);
        void deleteAction(Action*);
        QList<Action*> actions() const;
        void appendAction(Action*);

        virtual QVariantMap toJsonObject();
        QIcon icon();
        QPixmap* pixmap();

        void show();
        void hide();

        void paint(QPainter&);
        
    private slots:
        void onResizeEvent(const QResizeEvent&);

    public slots:
        void moveSelectedObjectUp();
        void moveSelectedObjectDown();
        void fillWidth();
        void clearBackground();
        
    signals:
       void resized(const QResizeEvent&);
       void dataChanged();
       void selectionChanged(Object*);

private:
       void init(const QString&);
       void removeTemporaryBackground();
};


#endif

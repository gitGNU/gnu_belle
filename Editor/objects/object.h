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

#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

#include <QObject>
#include <QPixmap>
#include <QRectF>
#include <QResizeEvent>
#include <QString>
#include <QHash>

#include "action.h"
#include "interaction.h"
#include "object_editor_widget.h"
#include "padding.h"
#include "animationimage.h"

#define RESIZE_RECT_WIDTH 7

class Scene;
class ObjectEditorWidget;
class Action;

class Object : public QObject
{
    Q_OBJECT
    
    public:
        explicit Object(QObject* parent=0, const QString& name="Object");
        Object(const QVariantMap& data, QObject* parent=0);
        virtual ~Object();
        bool contains(qreal, qreal);
        QRect sceneRect() const;
        virtual void resize(int, int);
        virtual void resize(int, int, int);
        virtual void move(int x, int y);
        void dragMove(int x, int y);
        void stopMove();
        QList<QRect> previousSceneRects() const;
        static ObjectEditorWidget* objectEditorWidget();
        static void setObjectEditorWidget(ObjectEditorWidget*);
        virtual ObjectEditorWidget* editorWidget();
        QColor backgroundColor() const;
        void setBackgroundColor(const QColor&);
        void stopResizing();
        int percentWidth() const;
        int percentHeight() const;
        Scene* scene();
        bool isValidName(const QString&);

        int borderWidth();
        void setBorderWidth(int);
        QColor borderColor();
        void setBorderColor(const QColor&);

        void setEditableName(bool);
        bool editableName();

        virtual void show();
        virtual void hide();

        /*int backgroundColorOpacity() const;
        void setBackgroundColorOpacity(int);

        int backgroundImageOpacity() const;
        void setBackgroundImageOpacity(int);*/

        int backgroundOpacity() const;
        void setBackgroundOpacity(int);

        int width() const;
        int height() const;
        int contentWidth() const;
        int contentHeight() const;
        virtual void setWidth(int, bool percent=false);
        virtual void setHeight(int, bool percent=false);
        virtual void setX(int);
        virtual void setY(int);
        int x() const;
        int y() const;

        int xRadius() const;
        void setXRadius(int);

        int yRadius() const;
        void setYRadius(int);

        bool roundedRect() const;
        void setRoundedRect(bool);

        AnimationImage* backgroundImage() const;
        void setBackgroundImage(const QString&);

        bool visible();
        void setVisible(bool);

        QString type();
        void setType(const QString&);
        void update();
        void appendEventAction(Interaction::InputEvent, Action*);
        void insertEventAction(Interaction::InputEvent, int, Action*);
        void removeEventActionAt(Interaction::InputEvent, int, bool del=false);
        QList<Action*> actionsForEvent(Interaction::InputEvent);
        virtual void paint(QPainter&);
        Object* copy();
        virtual QVariantMap toJsonObject();
        bool containsResizeRectAt(int, int);
        QRect resizeRectAt(int, int);
        QList<QRect> resizeRects();
        void updateResizeRect(int, const QPoint&);
        void setHoveredResizeRect(int);
        virtual Object* objectAt(qreal, qreal);
        void selectObjectAt(qreal, qreal);
        Object* selectedObject();

        Padding padding();
        int padding(const QString&);
        void setPadding(const Padding&);
        void setPadding(const QString&, int);

        void filterResourceData(QVariantMap&);
        QVariantMap fillWithResourceData(QVariantMap);

        void setResource(Object*);
        Object* resource();

        bool hasObjectAsParent();

        static QString defaultFontFamily();
        static void setDefaultFontFamily(const QString&);
        static int defaultFontSize();
        static void setDefaultFontSize(int);
        static QFont defaultFont();
        static void setDefaultFont(const QFont&);

    public slots:
        void onResizeEvent(QResizeEvent*);
        void setProperties(const QVariantMap &);
        void onParentResized(int, int);

    signals:
        void dataChanged(const QVariantMap& data=QVariantMap());
        void positionChanged(int, int);
        void resized(int, int);

    private:
        //void init(const QString &, int, int, QObject*);
        void init(const QString&);
        void updateResizeRects();
        void movePoint(int, const QPoint&);
        int parentWidth() const;
        int parentHeight() const;

    protected:
        QRect mSceneRect;
        QRect mBoundingRect;
        QRect mPreviousSceneRect;
        QRect mPreviousResizeRect;
        int mPercentWidth;
        int mPercentHeight;
        float mXDif;
        float mYDif;
        Padding mPadding;
        QList<QRect> mPreviousSceneRects;
        QColor mBackgroundColor;
        bool mRoundedRect;
        int mXRadius;
        int mYRadius;
        AnimationImage *mBackgroundImage;
        QHash<Interaction::InputEvent, QList<Action*> > mEventToActions;
        bool mEditableName;
        Object* mResource;
        void notify(const QString&, const QVariant&, const QVariant&);

    private: //variables
        QString mType;
        QList<QRect> mResizeRects;
        bool mVisible;
        int mOriginalResizePointIndex;
        int mBorderWidth;
        QColor mBorderColor;
        Object* mSelectedObject;

private slots:
        void onResourceDestroyed();
        void onResourceChanged(const QVariantMap&);

};


#endif

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

#include "change_visibility.h"

#include <QDebug>

static ChangeVisibilityEditorWidget* mEditorWidget = 0;

ChangeVisibility::ChangeVisibility(bool show, QObject *parent) :
    Action(parent)
{
    init(show);
}

ChangeVisibility::ChangeVisibility(const QVariantMap& data, QObject *parent) :
    Action(data, parent)
{
    QString type("Show");
    if (data.contains("type") && data.value("type").type() == QVariant::String)
        type = data.value("type").toString();

    if (type == "Hide")
        init(false);
    else
        init(true);

    if (data.contains("transitions") && data.value("transitions").type() == QVariant::List ) {
        QVariantList transitions = data.value("transitions").toList();
        for(int i=0; i <transitions.size(); i++ ) {
            if (transitions[i].type() != QVariant::Map)
                continue;
            QVariantMap data = transitions[i].toMap();
            if (data.contains("type") && data.value("type") == "Fade")
                mFadeAction = qobject_cast<Fade*>(ActionInfoManager::typeToAction(data, this));
            if (data.contains("type") && data.value("type") == "Slide")
                mSlideAction = qobject_cast<Slide*>(ActionInfoManager::typeToAction(data, this));
        }
    }
}

void ChangeVisibility::init(bool show)
{
    mToShow = show;
    mFadeAction = new Fade(this);
    mSlideAction = new Slide(this);
    mFadeAction->setDuration(0);
    mSlideAction->setDuration(0);

    if (show) {
        setName(tr("Show"));
        setType("Show");
        mFadeAction->setFadeType(Fade::In);
    }
    else {
        setName(tr("Hide"));
        setType("Hide");
        mFadeAction->setFadeType(Fade::Out);
    }

    setDescription(name() + "...");
    setSupportedEvents(Interaction::MousePress | Interaction::MouseRelease |
    Interaction::MouseMove);
}

void ChangeVisibility::setChangeVisibilityEditorWidget(ChangeVisibilityEditorWidget * widget)
{
   mEditorWidget = widget;
}

ChangeVisibilityEditorWidget* ChangeVisibility::changeVisibilityEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* ChangeVisibility::editorWidget()
{
    return mEditorWidget;
}

void ChangeVisibility::setSceneObject(Object* obj)
{
    if (! obj)
        return;

    Action::setSceneObject(obj);
    if (qobject_cast<Character*>(obj))
        setDisplayText(obj->objectName() + " (" + qobject_cast<Character*>(obj)->currentState() + ")");
    else
        setDisplayText(obj->objectName());
    if (mFadeAction)
        mFadeAction->setSceneObject(obj);
    if (mSlideAction)
        mSlideAction->setSceneObject(obj);
    emit dataChanged();
}

bool ChangeVisibility::toShow()
{
    return mToShow;
}

bool ChangeVisibility::toHide()
{
    return ! mToShow;
}

void ChangeVisibility::setFadeAction(Fade* action)
{
    mFadeAction = action;
}

Fade* ChangeVisibility::fadeAction()
{
    return mFadeAction;
}

void ChangeVisibility::setSlideAction(Slide* action)
{
    mSlideAction = action;
}

Slide* ChangeVisibility::slideAction()
{
    return mSlideAction;
}

QVariantMap ChangeVisibility::toJsonObject()
{
    QVariantMap object = Action::toJsonObject();
    QVariantList transitions;
    if (mSlideAction && mSlideAction->startPoint() != mSlideAction->endPoint())
        transitions.append(mSlideAction->toJsonObject());
    if (mFadeAction && mFadeAction->duration() > 0)
        transitions.append(mFadeAction->toJsonObject());
    object.insert("transitions", transitions);

    return object;
}

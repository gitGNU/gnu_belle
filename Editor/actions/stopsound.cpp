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

#include "stopsound.h"

#include "scene_manager.h"

#include <QDebug>

static StopSoundEditorWidget * mEditorWidget = 0;
ActionInfo StopSound::Info;

StopSound::StopSound(QObject *parent) :
    Action(parent)
{
    init();
}


StopSound::StopSound(const QVariantMap& data,QObject *parent) :
    Action(data, parent)
{
    init();

    if (data.contains("sound") && data.value("sound").type() == QVariant::String)
        setSound(data.value("sound").toString());
}

void StopSound::init()
{
    setType(Info.type);
    setName(Info.name);
    setIcon(Info.icon);
    mSound = "";
    mFadeTime = 0;
}

void StopSound::setStopSoundEditorWidget(StopSoundEditorWidget * widget)
{
   mEditorWidget = widget;
}

StopSoundEditorWidget* StopSound::stopSoundEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* StopSound::editorWidget()
{
    return mEditorWidget;
}

void StopSound::setSound(const QString & name)
{
    Scene* scene = SceneManager::currentScene();
    if (! scene)
        return;

    QList<Action*> actions = scene->actions();
    QString sound = "";
    int i = 0;

    if (! name.isEmpty()) {
        for(i=actions.size()-1; i >= 0; i--) {
            if (actions[i]->type() == "PlaySound" && actions[i]->objectName() == name)
                break;
        }

        if (i != -1)
            sound = name;
    }

    if (mSound != sound) {
        mSound = sound;
        setDisplayText(sound);
    }
}

QString StopSound::sound()
{
    return mSound;
}

void StopSound::setFadeTime(double time)
{
    mFadeTime = time;
}

double StopSound::fadeTime()
{
    return mFadeTime;
}


QVariantMap StopSound::toJsonObject()
{
    QVariantMap data = Action::toJsonObject();

    data.insert("sound", mSound);

    if (mFadeTime > 0.0)
        data.insert("fadeTime", mFadeTime);

    return data;
}

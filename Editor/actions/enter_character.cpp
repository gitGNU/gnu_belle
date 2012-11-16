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

#include "enter_character.h"
#include "javascript/object.h"
#include "javascript/array.h"

static EnterCharacterEditorWidget* mEditorWidget = 0;

EnterCharacter::EnterCharacter(QObject *parent) :
    Action(parent)
{
    setTitle(tr("Enter Character"));
    setDescription(title() + "...");
    mCurrentCharacter = 0;
}

void EnterCharacter::setEnterCharacterEditorWidget(EnterCharacterEditorWidget * widget)
{
   mEditorWidget = widget;
}

EnterCharacterEditorWidget* EnterCharacter::enterCharacterEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* EnterCharacter::editorWidget()
{
    return mEditorWidget;
}

void EnterCharacter::setCharacter(CharacterObject * character)
{
    mCurrentCharacter = character;
    if (character) {
        setValue(character->name() + " (" + character->currentState() + ")");
        emit dataChanged();
    }
}

CharacterObject* EnterCharacter::character()
{
    return mCurrentCharacter;
}


QString EnterCharacter::initialState()
{
    return mInitialState;
}

QString EnterCharacter::toJavascript()
{
    Javascript::Object obj("", "EnterCharacter");
    Javascript::Array transitions;

    obj.addArg(QString("%1.%2").arg(RESOURCES_VAR_NAME).arg(mCurrentCharacter->objectName()));
    obj.addArg(transitions.toString());

    return obj.toString();
}


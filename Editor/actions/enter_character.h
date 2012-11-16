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

#ifndef ENTER_CHARACTER_H
#define ENTER_CHARACTER_H

#include "character_object.h"
#include "action.h"
#include "enter_character_editor_widget.h"

class EnterCharacterEditorWidget;
class CharacterObject;

class EnterCharacter : public Action
{
    Q_OBJECT

    CharacterObject* mCurrentCharacter;
    QString mInitialState;
    QString mInitialStateImagePath;

public:
    EnterCharacter(QObject *parent = 0);
    static EnterCharacterEditorWidget* enterCharacterEditorWidget();
    static void setEnterCharacterEditorWidget(EnterCharacterEditorWidget*);
    virtual ActionEditorWidget* editorWidget();
    void setCharacter(CharacterObject*);
    CharacterObject* character();
    QString initialState();
    virtual QString toJavascript();

signals:

public slots:

};

#endif // ENTER_CHARACTER_H

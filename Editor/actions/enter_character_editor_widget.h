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

#ifndef ENTER_CHARACTER_EDITOR_WIDGET_H
#define ENTER_CHARACTER_EDITOR_WIDGET_H

#include <QWidget>
#include <QComboBox>

#include "action_editor_widget.h"
#include "scene_manager.h"
#include "character_object.h"
#include "enter_character.h"

class CharacterObject;
class EnterCharacter;

class EnterCharacterEditorWidget : public ActionEditorWidget
{
    Q_OBJECT

    QComboBox * mCharactersWidget;
    QComboBox* mCharacterInitialStateWidget;
    SceneManager* mSceneManager;
    QList<CharacterObject*> mCharacters;
    EnterCharacter* mCurrentAction;

public:
    explicit EnterCharacterEditorWidget(QWidget *parent = 0);
    EnterCharacterEditorWidget(SceneManager*, QWidget *parent = 0);
    virtual void updateData(Action*);
    void setCharacters();

private slots:
    void onCharacterChanged(int);
    void onInitialStateChanged(int);

private:
   void init();

signals:

public slots:


};

#endif // ENTER_CHARACTER_PROPERTIES_WIDGET_H

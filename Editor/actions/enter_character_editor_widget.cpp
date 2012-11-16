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

#include "enter_character_editor_widget.h"

#include <QDebug>

#include "scene_manager.h"

EnterCharacterEditorWidget::EnterCharacterEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    init();
}

EnterCharacterEditorWidget::EnterCharacterEditorWidget(SceneManager* sceneManager, QWidget *parent) :
    ActionEditorWidget(parent)
{
    mSceneManager = sceneManager;
    init();
}

void EnterCharacterEditorWidget::init()
{
    mCurrentAction = 0;
    mCharactersWidget = new QComboBox(this);
    mCharacterInitialStateWidget = new QComboBox(this);
    connect(mCharactersWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onCharacterChanged(int)));
    connect(mCharacterInitialStateWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onInitialStateChanged(int)));
    setCharacters();

    beginGroup(tr("Enter Character"));
    appendRow(tr("Character"), mCharactersWidget);
    appendRow(tr("Initial State"), mCharacterInitialStateWidget);
    endGroup();
}

void EnterCharacterEditorWidget::updateData(Action * action)
{
    setAction(qobject_cast<EnterCharacter*>(action));
    setCharacters();
}

void EnterCharacterEditorWidget::setCharacters()
{
    CharacterObject * character;
    Scene* scene = 0;
    QList<SceneObject*> objects;

    mCharactersWidget->clear();
    mCharacters.clear();
    mCharacterInitialStateWidget->clear();

    if (mCurrentAction && mCurrentAction->character() ) {
        mCharactersWidget->addItem(mCurrentAction->character()->name());
        mCharacters.append(mCurrentAction->character());
        mCharacterInitialStateWidget->addItems(mCurrentAction->character()->statesToPaths().keys());
        mCharacterInitialStateWidget->setDisabled(false);
    }
    else {
        mCharactersWidget->addItem("");
        mCharacters.append(0);
        mCharacterInitialStateWidget->setDisabled(true);
    }

    for (int i=0; i < mSceneManager->size(); i++) {
        scene = mSceneManager->at(i);
        if (! scene) continue;
        objects = scene->objects();

        for(int j=0; j < objects.size(); j++) {
            character = qobject_cast<CharacterObject*> (objects[j]);
            if (character && character->isAvailable()) {
                mCharactersWidget->addItem(character->name());
                mCharacters.append(character);
            }
        }
    }
}

void EnterCharacterEditorWidget::onCharacterChanged(int index)
{
    if (! mCurrentAction || index >= mCharacters.size() || index < 0)
        return;


    if (mCurrentAction->character())
        mCurrentAction->character()->setAvailable(true);

    mCurrentAction->setCharacter(mCharacters[index]);
    mCurrentAction->character()->setAvailable(false);
    mCharacterInitialStateWidget->setEnabled(true);
    mCharacterInitialStateWidget->clear();
    mCharacterInitialStateWidget->addItems(mCurrentAction->character()->statesToPaths().keys());

}

void EnterCharacterEditorWidget::onInitialStateChanged(int index)
{
    if (mCurrentAction && mCurrentAction->character()) {
        mCurrentAction->character()->setCurrentState(mCharacterInitialStateWidget->currentText());
        mCurrentAction->setValue(mCurrentAction->character()->name() + " (" + mCurrentAction->character()->currentState() + ")");
    }
}

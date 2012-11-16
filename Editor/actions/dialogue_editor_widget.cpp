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

#include "dialogue_editor_widget.h"

#include "character.h"
#include "dialoguebox.h"

#include <QDebug>

DialogueEditorWidget::DialogueEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mCurrentAction = 0;

    mChooseCharacterWidget = new QComboBox(this);
    mChooseTextBoxWidget = new QComboBox(this);
    mChooseCharacterWidget->setEditable(true);
    mTextEdit = new QTextEdit(this);

    beginGroup("Dialogue Action");
    appendRow(tr("Character"), mChooseCharacterWidget);
    appendRow(tr("Text Box/Dialogue Box"), mChooseTextBoxWidget);
    appendRow(tr("Phrase"), mTextEdit);
    endGroup();

    mTextEdit->setMaximumHeight(mTextEdit->height()/2);

    connect(mTextEdit, SIGNAL(textChanged()), this, SLOT(onTextEdited()));
    connect(mChooseTextBoxWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onTextBoxChanged(int)));
    connect(mChooseTextBoxWidget, SIGNAL(highlighted(int)), this, SLOT(onTextBoxHighlighted(int)));
    connect(mChooseCharacterWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onCharacterChanged(int)));
    connect(mChooseCharacterWidget, SIGNAL(highlighted(int)), this, SLOT(onCharacterHighlighted(int)));
    connect(mChooseCharacterWidget, SIGNAL(editTextChanged(const QString&)), this, SLOT(onCharacterNameChanged(const QString&)));

    if (mChooseTextBoxWidget->view())
        mChooseTextBoxWidget->view()->installEventFilter(this);
    if (mChooseCharacterWidget->view())
        mChooseCharacterWidget->view()->installEventFilter(this);
}


void DialogueEditorWidget::updateData(Action * action)
{
    Character* character = 0;
    TextBox* textBox = 0;
    DialogueBox * dialogueBox = 0;

    mChooseCharacterWidget->clear();
    mChooseTextBoxWidget->clear();
    mOutputBoxes.clear();
    mCharacters.clear();

    mCurrentAction = qobject_cast<Dialogue*> (action);
    if (! mCurrentAction)
        return;

    Scene * scene = SceneManager::currentScene();
    if (! scene)
        return;

    if (mCurrentAction->character()) {
        mChooseCharacterWidget->addItem(mCurrentAction->character()->objectName());
        mCharacters.append(mCurrentAction->character());
    }

    if (mCurrentAction->sceneObject()) {
        textBox = qobject_cast<TextBox*>(mCurrentAction->sceneObject());
        if (textBox) {
            mChooseTextBoxWidget->addItem(textBox->objectName());
            mOutputBoxes.append(textBox);
        }
    }

    QList<Object*> objects = scene->objects();
    for (int i=0; i < objects.size(); i++) {
        character = qobject_cast<Character*>(objects[i]);
        if (character && character != mCurrentAction->character()) {
            mChooseCharacterWidget->addItem(character->objectName());
            mCharacters.append(character);
            continue;
        }

        if (isValidOutputBox(objects[i])){
            mChooseTextBoxWidget->addItem(objects[i]->objectName());
            mOutputBoxes.append(objects[i]);
        }
    }

    if (! mChooseTextBoxWidget->count())
        mTextEdit->setEnabled(false);
    else {
        mTextEdit->setEnabled(true);

        mCurrentAction->setSceneObject(mOutputBoxes[mChooseTextBoxWidget->currentIndex()]);
        mTextEdit->setText(mCurrentAction->text());
    }

    if (mChooseCharacterWidget->count() && ! mCurrentAction->character()) {
        mCurrentAction->setCharacter(mCharacters[0]);
    }

    onCharacterChanged(mChooseCharacterWidget->currentIndex());
}

void DialogueEditorWidget::onTextEdited()
{
    if (! mTextEdit || ! mCurrentAction)
        return;

    mCurrentAction->setText(mTextEdit->toPlainText());
    TextBox* textObj = qobject_cast<TextBox*>(mCurrentAction->sceneObject());
    if (textObj) {
        textObj->setPlaceholderText(mTextEdit->toPlainText());
    }
    /*DialogueBox* obj = qobject_cast<DialogueBox*>(mCurrentAction->sceneObject());
    if (obj) {
         obj->setText(mCurrentAction->value());
    }
    else {
        TextBox* textObj = qobject_cast<TextBox*>(mCurrentAction->sceneObject());
        if (textObj) {
            textObj->setText(mCurrentAction->value());
        }
    }*/
}

void DialogueEditorWidget::onTextBoxChanged(int index)
{
    if (mCurrentAction && index >= 0 && index < mOutputBoxes.size()) {
        mCurrentAction->setSceneObject(mOutputBoxes[index]);
        //onTextEdited();
    }
}

void DialogueEditorWidget::onCharacterChanged(int index)
{

    if (mCurrentAction && index >= 0 && index < mCharacters.size() && mCharacters[index]) {
        mCurrentAction->setCharacter(mCharacters[index]);

        /*if (mCurrentAction->sceneObject()) {
            DialogueBox* obj = qobject_cast<DialogueBox*>(mCurrentAction->sceneObject());
            if (obj) {
                obj->setSpeakerName(mChooseCharacterWidget->currentText());
                obj->setSpeakerColor(mCurrentAction->character()->nameColor());
                obj->setTextColor(mCurrentAction->character()->textColor());
            }
            else {
                TextBox* textObj = qobject_cast<TextBox*>(mCurrentAction->sceneObject());
                if (textObj)
                    textObj->setTextColor(mCurrentAction->character()->textColor());
            }
        }*/
    }
}

void DialogueEditorWidget::onCharacterNameChanged(const QString & name)
{
    if (mCurrentAction) {
        int i;
        for (i=0; i < mCharacters.size(); i++)
            if (mCharacters[i]->objectName() == name) {
                mCurrentAction->setCharacter(mCharacters[i]);
                break;
            }

        //if no character with name <name> found, set just the name
        if (i == mCharacters.size())
            mCurrentAction->setCharacterName(name);
    }
}

void DialogueEditorWidget::onTextBoxHighlighted(int index)
{
    if (index >= 0 && index < mOutputBoxes.size()) {
        SceneManager::currentScene()->highlightObject(mOutputBoxes[index]);
    }
}

void DialogueEditorWidget::onCharacterHighlighted(int index)
{
    if (index >= 0 && index < mCharacters.size()) {
        SceneManager::currentScene()->highlightObject(mCharacters[index]);
    }
}


bool DialogueEditorWidget::eventFilter(QObject *obj, QEvent *event)
{

    if ((obj == mChooseTextBoxWidget->view() || obj == mChooseCharacterWidget->view()) && event->type() == QEvent::Hide) {
       if ( SceneManager::currentScene())
           SceneManager::currentScene()->highlightObject(0);
       return false;
    }

    if (obj == mChooseTextBoxWidget->view() && event->type() == QEvent::Show && SceneManager::currentScene()) {
        if (mChooseTextBoxWidget->count() == 1 && ! mOutputBoxes.isEmpty())
            SceneManager::currentScene()->highlightObject(mOutputBoxes.first());
    }
    else if (obj == mChooseCharacterWidget->view() && event->type() == QEvent::Show && SceneManager::currentScene()) {
      if (mChooseCharacterWidget->count() == 1 && ! mCharacters.isEmpty())
           SceneManager::currentScene()->highlightObject(mCharacters.first());
    }


   return false;
}

bool DialogueEditorWidget::isValidOutputBox(Object* object)
{
    if ( qobject_cast<TextBox*>(object) || qobject_cast<DialogueBox*>(object))
        return true;

    return false;
}

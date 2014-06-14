/* Copyright (C) 2012-2014 Carlos Pais
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
    mWaitCheckBox = new QCheckBox(this);

    beginGroup("Dialogue Action");
    appendRow(tr("Character"), mChooseCharacterWidget);
    appendRow(tr("Text Box/Dialogue Box"), mChooseTextBoxWidget);
    appendRow(tr("Phrase"), mTextEdit);
    appendRow(tr("Wait on Finished"), mWaitCheckBox);
    endGroup();

    mTextEdit->setMaximumHeight(mTextEdit->height()/2);

    connect(mTextEdit, SIGNAL(textChanged()), this, SLOT(onTextEdited()));
    connect(mChooseTextBoxWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onTextBoxChanged(int)));
    connect(mChooseTextBoxWidget, SIGNAL(highlighted(int)), this, SLOT(onTextBoxHighlighted(int)));
    connect(mChooseCharacterWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onCharacterChanged(int)));
    connect(mChooseCharacterWidget, SIGNAL(highlighted(int)), this, SLOT(onCharacterHighlighted(int)));
    connect(mChooseCharacterWidget, SIGNAL(editTextChanged(const QString&)), this, SLOT(onCharacterNameChanged(const QString&)));
    connect(mWaitCheckBox, SIGNAL(clicked(bool)), this, SLOT(onWaitOnFinishedChanged(bool)));

    if (mChooseTextBoxWidget->view())
        mChooseTextBoxWidget->view()->installEventFilter(this);
    if (mChooseCharacterWidget->view())
        mChooseCharacterWidget->view()->installEventFilter(this);

    this->resizeColumnToContents(0);
}


void DialogueEditorWidget::updateData(Action * action)
{   
    mCurrentAction = 0;
    Character* character = 0;
    Object* object = 0;

    mChooseCharacterWidget->clear();
    mChooseCharacterWidget->clearEditText();
    mChooseTextBoxWidget->clear();
    mOutputBoxes.clear();
    mCharacters.clear();

    Dialogue* dialogueAction = qobject_cast<Dialogue*> (action);
    if (! dialogueAction)
        return;

    if (! dialogueAction->characterName().isEmpty()) {
        if (dialogueAction->character()) {
            mChooseCharacterWidget->addItem(dialogueAction->characterName());
            mCharacters.append(dialogueAction->character());
        }
        else {
            mChooseCharacterWidget->setEditText(dialogueAction->characterName());
        }
    }

    if (dialogueAction->sceneObject()) {
        object = dialogueAction->sceneObject();
        if (isValidOutputBox(object)) {
            mChooseTextBoxWidget->addItem(object->objectName());
            mOutputBoxes.append(object);
        }
    }

    Scene * scene = dialogueAction->scene();
    if (! scene)
        return;
    QList<Object*> objects = scene->objects();
    for (int i=0; i < objects.size(); i++) {
        character = qobject_cast<Character*>(objects[i]);
        if (character && character != dialogueAction->character()) {
            mChooseCharacterWidget->addItem(character->objectName());
            mCharacters.append(character);
            continue;
        }

        if (dialogueAction->sceneObject() != objects[i] && isValidOutputBox(objects[i])){
            mChooseTextBoxWidget->addItem(objects[i]->objectName());
            mOutputBoxes.append(objects[i]);
        }
    }

    if (! mChooseTextBoxWidget->count())
        mTextEdit->setEnabled(false);
    else {
        mTextEdit->setEnabled(true);
        dialogueAction->setSceneObject(mOutputBoxes[mChooseTextBoxWidget->currentIndex()]);
        mTextEdit->setText(dialogueAction->text());
    }

    if (mChooseCharacterWidget->count()) {
        if (dialogueAction->characterName().isEmpty())
            dialogueAction->setCharacter(mCharacters[0]);
    }

    if (dialogueAction->character())
        mChooseCharacterWidget->setCurrentIndex(0);
    else
        mChooseCharacterWidget->setEditText(dialogueAction->characterName());

    mWaitCheckBox->setChecked(dialogueAction->mouseClickOnFinish());

    //only set currentAction after updating all the widgets
    //otherwise updating the widgets would mess up the currentAction's data.
    mCurrentAction = dialogueAction;
    setTextInOutputBox();
}

void DialogueEditorWidget::onTextEdited()
{
    if (! mTextEdit || ! mCurrentAction)
        return;

    QString dialogue = mTextEdit->toPlainText();
    mCurrentAction->setText(dialogue);
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
    }
}

void DialogueEditorWidget::onCharacterNameChanged(const QString & name)
{
    if (mCurrentAction) {
        int i;
        for (i=0; i < mCharacters.size(); i++) {
            if (mCharacters[i]->objectName() == name) {
                mCurrentAction->setCharacter(mCharacters[i]);
                break;
            }
        }

        //if no character with name <name> found, set just the name
        if (i == mCharacters.size())
            mCurrentAction->setCharacterName(name);
    }
}

void DialogueEditorWidget::onTextBoxHighlighted(int index)
{
    if (index >= 0 && index < mOutputBoxes.size() && mCurrentAction && mCurrentAction->scene()) {
        mCurrentAction->scene()->highlightObject(mOutputBoxes[index]);
    }
}

void DialogueEditorWidget::onCharacterHighlighted(int index)
{
    if (index >= 0 && index < mCharacters.size() && mCurrentAction && mCurrentAction->scene()) {
        mCurrentAction->scene()->highlightObject(mCharacters[index]);
    }
}


bool DialogueEditorWidget::eventFilter(QObject *obj, QEvent *event)
{
    Scene* scene = 0;
    if (mCurrentAction)
        scene = mCurrentAction->scene();

    if ((obj == mChooseTextBoxWidget->view() || obj == mChooseCharacterWidget->view()) && event->type() == QEvent::Hide) {
       if ( scene)
           scene->highlightObject(0);
       return false;
    }

    if (obj == mChooseTextBoxWidget->view() && event->type() == QEvent::Show && scene) {
        if (mChooseTextBoxWidget->count() == 1 && ! mOutputBoxes.isEmpty())
            scene->highlightObject(mOutputBoxes.first());
    }
    else if (obj == mChooseCharacterWidget->view() && event->type() == QEvent::Show && scene) {
      if (mChooseCharacterWidget->count() == 1 && ! mCharacters.isEmpty())
           scene->highlightObject(mCharacters.first());
    }


   return false;
}

bool DialogueEditorWidget::isValidOutputBox(Object* object)
{
    if (! object)
        return false;

    QString className = object->metaObject()->className();
    if (className == "TextBox" || className == "DialogueBox")
        return true;

    return false;
}

void DialogueEditorWidget::setTextInOutputBox()
{
    if (! mCurrentAction || ! mCurrentAction->sceneObject())
        return;

    DialogueBox * dialogueBox = qobject_cast<DialogueBox*>(mCurrentAction->sceneObject());
    if (dialogueBox) {
        dialogueBox->setText(mCurrentAction->characterName(), mCurrentAction->text());
    }
    else {
        TextBox* textBox = qobject_cast<TextBox*>(mCurrentAction->sceneObject());
        if (textBox) {
            textBox->setPlaceholderText(mCurrentAction->text());
        }
    }
}

void DialogueEditorWidget::onWaitOnFinishedChanged(bool state)
{
    if (mCurrentAction)
        mCurrentAction->setMouseClickOnFinish(state);
}

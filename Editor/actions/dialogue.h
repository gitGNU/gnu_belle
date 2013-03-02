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

#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "dialogue_editor_widget.h"
#include "action.h"
#include "dialoguebox.h"
#include "character.h"

class DialogueEditorWidget;
class Action;
class DialogueBox;
class Character;


class Dialogue : public Action
{
    Q_OBJECT

    Character* mCharacter;
    QString mCharacterName;
    QString mText;

public:
    static ActionInfo Info;

public:
    explicit Dialogue(QObject *parent = 0);
    Dialogue(const QVariantMap&, QObject *parent = 0);
    static DialogueEditorWidget* dialogueEditorWidget();
    static void setDialogueEditorWidget(DialogueEditorWidget*);
    virtual ActionEditorWidget* editorWidget();

    QString text();
    void setText(const QString&);

    void setCharacter(Character*);
    Character* character();

    void setCharacterName(const QString&);
    QString characterName() const;

    virtual void paint(const QPainter &);
    virtual QString displayText() const;
    virtual QVariantMap toJsonObject();

signals:

private slots:
    void onCharacterDestroyed();

private:
    void init();

};

#endif // DIALOGUE_H

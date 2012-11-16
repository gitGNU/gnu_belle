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

#ifndef CHARACTER_PROPERTIES_WIDGET_H
#define CHARACTER_PROPERTIES_WIDGET_H

#include <QLineEdit>
#include <QComboBox>

#include "object_editor_widget.h"
#include "character.h"
#include "add_character_widget.h"

class Object;
class Character;
class AddCharacterWidget;

class CharacterPropertiesWidget : public ObjectEditorWidget
{
    Q_OBJECT

    QLineEdit *mNameEdit;
    QComboBox *mStatusChooser;
    AddCharacterWidget* mAddCharacterWidget;
    Character* mCurrentCharacter;
    ColorPushButton * mNameColorButton;
    ColorPushButton * mTextColorButton;

public:
    explicit CharacterPropertiesWidget(QWidget *parent = 0);
    virtual void updateData(Object *);

signals:

public slots:

private slots:
    void onNameColorChosen(const QColor&);
    void onTextColorChosen(const QColor&);
    void onStatusChosen(const QString &);
};

#endif // CHARACTER_PROPERTIES_WIDGET_H

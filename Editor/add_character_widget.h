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

#ifndef ADD_CHARACTER_WIDGET_H
#define ADD_CHARACTER_WIDGET_H

#include "widget_group.h"
#include <QLineEdit>
#include <QPushButton>

#include "character.h"

class AddCharacterWidget : public WidgetGroup
{
    Q_OBJECT

    QLineEdit *mStatusEdit;
    QPushButton *mPbSelectImage;
    QLineEdit *mNameEdit;
    QHash<QPushButton*, QString> mPushButtonToState;
    QHash<QString, QString> mStateToPath;
    QString mName;
    QIcon mDeleteIcon;


public:
    explicit AddCharacterWidget(QWidget* parent = 0);
    QString name();
    QHash<QString, QString> statesAndImagePaths();
    void addImage(const QString&);
    QLineEdit* nameEdit();

private:
    void addSelectImageWidget();

private slots:
    void onPbSelectImageClicked();
    void onStatusEdited(const QString&);
    void onDeleteCharacterImage();
    void onCreateCharacter();
    void onNameEdited(const QString&);
signals:
    void imageSelected(const QString& path);
    void widgetState(bool val);
};

#endif // ADD_CHARACTER_WIDGET_H

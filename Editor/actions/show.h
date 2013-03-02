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

#ifndef SHOW_H
#define SHOW_H

#include "change_visibility.h"
#include "show_editor_widget.h"

class Show : public ChangeVisibility
{
    Q_OBJECT

    QString mCharacterState;
    QString mStateImagePath;

public:
    static ActionInfo Info;

public:
    explicit Show(QObject *parent = 0);
    explicit Show(const QVariantMap& data, QObject *parent = 0);
    static ShowEditorWidget* showEditorWidget();
    static void setShowEditorWidget(ShowEditorWidget*);
    virtual ActionEditorWidget* editorWidget();
    void setCharacterState(const QString&);
    QString characterState();
    virtual QString displayText() const;
    QVariantMap toJsonObject();

private:
    void init();
};


#endif // SHOW_H

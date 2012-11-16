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

#ifndef ACTION_EDITOR_WIDGET_H
#define ACTION_EDITOR_WIDGET_H

#include <QLineEdit>

#include "properties_widget.h"
#include "action.h"

class Action;
class Object;

class ActionEditorWidget : public PropertiesWidget
{
    Q_OBJECT

    Action* mAction;
    QLineEdit* mNameEdit;
    QList<Object*> mObjects;

public:
    explicit ActionEditorWidget(QWidget *parent = 0);
    virtual void updateData(Action*);
    Action* action();
    void setAction(Action*);

    QList<Object*> objects();
    void setObjects(QList<Object*>&);
    ActionEditorWidget* copy();

signals:

private slots:
    void onNameEdited(const QString& name);

};

#endif // ACTION_PROPERTIES_WIDGET_H

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

#include "label_editor_widget.h"

#include <QLineEdit>
#include <QDebug>

#include "scene.h"

LabelEditorWidget::LabelEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mCurrentLabel = 0;

    mLabelEdit = new QLineEdit(this);
    connect(mLabelEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onLabelEdited(const QString&)));

    beginGroup(tr("Label Editor"));
    appendRow(tr("Label"), mLabelEdit);
    endGroup();
}

void LabelEditorWidget::updateData(Action * action)
{
    mCurrentLabel = qobject_cast<Label*>(action);

    if (! mCurrentLabel)
        return;

    mLabelEdit->setText(mCurrentLabel->objectName());

}

void LabelEditorWidget::onLabelEdited(const QString & text)
{
    if (! mCurrentLabel)
        return;

    Scene* scene = qobject_cast<Scene*>(mCurrentLabel->parent());
    if (! scene)
        return;

    QList<Action*> actions = scene->actions();
    bool validName = true;
    for(int i=0; i < actions.size(); i++) {
        if (qobject_cast<Label*>(actions[i]) && actions[i]->objectName() == text) {
            validName = false;
            break;
        }
    }

    if (validName) {
        mCurrentLabel->setObjectName(text);
        mLabelEdit->setStyleSheet("");
    }
    else
        mLabelEdit->setStyleSheet("background-color: rgba(255, 0, 0, 100);");
}

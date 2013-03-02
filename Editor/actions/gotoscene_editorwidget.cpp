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

#include "gotoscene_editorwidget.h"

GoToSceneEditorWidget::GoToSceneEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mCurrentGoToScene = 0;

    mNameEdit = new QLineEdit(this);
    connect(mNameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onNameEdited(const QString&)));

    beginGroup(tr("Go to scene Editor"));
    appendRow(tr("Scene"), mNameEdit);
    endGroup();

    resizeColumnToContents(0);
}

void GoToSceneEditorWidget::updateData(Action * action)
{
    mCurrentGoToScene = qobject_cast<GoToScene*>(action);

    if (! mCurrentGoToScene)
        return;

    mNameEdit->setText(mCurrentGoToScene->targetSceneName());

}

void GoToSceneEditorWidget::onNameEdited(const QString & text)
{
    if (! mCurrentGoToScene)
        return;

   mCurrentGoToScene->setTargetSceneName(text);
   if (! text.isEmpty() && mCurrentGoToScene->targetSceneName() == text)
        mNameEdit->setStyleSheet("background-color: rgba(0, 255, 0, 100);");
   else
        mNameEdit->setStyleSheet("background-color: rgba(255, 0, 0, 100);");
}

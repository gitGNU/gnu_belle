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

#include "get_user_input_editor_widget.h"

#include <QLineEdit>

#include "get_user_input.h"

GetUserInputEditorWidget::GetUserInputEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mMessageEdit = new QLineEdit(this);
    connect(mMessageEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onMessageChanged(const QString &)));

    mVariableEdit = new QLineEdit(this);
    connect(mVariableEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onVariableChanged(const QString &)));

    mDefaultValueEdit= new QLineEdit(this);
    connect(mDefaultValueEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onDefaultValueChanged(const QString &)));

    beginGroup(tr("Get user input editor"));
    appendRow(tr("Message"), mMessageEdit);
    appendRow(tr("Variable"), mVariableEdit);
    appendRow(tr("Default Value"), mDefaultValueEdit);
    endGroup();

    resizeColumnToContents(0);

    mCurrentAction = 0;

}

void GetUserInputEditorWidget::updateData(Action * action)
{
    mCurrentAction = qobject_cast<GetUserInput*>(action);

    if (! mCurrentAction)
        return;

    mMessageEdit->setText(mCurrentAction->message());
    mVariableEdit->setText(mCurrentAction->variable());
    mDefaultValueEdit->setText(mCurrentAction->defaultValue());
}

void GetUserInputEditorWidget::onMessageChanged(const QString & value)
{
    if (mCurrentAction)
        mCurrentAction->setMessage(value);
}

void GetUserInputEditorWidget::onVariableChanged(const QString & value)
{
    if (mCurrentAction)
        mCurrentAction->setVariable(value);
}

void GetUserInputEditorWidget::onDefaultValueChanged(const QString & value)
{
    if (mCurrentAction)
        mCurrentAction->setDefaultValue(value);
}

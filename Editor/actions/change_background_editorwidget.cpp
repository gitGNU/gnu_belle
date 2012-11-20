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

#include "change_background_editorwidget.h"

#include <QtDebug>

ChangeBackgroundEditorWidget::ChangeBackgroundEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mImageChooser = new ChooseFileButton(ChooseFileButton::ImageFilter, this);
    mChooseBackgroundColorButton = new ColorPushButton(this);

   beginGroup(tr("Change Background"));
   appendRow(tr("New Image"), mImageChooser);
   appendRow(tr("Background Color"), mChooseBackgroundColorButton);
   endGroup();

   connect(mImageChooser, SIGNAL(fileSelected(const QString&)), this, SLOT(onFileSelected(const QString&)));
   connect(mChooseBackgroundColorButton, SIGNAL(colorChosen(QColor)), this, SLOT(onBackgroundColorSelected(const QColor&)));
   resizeColumnToContents(0);
}

void ChangeBackgroundEditorWidget::updateData(Action * action)
{
    mCurrentAction = qobject_cast<ChangeBackground*>(action);

    if (! mCurrentAction)
        return;

    mImageChooser->blockSignals(true);
    mChooseBackgroundColorButton->blockSignals(true);
    mImageChooser->setFilePath(mCurrentAction->backgroundPath());
    mChooseBackgroundColorButton->setColor(mCurrentAction->backgroundColor());
    mChooseBackgroundColorButton->blockSignals(false);
    mImageChooser->blockSignals(false);
}

void ChangeBackgroundEditorWidget::onFileSelected(const QString & filepath)
{
    if (mCurrentAction)
        mCurrentAction->setBackgroundImage(filepath);
}

void ChangeBackgroundEditorWidget::onBackgroundColorSelected(const QColor & color)
{
    if (mCurrentAction)
        mCurrentAction->setBackgroundColor(color);
}

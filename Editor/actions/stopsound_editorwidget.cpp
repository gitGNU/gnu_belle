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

#include "stopsound_editorwidget.h"

#include <QDebug>

StopSoundEditorWidget::StopSoundEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mTargetSoundEdit = new QLineEdit(this);
    mFadeOutSpinBox = new QDoubleSpinBox(this);
    mFadeOutSpinBox->setMaximum(10);
    mFadeOutSpinBox->setSingleStep(0.1);

    beginGroup(tr("Stop Sound Editor"));
    appendRow(tr("Target sound"), mTargetSoundEdit);
    appendRow(QString("%1 %2").arg(tr("Fade out duration")).arg("(s)") , mFadeOutSpinBox);
    endGroup();

    connect(mTargetSoundEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onTargetSoundEdited(const QString&)));
    connect(mFadeOutSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onFadeChanged(double)));

    resizeColumnToContents(0);
}

void StopSoundEditorWidget::updateData(Action * action)
{
    ActionEditorWidget::updateData(action);

    mCurrentStopSoundAction = qobject_cast<StopSound*>(action);
    if (! mCurrentStopSoundAction)
        return;

    mTargetSoundEdit->setText(mCurrentStopSoundAction->sound());
    mFadeOutSpinBox->setValue(mCurrentStopSoundAction->fadeTime());
}


void StopSoundEditorWidget::onTargetSoundEdited(const QString& text)
{
    if (! mCurrentStopSoundAction)
        return;

    mCurrentStopSoundAction->setSound(text);

    if (mCurrentStopSoundAction->sound() != text)
        mTargetSoundEdit->setStyleSheet("background-color: rgba(255, 0, 0, 100);");
    else
        mTargetSoundEdit->setStyleSheet("background-color: rgba(0, 255, 0, 100);");
}


void StopSoundEditorWidget::onFadeChanged(double val)
{
    if (mCurrentStopSoundAction)
        mCurrentStopSoundAction->setFadeTime(val);
}

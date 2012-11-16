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

#include "wait_editor_widget.h"

#include <QDebug>

WaitEditorWidget::WaitEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mWaitTypeWidget = new QComboBox(this);
    mWaitTypeWidget->addItem(tr("Timed"));
    mWaitTypeWidget->addItem(tr("Until mouse button is clicked"));
    mWaitTypeWidget->addItem(tr("Forever"));


    mSkipBox = new QCheckBox(this);
    mTimeSpin = new QDoubleSpinBox(this);
    mTimeSpin->setValue(1);
    mTimeSpin->setSingleStep(0.1);

    beginGroup(tr("Wait"));
    appendRow(tr("Type"), mWaitTypeWidget);
    appendRow(tr("Time (sec)"), mTimeSpin);
    appendRow(tr("Allow skipping"), mSkipBox);
    endGroup();

    resizeColumnToContents(0);

    mCurrentAction = 0;

    connect(mWaitTypeWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(mTimeSpin, SIGNAL(valueChanged(double)), this, SLOT(onTimeChanged(double)));

}

void WaitEditorWidget::updateData(Action * action)
{
    mCurrentAction = qobject_cast<Wait*>(action);
    if (! mCurrentAction)
        return;

    mWaitTypeWidget->setCurrentIndex(mCurrentAction->waitType());
    updateWidgets(mWaitTypeWidget->currentIndex());
}

void WaitEditorWidget::onCurrentIndexChanged(int index)
{
    if (! mCurrentAction)
        return;

    if (index >= 1) {
       mCurrentAction->setDisplayText(mWaitTypeWidget->currentText());
       mCurrentAction->setWaitTypeFromIndex(index);
    }
    else
       mCurrentAction->setTime(mTimeSpin->value());

    updateWidgets(index);
}

void WaitEditorWidget::onTimeChanged(double value)
{
    if (mCurrentAction)
        mCurrentAction->setTime(value);
}

void WaitEditorWidget::updateWidgets(int index)
{
    bool enabled = false;

    if (index >= 1)
       enabled = false;
    else
       enabled = true;

    mSkipBox->setEnabled(enabled);
    mTimeSpin->setEnabled(enabled);
}

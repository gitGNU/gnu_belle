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

#include "changecolor_editor_widget.h"

#include "changecolor.h"

ChangeColorEditorWidget::ChangeColorEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mObjectsComboBox = new QComboBox(this);
    mColorButton = new ColorPushButton(this);
    mOpacitySlider = new QSlider(Qt::Horizontal, this);
    mChangeObjectColorCheckBox = new QCheckBox(this);
    mChangeObjectColorCheckBox->setChecked(true);
    mChangeObjectBackgroundColorCheckBox = new QCheckBox(this);
    mOpacitySlider->setMinimum(0);
    mOpacitySlider->setMaximum(255);
    mChangeColorAction = 0;

    beginGroup(tr("Change color editor"));
    beginSubGroup(tr("Object"), mObjectsComboBox);
    appendRow(tr("Object"), mChangeObjectColorCheckBox);
    appendRow(tr("Object's background"), mChangeObjectBackgroundColorCheckBox);
    endGroup();
    appendRow(tr("Color"), mColorButton);
    appendRow(tr("Opacity"), mOpacitySlider);
    endGroup();
    resizeColumnToContents(0);

    connect(mObjectsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onObjectChanged(int)));
    connect(mChangeObjectColorCheckBox, SIGNAL(toggled(bool)), this, SLOT(onChangeObjectColorToggled(bool)));
    connect(mChangeObjectBackgroundColorCheckBox, SIGNAL(toggled(bool)), this, SLOT(onChangeObjectBackgroundColorToggled(bool)));
    connect(mColorButton, SIGNAL(colorChosen(const QColor&)), this, SLOT(onColorChosen(const QColor&)));
    connect(mOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(onOpacityChanged(int)));

}

void ChangeColorEditorWidget::updateData(Action * action)
{
    if (! action || mChangeColorAction == action)
        return;

    mChangeColorAction = 0;
    ChangeColor* changeColorAction = qobject_cast<ChangeColor*>(action);

    mObjectsComboBox->clear();

    if (changeColorAction->sceneObject()) {
        mObjectsComboBox->addItem(changeColorAction->sceneObject()->objectName());
        mObjects.append(changeColorAction->sceneObject());
    }

    QList<Object*> objects = ResourceManager::resources();

    foreach(Object* obj, objects) {
        if (obj == changeColorAction->sceneObject())
            continue;

        if (obj) {
            mObjectsComboBox->addItem(obj->objectName());
            mObjects.append(obj);
        }
    }

    if (! changeColorAction->sceneObject() && ! mObjects.isEmpty())
        changeColorAction->setSceneObject(mObjects[0]);

    mColorButton->setColor(changeColorAction->color());
    mOpacitySlider->setValue(changeColorAction->opacity());

    mChangeColorAction = changeColorAction;

}

void ChangeColorEditorWidget::onColorChosen(const QColor & color)
{
    if (mChangeColorAction) {
        mChangeColorAction->setColor(color);
    }
}

void ChangeColorEditorWidget::onObjectChanged(int index)
{
    if (! mChangeColorAction || index < 0 || index >= mObjects.size())
        return;

    mChangeColorAction->setSceneObject(mObjects[index]);

}

void ChangeColorEditorWidget::onChangeObjectColorToggled(bool checked)
{
    if (! checked && !mChangeObjectBackgroundColorCheckBox->isChecked())
        mChangeObjectColorCheckBox->setChecked(true);

    if(mChangeColorAction)
        mChangeColorAction->setChangeObjectColor(mChangeObjectColorCheckBox->isChecked());
}

void ChangeColorEditorWidget::onChangeObjectBackgroundColorToggled(bool checked)
{
    if (! checked && ! mChangeObjectColorCheckBox->isChecked())
        mChangeObjectBackgroundColorCheckBox->setChecked(true);

    if(mChangeColorAction)
        mChangeColorAction->setChangeObjectBackgroundColor(mChangeObjectBackgroundColorCheckBox->isChecked());
}

void ChangeColorEditorWidget::onOpacityChanged(int value)
{
    if(mChangeColorAction)
        mChangeColorAction->setOpacity(value);
}

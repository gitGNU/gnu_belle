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

#include "slide_editor_widget.h"

#include <QLineEdit>
#include <QComboBox>
#include <QDebug>

#include "slide.h"
#include "scene_manager.h"

SlideEditorWidget::SlideEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{

    //mStartButton = new QPushButton("(0, 0)", this);
    //mEndButton = new QPushButton("(0, 0)", this);
    mObjectChooser = new QComboBox(this);
    mStartXSlider = new QSpinBox(this);
    mStartYSlider = new QSpinBox(this);
    QPushButton* objectCoordStartButton = new QPushButton(tr("Get Coordinates"), this);
    objectCoordStartButton->setObjectName("start");
    mEndXSlider = new QSpinBox(this);
    mEndYSlider = new QSpinBox(this);
    QPushButton* objectCoordEndButton = new QPushButton(tr("Get Coordinates"), this);
    objectCoordEndButton->setObjectName("end");
    mDurationSpinBox = new QDoubleSpinBox(this);
    mDurationSpinBox->setSingleStep(0.1);
    mDurationSpinBox->setMinimum(0);

    mStartXSlider->setObjectName("start x");
    mStartYSlider->setObjectName("start y");
    mEndXSlider->setObjectName("end x");
    mEndYSlider->setObjectName("end y");

    beginGroup(tr("Slide Action"));
    appendRow(tr("Object"), mObjectChooser, "Object");

    beginSubGroup(tr("Start Point"));
    appendRow(tr("Get object's coordinates"), objectCoordStartButton);
    appendRow("x", mStartXSlider);
    appendRow("y", mStartYSlider);
    endGroup();

    beginSubGroup(tr("End Point"));
    appendRow(tr("Get object's coordinates"), objectCoordEndButton);
    appendRow("x", mEndXSlider);
    appendRow("y", mEndYSlider);
    endGroup();

    appendRow(tr("Duration (sec)"),  mDurationSpinBox);
    endGroup();

    connect(mStartXSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(mStartYSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(mEndXSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(mEndYSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(mObjectChooser, SIGNAL(currentIndexChanged(int)), this, SLOT(onResourceChanged(int)));
    connect(mDurationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onDurationChanged(double)));
    connect(objectCoordStartButton, SIGNAL(clicked()), this, SLOT(onGetObjectCoordinates()));
    connect(objectCoordEndButton, SIGNAL(clicked()), this, SLOT(onGetObjectCoordinates()));

    /*connect(mStartButton, SIGNAL(pressed()), this, SLOT(onButtonClicked()));
    connect(mEndButton, SIGNAL(pressed()), this, SLOT(onButtonClicked()));
    mStartButton->installEventFilter(this);
    mEndButton->installEventFilter(this);*/
    mIsDown = false;
    mCurrentSlide = 0;
    resizeColumnToContents(0);
}

void SlideEditorWidget::updateData(Action * action)
{
    Slide * slide  = qobject_cast<Slide*>(action);
    if (! slide || ! slide->scene())
        return;

    mCurrentSlide = 0;

    mStartXSlider->setValue(slide->startX());
    mStartYSlider->setValue(slide->startY());

    mEndXSlider->setValue(slide->endX());
    mEndYSlider->setValue(slide->endY());

    mDurationSpinBox->setValue(slide->duration());

    mObjectChooser->clear();

    Object* currObj = slide->sceneObject();
    QList<Object*> objects;
    if (currObj) {
        mObjectChooser->addItem(currObj->objectName());
        objects.append(currObj);
    }

    QList<Object*> objs = slide->scene()->objects();
    for (int i=0; i < objs.size(); i++) {
        if (objs[i] && objs[i] != currObj) {
            mObjectChooser->addItem(objs[i]->objectName());
            objects.append(objs[i]);
        }
    }

    this->setObjects(objects);

    if (! currObj && ! objects.isEmpty())
        slide->setSceneObject(objects[0]);

    if (slide->sceneObject()) {
        mStartXSlider->setRange(-slide->sceneObject()->width(), Scene::width());
        mStartYSlider->setRange(-slide->sceneObject()->height(), Scene::height());
        mEndXSlider->setRange(-slide->sceneObject()->width(), Scene::width());
        mEndYSlider->setRange(-slide->sceneObject()->height(), Scene::height());
    }

    mCurrentSlide = slide;
}

void SlideEditorWidget::onButtonClicked()
{
    QPushButton* btn = static_cast<QPushButton*>(sender());
    btn->setChecked(! btn->isChecked());
    //mIsDown = true;
    //btn->setDown(true);

}

bool SlideEditorWidget::eventFilter(QObject *obj, QEvent *event)
{
    /*if (event->type() == QEvent::MouseButtonRelease) {
        if (mIsDown)
            return true;
    }
    else if (event->type() == QEvent::MouseButtonPress) {

        if (obj == mStartButton) {
            mStartButton->setChecked(! mStartButton->isChecked());
            //mStartButton->setDown(! mStartButton->isDown());
            //mIsDown = mStartButton->isDown();
            return true;
        }
        else if (obj == mEndButton) {
            mEndButton->setDown(! mEndButton->isDown());
            mIsDown = mEndButton->isDown();
            return true;
        }
    }*/

    return false;
    //if ((obj == mStartButton || obj == mEndButton) && event->)
}

void SlideEditorWidget::onSliderValueChanged(int value)
{
    if (! mCurrentSlide)
        return;

    QString name(sender()->objectName());
    if (name.contains("start")) {
        if (name.contains("x"))
            mCurrentSlide->setStartX(value);
        else
            mCurrentSlide->setStartY(value);
    }
    else if(name.contains("end")) {
        if (name.contains("x"))
            mCurrentSlide->setEndX(value);
        else
            mCurrentSlide->setEndY(value);
    }

}

void SlideEditorWidget::onDurationChanged(double dur) {
    if (mCurrentSlide)
        mCurrentSlide->setDuration(dur);
}

void SlideEditorWidget::onResourceChanged(int index)
{
    if (! mCurrentSlide)
        return;

    if (index >= 0 && index < objects().size()) {
        Object* obj = objects()[index];

        if (mCurrentSlide->sceneObject() != obj) {
            mCurrentSlide->setSceneObject(obj);
            mCurrentSlide->setStartX(obj->x());
            mCurrentSlide->setStartY(obj->y());

            mStartXSlider->setValue(mCurrentSlide->startX());
            mStartYSlider->setValue(mCurrentSlide->startY());
        }
    }
}

void SlideEditorWidget::onGetObjectCoordinates()
{
    if (mCurrentSlide && mCurrentSlide->sceneObject()) {
        if (sender()->objectName() == "start") {
            mCurrentSlide->setStartX(mCurrentSlide->sceneObject()->x());
            mCurrentSlide->setStartY(mCurrentSlide->sceneObject()->y());
            mStartXSlider->setValue(mCurrentSlide->startX());
            mStartYSlider->setValue(mCurrentSlide->startY());
        } else if (sender()->objectName() == "end") {
            mCurrentSlide->setEndX(mCurrentSlide->sceneObject()->x());
            mCurrentSlide->setEndY(mCurrentSlide->sceneObject()->y());
            mEndXSlider->setValue(mCurrentSlide->endX());
            mEndYSlider->setValue(mCurrentSlide->endY());
        }
    }
}

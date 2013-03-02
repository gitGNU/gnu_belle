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

#include "object_editor_widget.h"

#include <QPushButton>
#include <QSlider>
#include <QColorDialog>
#include <QDebug>
#include <QLineEdit>
#include <QVariant>
#include <QFileDialog>

#include "lineedit.h"
#include "gotolabel.h"
#include "color_pushbutton.h"
#include "combobox.h"
#include "add_action_dialog.h"
#include "scene.h"
#include "drawing_surface_widget.h"

QHash<QObject*, Interaction::InputEvent> mWidgetToEvent = QHash<QObject*, Interaction::InputEvent>();

ObjectEditorWidget::ObjectEditorWidget(QWidget *parent) :
    PropertiesWidget(parent)
{
    QRegExpValidator * validator = new QRegExpValidator(QRegExp("^[0-9]+(%){0,1}$"), this);

    //mChooseObjectComboBox = new QComboBox(this);
    //mChooseObjectComboBox = 0;
    mNameEdit = new QLineEdit(this);
    mVisibleCheckbox = new QCheckBox(this);
    mXSpin = new QSpinBox(this);
    mYSpin = new QSpinBox(this);
    mWidthEditor = new QLineEdit(this);
    mWidthEditor->setValidator(validator);
    mWidthEditor->setObjectName("widthEditor");
    mHeightEditor = new QLineEdit(this);
    mHeightEditor->setValidator(validator);
    mHeightEditor->setObjectName("heightEditor");

    //connect(mChooseObjectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentWorkingObjectChanged(int)));
    connect(mNameEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onNameChanged(const QString&)));
    connect(mVisibleCheckbox, SIGNAL(toggled(bool)), this, SLOT(onVisibilityChanged(bool)));
    connect(mXSpin, SIGNAL(valueChanged(int)), this, SLOT(onXChanged(int)));
    connect(mYSpin, SIGNAL(valueChanged(int)), this, SLOT(onYChanged(int)));
    connect(mWidthEditor, SIGNAL(textEdited(const QString &)), this, SLOT(onSizeEdited(const QString&)));
    connect(mHeightEditor, SIGNAL(textEdited(const QString &)), this, SLOT(onSizeEdited(const QString&)));

    this->beginGroup(tr("Object"), "Object");
    //this->appendRow(tr("Current"), mChooseObjectComboBox);
    this->appendRow(tr("Name"), mNameEdit);
    this->appendRow(tr("Visible"), mVisibleCheckbox);
    this->appendRow("x", mXSpin);
    this->appendRow("y", mYSpin);
    this->appendRow(tr("Width"), mWidthEditor);
    this->appendRow(tr("Height"), mHeightEditor);
    this->endGroup();

    this->beginGroup(tr("Bounding Rect"));
    mBorderWidthSpinBox = new QSpinBox(this);
    mBorderWidthSpinBox->setMaximum(50);
    mBorderColorButton = new ColorPushButton(this);
    mRoundedRectCheckBox = new QCheckBox(this);
    mXRadiusSpinBox = new QSpinBox(this);
    mYRadiusSpinBox = new QSpinBox(this);
    mXRadiusSpinBox->setDisabled(true);
    mYRadiusSpinBox->setDisabled(true);

    connect(mBorderWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onBorderWidthChanged(int)));
    connect(mBorderColorButton, SIGNAL(colorChosen(const QColor&)), this, SLOT(onBorderColorChanged(const QColor&)));
    connect(mXRadiusSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onXRadiusValueChanged(int)));
    connect(mYRadiusSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onYRadiusValueChanged(int)));
    connect(mRoundedRectCheckBox, SIGNAL(toggled(bool)), this, SLOT(onRoundedRectToggled(bool)));
    connect(mRoundedRectCheckBox, SIGNAL(toggled(bool)), mXRadiusSpinBox, SLOT(setEnabled(bool)));
    connect(mRoundedRectCheckBox, SIGNAL(toggled(bool)), mYRadiusSpinBox, SLOT(setEnabled(bool)));

    appendRow(tr("Border Width"), mBorderWidthSpinBox);
    appendRow(tr("Border Color"), mBorderColorButton);

    beginSubGroup(tr("Rounded"), mRoundedRectCheckBox);
    appendRow(tr("X Radius"), mXRadiusSpinBox);
    appendRow(tr("Y Radius"), mYRadiusSpinBox);
    endGroup();

    endGroup();

    mImageChooser = new ChooseFileButton(ChooseFileButton::ImageFilter, this);
    mColorButton = new ColorPushButton(this);
    mOpacitySlider = new QSlider(Qt::Horizontal, this);
    mOpacitySlider->setMaximum(255);

    connect(mImageChooser, SIGNAL(fileSelected(const QString&)), this, SLOT(onImageChosen(const QString&)));
    connect(mOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    connect(mColorButton, SIGNAL(colorChosen(const QColor&)), this, SLOT(onColorChosen(const QColor&)));

    this->beginGroup(tr("Background"));
    this->appendRow(tr("Color"), mColorButton);
    this->appendRow(tr("Image"), mImageChooser);
    this->appendRow(tr("Opacity"), mOpacitySlider);
    this->endGroup();

    this->beginGroup(tr("Events"));
    mMousePressComboBox = new ComboBox(this);
    mMousePressComboBox->setObjectName("MousePressComboBox");
    connect (mMousePressComboBox, SIGNAL(addItemActivated()), this, SLOT(onAddItemActivated()));
    connect (mMousePressComboBox, SIGNAL(itemRemoved(int)), this, SLOT(onEventItemRemoved(int)));
    this->appendRow("OnMousePress", mMousePressComboBox);
    mWidgetToEvent.insert(mMousePressComboBox, Interaction::MousePress);

    mMouseReleaseComboBox = new ComboBox(this);
    mMouseReleaseComboBox->setObjectName("MouseReleaseComboBox");
    connect (mMouseReleaseComboBox, SIGNAL(addItemActivated()), this, SLOT(onAddItemActivated()));
    connect (mMouseReleaseComboBox, SIGNAL(itemRemoved(int)), this, SLOT(onEventItemRemoved(int)));
    this->appendRow("OnMouseRelease", mMouseReleaseComboBox);
    mWidgetToEvent.insert(mMouseReleaseComboBox, Interaction::MouseRelease);
    this->endGroup();

    mMouseMoveComboBox = new ComboBox(this);
    mMouseMoveComboBox->setObjectName("MouseMoveComboBox");
    connect (mMouseMoveComboBox, SIGNAL(addItemActivated()), this, SLOT(onAddItemActivated()));
    connect (mMouseMoveComboBox, SIGNAL(itemRemoved(int)), this, SLOT(onEventItemRemoved(int)));
    this->appendRow("OnMouseMove", mMouseMoveComboBox);
    mWidgetToEvent.insert(mMouseMoveComboBox, Interaction::MouseMove);
    this->endGroup();

    mCurrentObject = 0;

    resizeColumnToContents(0);
}

void ObjectEditorWidget::onXChanged(int x)
{
    if (mCurrentObject)
        mCurrentObject->setX(x);
}

void ObjectEditorWidget::onYChanged(int y)
{
    if (mCurrentObject)
        mCurrentObject->setY(y);
}

void ObjectEditorWidget::onRoundedRectToggled(bool checked)
{
    if (mCurrentObject) {
        mCurrentObject->setRoundedRect(checked);
    }
}

void ObjectEditorWidget::onXRadiusValueChanged(int v)
{
    if (mCurrentObject)
        mCurrentObject->setXRadius(v);
}

void ObjectEditorWidget::onYRadiusValueChanged(int v)
{
    if (mCurrentObject)
        mCurrentObject->setYRadius(v);
}

void ObjectEditorWidget::onSliderValueChanged(int value)
{
    if (mCurrentObject)
        mCurrentObject->setBackgroundOpacity(value);
}

void ObjectEditorWidget::onColorChosen(const QColor & color)
{
    if (mCurrentObject)
        mCurrentObject->setBackgroundColor(color);
}

void ObjectEditorWidget::updateData(Object *obj)
{
    if (obj == mCurrentObject)
        return;

    if (mCurrentObject)
        mCurrentObject->disconnect(this);

    mCurrentObject = obj;
    if (! mCurrentObject)
        return;

    connect(mCurrentObject, SIGNAL(dataChanged(const QVariantMap&)), this, SLOT(onObjectDataChanged(const QVariantMap&)));
    connect(mCurrentObject, SIGNAL(destroyed()), this, SLOT(onCurrentObjectDestroyed()));

    mBorderColorButton->blockSignals(true);

    ////mChooseObjectComboBox->clear();
    mObjectsHierarchy.clear();

    if (mCurrentObject->resource()) {
        //mChooseObjectComboBox->addItem(mCurrentObject->resource()->objectName() + tr("(resource)"));
        mObjectsHierarchy.append(mCurrentObject->resource());
    }

    //mChooseObjectComboBox->addItem(mCurrentObject->objectName() + tr("(copy)"));
    mObjectsHierarchy.append(mCurrentObject);

    mBorderWidthSpinBox->setValue(mCurrentObject->borderWidth());
    mBorderColorButton->setColor(mCurrentObject->borderColor());
    mColorButton->setText(obj->backgroundColor().name());
    mColorButton->setColor(obj->backgroundColor());
    mOpacitySlider->setValue(obj->backgroundOpacity());
    mNameEdit->setText(mCurrentObject->objectName());
    mNameEdit->setStyleSheet("");
    mNameEdit->setEnabled(mCurrentObject->editableName());
    mXSpin->setRange(-mCurrentObject->width(), Scene::width());
    mXSpin->setValue(mCurrentObject->x());
    mYSpin->setRange(-mCurrentObject->height(), Scene::height());
    mYSpin->setValue(mCurrentObject->y());
    if (mCurrentObject->percentWidth())
        mWidthEditor->setText(QString::number(mCurrentObject->percentWidth()));
    else
        mWidthEditor->setText(QString::number(mCurrentObject->width()));
    if (mCurrentObject->percentWidth())
        mHeightEditor->setText(QString::number(mCurrentObject->percentHeight()));
    else
        mHeightEditor->setText(QString::number(mCurrentObject->height()));


    QList<Action*> actions;
    QObject *object;

    mMousePressComboBox->clear();
    actions = mCurrentObject->actionsForEvent(Interaction::MousePress);
    for(int i=0; i < actions.size(); i++) {
        object = actions[i];
        mMousePressComboBox->addItem(actions[i]->icon(), actions[i]->toString(), qVariantFromValue(object));
    }

    mMouseReleaseComboBox->clear();
    actions = mCurrentObject->actionsForEvent(Interaction::MouseRelease);
    for(int i=0; i < actions.size(); i++) {
        object = actions[i];
        mMouseReleaseComboBox->addItem(actions[i]->icon(), actions[i]->toString(), qVariantFromValue(object));
    }

    mMouseMoveComboBox->clear();
    actions = mCurrentObject->actionsForEvent(Interaction::MouseMove);
    for(int i=0; i < actions.size(); i++) {
        object = actions[i];
        mMouseMoveComboBox->addItem(actions[i]->icon(), actions[i]->toString(), qVariantFromValue(object));
    }

    mRoundedRectCheckBox->setChecked(mCurrentObject->roundedRect());
    mXRadiusSpinBox->setValue(mCurrentObject->xRadius());
    mYRadiusSpinBox->setValue(mCurrentObject->yRadius());

    mVisibleCheckbox->setChecked(mCurrentObject->visible());

    mBorderColorButton->blockSignals(false);

}

void ObjectEditorWidget::onAddItemActivated()
{
    if (! mCurrentObject)
        return;

    Interaction::InputEvent event = mWidgetToEvent.value(sender());
    AddActionDialog dialog(event);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted && dialog.selectedAction()) {
        Action* action = dialog.selectedAction();
        action->setSceneObject(mCurrentObject);
        action->setParent(mCurrentObject);
        mCurrentObject->appendEventAction(event, action);
        ComboBox *comboBox = qobject_cast<ComboBox*>(sender());
        if(comboBox) {
            comboBox->addItem(action->icon(), action->toString(), qVariantFromValue(static_cast<QObject*>(action)));
        }
    }
}


void ObjectEditorWidget::onNameChanged(const QString & name)
{
    if (! mCurrentObject)
        return;

    if (mCurrentObject->isValidName(name)) {
        mNameEdit->setStyleSheet("background: rgba(0, 200, 0, 100);");
        mNameEdit->setToolTip("");
        mCurrentObject->setObjectName(name);
    }
    else {
        mNameEdit->setStyleSheet("background: rgba(200, 0, 0, 100);");
        mNameEdit->setToolTip(tr("Another object already has this name. You can't have different objects with the same name."));
    }
}


void ObjectEditorWidget::onSizeEdited(const QString & text)
{
    if (! mCurrentObject)
        return;

    bool ok;
    int size = 0;
    QString value(text);
    bool percent = (text.indexOf("%") != -1);

    if (percent)
        value.remove("%");

    size = value.toInt(&ok);

    if (ok) {
        if (sender()->objectName().contains("width"))
            mCurrentObject->setWidth(size, percent);
        else
            mCurrentObject->setHeight(size, percent);
    }
}

/*bool ObjectEditorWidget::eventFilter(QObject * obj, QEvent *event)
{
    if (obj == mImagePathEdit && event->type() == QEvent::MouseButtonDblClick) {
        QString filter = tr("Images(*.png *.xpm *.jpg *.gif)");
        QString path = QFileDialog::getOpenFileName(this, tr("Choose Background Image"), QDir::currentPath(), filter);
        if (mCurrentObject && ! path.isEmpty()) {
            mCurrentObject->setBackgroundImage(path);
            mImagePathEdit->setText(path);
            mImagePathEdit->setToolTip(QString("<img src=%1></img>").arg(path));
        }
        return true;
    }

    return PropertiesWidget::eventFilter(obj, event);
}*/

void ObjectEditorWidget::onImageChosen(const QString & filepath)
{
    if (mCurrentObject)
        mCurrentObject->setBackgroundImage(filepath);
}

void ObjectEditorWidget::onVisibilityChanged(bool visible)
{
    if (mCurrentObject) {
        mCurrentObject->setVisible(visible);
    }
}

void ObjectEditorWidget::onEventItemRemoved(int index)
{
    if (! mCurrentObject || index < 0)
        return;

    QString name = sender()->objectName();

    if (name == "MousePressComboBox") {
        mCurrentObject->removeEventActionAt(Interaction::MousePress, index);
    }
    else if (name == "MouseReleaseComboBox") {
        mCurrentObject->removeEventActionAt(Interaction::MouseRelease, index);
    }
    else {
        mCurrentObject->removeEventActionAt(Interaction::MouseMove, index);
    }
}

void ObjectEditorWidget::onCurrentWorkingObjectChanged(int index)
{
    if (mObjectsHierarchy.isEmpty() || index < 0 || index >= mObjectsHierarchy.size() || index == mObjectsHierarchy.indexOf(mCurrentObject))
        return;

    mCurrentObject = mObjectsHierarchy[index];
    updateData(mCurrentObject);
    if (DrawingSurfaceWidget::instance())
        DrawingSurfaceWidget::instance()->update();
}

void ObjectEditorWidget::onBorderWidthChanged(int w)
{
    if (mCurrentObject)
        mCurrentObject->setBorderWidth(w);
}

void ObjectEditorWidget::onBorderColorChanged(const QColor & color)
{
    if (mCurrentObject)
        mCurrentObject->setBorderColor(color);
}

void ObjectEditorWidget::onObjectDataChanged(const QVariantMap & data)
{
    int w=-1, h=-1;

    mXSpin->blockSignals(true);
    mYSpin->blockSignals(true);

    if (data.contains("x") && data.value("x").canConvert(QVariant::Int))
        mXSpin->setValue(data.value("x").toInt());
    if (data.contains("y") && data.value("y").canConvert(QVariant::Int))
        mYSpin->setValue(data.value("y").toInt());
    if (data.contains("width") && data.value("width").canConvert(QVariant::String))
        w = data.value("width").toInt();
    if (data.contains("height") && data.value("height").canConvert(QVariant::String))
        h = data.value("height").toInt();

    setObjectSize(w, h);

    mXSpin->blockSignals(false);
    mYSpin->blockSignals(false);
}

void ObjectEditorWidget::setObjectSize(int w, int h)
{
    if (! mCurrentObject)
        return;

    mWidthEditor->blockSignals(true);
    mHeightEditor->blockSignals(true);

    if (w != -1) {
        if(mCurrentObject->percentWidth())
            mWidthEditor->setText(QString::number(mCurrentObject->percentWidth())+"%");
        else
            mWidthEditor->setText(QString::number(w));
    }

    if (h != -1) {
        if(mCurrentObject->percentHeight())
            mHeightEditor->setText(QString::number(mCurrentObject->percentHeight())+"%");
        else
            mHeightEditor->setText(QString::number(h));
    }

    mWidthEditor->blockSignals(false);
    mHeightEditor->blockSignals(false);
}

void ObjectEditorWidget::onCurrentObjectDestroyed()
{
    mCurrentObject = 0;
}

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

#ifndef SCENE_OBJECT_PROPERTIES_WIDGET_H
#define SCENE_OBJECT_PROPERTIES_WIDGET_H

#include <QHash>
#include <QCheckBox>
#include <QSpinBox>

//#include "action_info_manager.h"
#include "combobox.h"
#include "color_pushbutton.h"
#include "properties_widget.h"
#include "object.h"
#include "choosefilebutton.h"

class Object;

class ObjectEditorWidget : public PropertiesWidget
{
    Q_OBJECT

    QList<Object*> mObjectsHierarchy;

public:
    explicit ObjectEditorWidget(QWidget *parent = 0);
    virtual void updateData(Object*);
    //virtual bool eventFilter(QObject *, QEvent *);
    void setObjectSize(int w=-1, int h=-1);

signals:

public slots:
    void onColorChosen(const QColor&);
    void onSliderValueChanged(int);
    void onOpacityChanged(int);
    void onAddItemActivated();
    void onItemActivated(int);
    void onVisibilityChanged(bool);
    void onBorderWidthChanged(int);
    void onBorderColorChanged(const QColor&);


private slots:
    void onNameChanged(const QString&);
    void onSizeEdited(const QString&);
    void onEventItemRemoved(int);
    void onCurrentWorkingObjectChanged(int);
    void onImageChosen(const QString&);
    void onXChanged(int);
    void onYChanged(int);
    void onCornerRadiusValueChanged(int);
    void onObjectDataChanged(const QVariantMap&);
    void onCurrentObjectDestroyed();
    void onKeepAspectRatioToggled(bool);

private:
    ColorPushButton* mColorButton;
    QSlider* mBackgroundOpacitySlider;
    QSlider* mOpacitySlider;
    Object *mCurrentObject;
    Object *mResourceObject;
    QComboBox* mChooseObjectComboBox;
    QLineEdit* mNameEdit;
    ComboBox * mMousePressComboBox;
    ComboBox * mMouseReleaseComboBox;
    ComboBox * mMouseMoveComboBox;
    ChooseFileButton* mImageChooser;
    QSpinBox* mCornerRadiusSpinBox;
    QCheckBox* mVisibleCheckbox;
    QSpinBox* mXSpin;
    QSpinBox* mYSpin;
    QLineEdit* mWidthEditor;
    QLineEdit* mHeightEditor;
    QSpinBox* mBorderWidthSpinBox;
    ColorPushButton* mBorderColorButton;
    QCheckBox* mKeepAspectRatioCheckbox;
};

#endif // SCENE_OBJECT_PROPERTIES_WIDGET_H

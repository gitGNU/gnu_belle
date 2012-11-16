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

#include "textbox_editor_widget.h"

#include <QTextEdit>
#include <QPushButton>
#include <QDebug>

TextPropertiesWidget::TextPropertiesWidget(QWidget *parent) :
    ObjectEditorWidget(parent)
{

    mColorButton = new ColorPushButton(this);

    QColor color = this->palette().color(QPalette::WindowText);
    mColorButton->setText(color.name());
    mLeftPaddingSpinBox = new QSpinBox(this);
    mTopPaddingSpinBox = new QSpinBox(this);
    mTextEdit = new QTextEdit(this);
    mHorizontalAlignmentComboBox = new QComboBox(this);
    mHorizontalAlignmentComboBox->addItem(tr("Center"), "hcenter");
    mHorizontalAlignmentComboBox->addItem(tr("Right"), "right");
    mHorizontalAlignmentComboBox->addItem(tr("Left"), "left");
    mVerticalAlignmentComboBox = new QComboBox(this);
    mVerticalAlignmentComboBox->addItem(tr("Center"), "vcenter");
    mVerticalAlignmentComboBox->addItem(tr("Bottom"), "bottom");
    mVerticalAlignmentComboBox->addItem(tr("Top"), "top");

    this->beginGroup("Text");
    this->appendRow(tr("Text"), mTextEdit);
    this->appendRow(tr("Color"), mColorButton);
    appendRow(tr("Left Padding"), mLeftPaddingSpinBox);
    appendRow(tr("Top Padding"), mTopPaddingSpinBox);
    appendRow(tr("Horizontal Alignment"), mHorizontalAlignmentComboBox);
    appendRow(tr("Vertical Alignment"), mVerticalAlignmentComboBox);
    this->endGroup();

    connect(mLeftPaddingSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLeftPaddingValueChanged(int)));
    connect(mTopPaddingSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onTopPaddingValueChanged(int)));
    connect(mTextEdit, SIGNAL(textChanged()), this, SLOT(onTextEditDataChanged()));
    connect(mColorButton, SIGNAL(colorChosen(const QColor&)), this, SLOT(onColorChosen(const QColor&)));
    connect(mHorizontalAlignmentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlignmentChanged(int)));
    connect(mVerticalAlignmentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlignmentChanged(int)));

    mTextEdit->setMaximumHeight(mTextEdit->height()/2);

    mCurrentObject = 0;
}

void TextPropertiesWidget::onLeftPaddingValueChanged(int v)
{
    if (mCurrentObject)
        mCurrentObject->setTextPadding("left", v);
}

void TextPropertiesWidget::onTopPaddingValueChanged(int v)
{
    if (mCurrentObject)
        mCurrentObject->setTextPadding("top", v);
}

void TextPropertiesWidget::onTextEditDataChanged()
{
    if (mCurrentObject)
        mCurrentObject->setText(mTextEdit->toPlainText());
}

void TextPropertiesWidget::onColorChosen(const QColor& color)
{
    if (mCurrentObject)
        mCurrentObject->setTextColor(color);
}

void TextPropertiesWidget::updateData(Object *obj)
{
    if (obj == mCurrentObject)
        return;

    ObjectEditorWidget::updateData(obj);

    mCurrentObject = qobject_cast<TextBox*>(obj);
    if (! mCurrentObject)
        return;

    mHorizontalAlignmentComboBox->blockSignals(true);
    mVerticalAlignmentComboBox->blockSignals(true);

    mHorizontalAlignmentComboBox->setCurrentIndex(mHorizontalAlignmentComboBox->count()-1);
    mVerticalAlignmentComboBox->setCurrentIndex(mVerticalAlignmentComboBox->count()-1);

    QStringList alignment = mCurrentObject->textAlignmentAsString().toLower().split("|");

    if (alignment.size() >= 1) {
        int index = mHorizontalAlignmentComboBox->findData(alignment[0], Qt::UserRole, Qt::MatchFixedString);
        if (index >= 0)
            mHorizontalAlignmentComboBox->setCurrentIndex(index);
    }

    if (alignment.size() >= 2) {
        int index = mVerticalAlignmentComboBox->findData(alignment[1], Qt::UserRole, Qt::MatchFixedString);
        if (index >= 0)
            mVerticalAlignmentComboBox->setCurrentIndex(index);
    }

    mHorizontalAlignmentComboBox->blockSignals(false);
    mVerticalAlignmentComboBox->blockSignals(false);

    mCurrentObject->setPlaceholderText("");
    mTextEdit->setText(mCurrentObject->text());
    mColorButton->setText(mCurrentObject->textColor().name());
    mColorButton->setColor(mCurrentObject->textColor());
    mLeftPaddingSpinBox->setValue(mCurrentObject->textPadding("left"));
    mTopPaddingSpinBox->setValue(mCurrentObject->textPadding("top"));
}

void TextPropertiesWidget::onAlignmentChanged(int index)
{
    QComboBox* senderCombo = static_cast<QComboBox*>(this->sender());

    if (index < 0 || index >= senderCombo->count() || ! mCurrentObject)
        return;

    Qt::Alignment alignment = horizontalAlignment();
    alignment |= verticalAlignment();
    mCurrentObject->setTextAlignment(alignment);
}

Qt::Alignment TextPropertiesWidget::horizontalAlignment()
{
    Qt::Alignment alignment;

    QVariant data = mHorizontalAlignmentComboBox->itemData(mHorizontalAlignmentComboBox->currentIndex());
    QString align = data.toString();

    if (align == "hcenter")
        alignment = Qt::AlignHCenter;
    else if (align == "right")
        alignment = Qt::AlignRight;
    else
        alignment = Qt::AlignLeft;

    return alignment;
}


Qt::Alignment TextPropertiesWidget::verticalAlignment()
{
    Qt::Alignment alignment;

    QVariant data = mVerticalAlignmentComboBox->itemData(mVerticalAlignmentComboBox->currentIndex());
    QString align = data.toString();

    if (align == "vcenter")
        alignment = Qt::AlignVCenter;
    else if (align == "bottom")
        alignment = Qt::AlignBottom;
    else
        alignment = Qt::AlignTop;

    return alignment;
}

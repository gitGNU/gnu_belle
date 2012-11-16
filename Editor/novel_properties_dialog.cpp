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

#include "novel_properties_dialog.h"

#include <QFileDialog>
#include <QDebug>

#include "engine.h"

NovelPropertiesDialog::NovelPropertiesDialog(QVariantMap& data, QWidget *parent) :
    QDialog(parent)
{
    mUi.setupUi(this);
    mNovelData = data;
    QIntValidator *validator = new QIntValidator(this);
    mUi.widthCombo->setValidator(validator);
    mUi.heightCombo->setValidator(validator);
    mUi.titleEdit->setText(data.value("title").toString());
    mUi.widthCombo->setEditText(data.value("width").toString());
    mUi.heightCombo->setEditText(data.value("height").toString());
    mUi.fontFamilyChooser->setEditText(data.value("fontFamily").toString());
    mUi.fontSizeSpinner->setValue(data.value("fontSize").toInt());
    mUi.engineDirectoryEdit->setText(Engine::path());

    connect(mUi.widthCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onWidthChanged(int)));
    connect(mUi.widthCombo, SIGNAL(editTextChanged(const QString&)), this, SLOT(onSizeEdited(const QString&)));
    connect(mUi.heightCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onHeightChanged(int)));
    connect(mUi.heightCombo, SIGNAL(editTextChanged(const QString&)), this, SLOT(onSizeEdited(const QString&)));
    connect(mUi.titleEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleEdited(const QString&)));
    connect(mUi.fontFamilyChooser, SIGNAL(editTextChanged(const QString&)), this, SLOT(onFontFamilyChosen(const QString&)));
    connect(mUi.fontFamilyChooser, SIGNAL(activated(const QString&)), this, SLOT(onFontFamilyChosen(const QString&)));
    connect(mUi.fontSizeSpinner, SIGNAL(valueChanged(int)), this, SLOT(onFontSizeChanged(int)));
    connect(mUi.engineDirectoryButton, SIGNAL(clicked()), this, SLOT(onEnginePathChangeRequest()));
}

void NovelPropertiesDialog::onWidthChanged(int index)
{
    mUi.heightCombo->setCurrentIndex(index);
    mNovelData.insert("width",  mUi.widthCombo->currentText().toInt());
    mNovelData.insert("height", mUi.heightCombo->currentText().toInt());
}

void NovelPropertiesDialog::onHeightChanged(int index)
{
    mUi.widthCombo->setCurrentIndex(index);
    mNovelData.insert("width",  mUi.widthCombo->currentText().toInt());
    mNovelData.insert("height", mUi.heightCombo->currentText().toInt());
}

void NovelPropertiesDialog::onSizeEdited(const QString & text)
{
    if (sender()->objectName().contains("width"))
        mNovelData.insert("width", text.toInt());
    else if (sender()->objectName().contains("height"))
        mNovelData.insert("height", text.toInt());
}

void NovelPropertiesDialog::onTitleEdited(const QString & title)
{
    if (title.isEmpty()) {
        mUi.titleEdit->setText(mNovelData.value("title").toString());
        return;
    }

    mNovelData.insert("title", title);
}

QVariantMap NovelPropertiesDialog::novelData()
{
    return mNovelData;
}

void NovelPropertiesDialog::onFontFamilyChosen(const QString & family)
{
    mNovelData.insert("fontFamily", family);
}

void NovelPropertiesDialog::onFontSizeChanged(int size)
{
    mNovelData.insert("fontSize", size);
}

void NovelPropertiesDialog::onEnginePathChangeRequest()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose engine directory"), QDir::currentPath());

    if (! path.isEmpty() && Engine::isValidPath(path)) {
        mUi.engineDirectoryEdit->setText(path);
        Engine::setPath(path);
    }
}

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
#include <QDesktopServices>
#include <QFontDatabase>

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
    mUi.fontFamilyChooser->setCurrentFontFamily(mNovelData.value("fontFamily").toString());
    mUi.fontSizeSpinner->setValue(data.value("fontSize").toInt());
    setEnginePath(Engine::path());
    mUi.textSpeedSlider->setValue(data.value("textSpeed").toInt());
    mUi.textSpeedValueLabel->setText(data.value("textSpeed").toString());
    mUi.browserEdit->setText(Engine::browserPath());
    mUi.browserEdit->setPlaceholderText("Default");

    connect(mUi.widthCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onWidthChanged(int)));
    connect(mUi.widthCombo, SIGNAL(editTextChanged(const QString&)), this, SLOT(onSizeEdited(const QString&)));
    connect(mUi.heightCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onHeightChanged(int)));
    connect(mUi.heightCombo, SIGNAL(editTextChanged(const QString&)), this, SLOT(onSizeEdited(const QString&)));
    connect(mUi.titleEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onTitleEdited(const QString&)));
    connect(mUi.fontFamilyChooser, SIGNAL(editTextChanged(const QString&)), this, SLOT(onFontFamilyChosen(const QString&)));
    connect(mUi.fontFamilyChooser, SIGNAL(activated(const QString&)), this, SLOT(onFontFamilyChosen(const QString&)));
    connect(mUi.fontSizeSpinner, SIGNAL(valueChanged(int)), this, SLOT(onFontSizeChanged(int)));
    connect(mUi.textSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(updateTextSpeedSliderTooltip(int)));
    connect(mUi.engineDirectoryButton, SIGNAL(clicked()), this, SLOT(onEnginePathChangeRequest()));
    connect(mUi.browserButton, SIGNAL(clicked()), this, SLOT(onBrowserSelect()));
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
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose engine directory"), QDir::homePath());
    setEnginePath(path);
}

void NovelPropertiesDialog::setEnginePath(const QString & path)
{
    mUi.engineDirectoryEdit->setText(path);

    if (Engine::isValidPath(path)) {
        mUi.engineDirectoryEdit->setStyleSheet("background-color: rgba(0, 255, 0, 100);");
        mUi.engineDirectoryEdit->setToolTip(tr("Valid engine folder"));
    }
    else {
        mUi.engineDirectoryEdit->setToolTip(tr("Not a valid engine folder"));
        mUi.engineDirectoryEdit->setStyleSheet("background-color: rgba(255, 0, 0, 100);");
    }

}

void NovelPropertiesDialog::updateTextSpeedSliderTooltip(int val)
{
    mUi.textSpeedValueLabel->setText(QString::number(val));
}

void NovelPropertiesDialog::onBrowserSelect()
{
    QString filter = "";
    QString startPath = QDesktopServices::storageLocation(QDesktopServices::ApplicationsLocation);

#if defined(Q_WS_X11)
    if (startPath.isEmpty()) {
        if (QFile::exists("/usr/bin"))
            startPath = "/usr/bin/";
        else if (QFile::exists("/usr/local/bin"))
            startPath = "/usr/local/bin";
    }

#elif defined(Q_WS_WIN)
    filter = tr("Executables (*.exe)");
#elif defined(Q_WS_MACX)
    if (startPath.isEmpty() && QFile::exists("/Applications"))
        startPath = "/Applications";
#endif

    if (startPath.isEmpty())
        startPath = QDir::homePath();

    QString path = QFileDialog::getOpenFileName(this, tr("Select your preferred browser"), startPath, filter);
    mUi.browserEdit->setText(path);
}

QString NovelPropertiesDialog::enginePath()
{
    return mUi.engineDirectoryEdit->text();
}

QString NovelPropertiesDialog::browserPath()
{
    return mUi.browserEdit->text();
}

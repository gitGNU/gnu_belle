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

#include "add_character_widget.h"
#include "extendedlineedit.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QSizePolicy>
#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QDir>

AddCharacterWidget::AddCharacterWidget(QWidget* parent) :
    WidgetGroup(Qt::Vertical, parent)
{
    beginLayout(Qt::Horizontal);
    addWidget(new QLabel(tr("Name: "), this), true, QSizePolicy::Fixed);
    mNameEdit = new QLineEdit(this);
    connect(mNameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onNameEdited(const QString&)));
    addWidget(mNameEdit, true, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    endLayout();
    addSelectImageWidget();
    mDeleteIcon = QIcon(":/media/delete.png");
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

void AddCharacterWidget::addSelectImageWidget() {
    beginLayout(Qt::Horizontal);
    addWidget(new QLabel(tr("State: "), this), true, QSizePolicy::Fixed);
    mStatusEdit = new QLineEdit(this);
    connect(mStatusEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onStatusEdited(const QString&)));
    addWidget(mStatusEdit, true, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    mPbSelectImage = new QPushButton(tr("Select Image"), this);
    connect(mPbSelectImage, SIGNAL(clicked()), this, SLOT(onPbSelectImageClicked()));
    addWidget(mPbSelectImage , true, QSizePolicy::Fixed);
    endLayout();

    mPbSelectImage->setDisabled(true);
}

void AddCharacterWidget::onStatusEdited(const QString& text) {
    if( text.isEmpty())
        mPbSelectImage->setDisabled(true);
    else
        mPbSelectImage->setEnabled(true);
}


void AddCharacterWidget::onNameEdited(const QString & text)
{
    mName = text;
    if (text.isEmpty())
        emit widgetState(false);
    else if (! mStateToPath.isEmpty())
        emit widgetState(true);
}

void AddCharacterWidget::onDeleteCharacterImage()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if ( ! button )
        return;

    QString state("");

    if (mPushButtonToState.contains(button)) {
        state = mPushButtonToState.value(button);
        mPushButtonToState.remove(button);
    }

    removeLayoutWithWidget(button);

    if (mStateToPath.contains(state))
        mStateToPath.remove(state);

    if (mStateToPath.isEmpty())
        emit widgetState(false);
}

void AddCharacterWidget::onCreateCharacter()
{
}

void AddCharacterWidget::onPbSelectImageClicked()
{
    QString filter = tr("Images(*.png *.xpm *.jpg *.gif)");
    QString path = QFileDialog::getOpenFileName(this, tr("Choose Image"), QDir::currentPath(), filter);

    if (QFile::exists(path)) {
        mStateToPath.insert(mStatusEdit->text(), path);

        QString labelText = QFileInfo(path).fileName() + " (" +  mStatusEdit->text() + ")";
        QLabel *label = new QLabel(labelText, this);
        QString imgHtml = QString("<img src='%1' >").arg(path);
        label->setToolTip(imgHtml);

        QPushButton *pb = new QPushButton(mDeleteIcon, "", this);
        mPushButtonToState.insert(pb, mStatusEdit->text());
        connect(pb, SIGNAL(clicked()), this, SLOT(onDeleteCharacterImage()));

        removeLastMainLayout();

        beginLayout(Qt::Horizontal);
        addWidget(label);
        addWidget(pb, true, QSizePolicy::Fixed);
        endLayout();

        addSelectImageWidget();
        emit imageSelected(path);

        if (! mName.isEmpty())
            emit widgetState(true);
    }
}

void AddCharacterWidget::addImage(const QString& path)
{
    if (QFile::exists(path)) {
        mStateToPath.insert(mStatusEdit->text(), path);

        QString labelText = QFileInfo(path).fileName() + " (" +  mStatusEdit->text() + ")";
        QLabel *label = new QLabel(labelText, this);
        QString imgHtml = QString("<img src='%1' >").arg(path);
        label->setToolTip(imgHtml);

        QPushButton *pb = new QPushButton(mDeleteIcon, "", this);
        mPushButtonToState.insert(pb, mStatusEdit->text());
        connect(pb, SIGNAL(clicked()), this, SLOT(onDeleteCharacterImage()));

        removeLastMainLayout();

        beginLayout(Qt::Horizontal);
        addWidget(label);
        addWidget(pb, true, QSizePolicy::Fixed);
        endLayout();

        addSelectImageWidget();
        emit imageSelected(path);

        if (! mName.isEmpty())
            emit widgetState(true);
    }
}

QString AddCharacterWidget::name()
{
    return mName;
}

QHash<QString, QString> AddCharacterWidget::statesAndImagePaths()
{
    return mStateToPath;
}


QLineEdit* AddCharacterWidget::nameEdit()
{
    return mNameEdit;
}

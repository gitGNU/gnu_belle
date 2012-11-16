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

#include "textbox.h"

#include <QDebug>
#include <QTextCodec>

#include "utils.h"

static TextPropertiesWidget* mEditorWidget = 0;

TextBox::TextBox(QObject *parent) :
    Object(parent)
{
    mSceneRect.setY(Scene::height()-(Scene::height()/3));
    mSceneRect.setHeight(Scene::height()/3);
    mSceneRect.setWidth(Scene::width());
    init("");
}

TextBox::TextBox(const QString& text, QObject* parent):
    Object(parent)
{
    mSceneRect.setY(Scene::height()-(Scene::height()/3));
    mSceneRect.setHeight(Scene::height()/3);
    mSceneRect.setWidth(Scene::width());
    init(text);
}

TextBox::TextBox(const QVariantMap & data, QObject * parent):
    Object(data, parent)
{
    init("");

    if (data.contains("text") && data.value("text").type() == QVariant::String) {
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");

        if (codec)
            mText = codec->toUnicode(data.value("text").toByteArray());
        else
            mText = data.value("text").toString();
    }

    if (data.contains("textPadding") && data.value("textPadding").type() == QVariant::Map)
        mTextPadding = Padding(data.value("textPadding").toMap());

    if (data.contains("textColor") && data.value("textColor").type() == QVariant::List)
        mTextColor = Utils::listToColor(data.value("textColor").toList());

    if (data.contains("textAlignment") && data.value("textAlignment").type() == QVariant::String) {
        QString align = data.value("textAlignment").toString();
        Qt::Alignment alignment;

        if (align.contains("HCenter", Qt::CaseInsensitive))
            alignment |= Qt::AlignHCenter;
        else if (align.contains("Left", Qt::CaseInsensitive))
            alignment |= Qt::AlignLeft;
        else if (align.contains("Right", Qt::CaseInsensitive))
            alignment |= Qt::AlignRight;

        if (align.contains("VCenter", Qt::CaseInsensitive))
            alignment |= Qt::AlignVCenter;
        else if (align.contains("Top", Qt::CaseInsensitive))
            alignment |= Qt::AlignTop;
        else if (align.contains("Bottom", Qt::CaseInsensitive))
            alignment |= Qt::AlignBottom;

        mTextAlignment = alignment;
    }
}

void TextBox::init(const QString& text)
{

    QString newText(text);
    if (text.isNull() || text.isEmpty())
        newText = tr("The text goes here");

    mText = newText;
    mTextColor = QColor(Qt::black);
    mTextRect = sceneRect();
    mPlaceholderText = "";
    mTextAlignment = Qt::AlignLeft | Qt::AlignTop;
    setType("TextBox");
}

TextBox::~TextBox()
{
}

QRect TextBox::textRect()
{
    return mTextRect;
}

void TextBox::setTextRect(const QRect & rect)
{
    mTextRect = rect;
}

QString TextBox::text()
{
    return mText;
}

void TextBox::setText(const QString & text)
{
    mText = text;
    emit dataChanged();
}


TextPropertiesWidget* TextBox::textEditorWidget()
{
    return mEditorWidget;
}

ObjectEditorWidget* TextBox::editorWidget()
{
    return mEditorWidget;
}

void TextBox::setTextEditorWidget(TextPropertiesWidget* widget)
{
    if (! widget)
        return;

    if (mEditorWidget)
        mEditorWidget->deleteLater();

    mEditorWidget = widget;
}

QColor TextBox::textColor()
{
    return mTextColor;
}

void TextBox::setTextColor(const QColor& color)
{
    mTextColor = color;
    emit dataChanged();
}

void TextBox::move(int x, int y)
{
    mTextRect.moveTo(x, y);
    Object::move(x, y);
    alignText();
}

void TextBox::setX(int x)
{
    Object::setX(x);
    mTextRect.moveTo(this->x()+mTextPadding.left(), mTextRect.y());
}

void TextBox::setY(int y)
{
    Object::setY(y);
    mTextRect.moveTo(mTextRect.x(), this->y()+mTextPadding.top());
}


void TextBox::setWidth(int w, bool percent)
{
    Object::setWidth(w, percent);
    mTextRect.setWidth(width()-mTextPadding.left()-mTextPadding.right());
    emit dataChanged();
}

void TextBox::setHeight(int h, bool percent)
{
    Object::setHeight(h, percent);
    mTextRect.setHeight(h-mTextPadding.top()-mTextPadding.bottom());
    emit dataChanged();

}

void TextBox::alignText()
{
    /*QRect rect = sceneRect();
    QFontMetrics metrics(mFont);
    int textWidth = metrics.width(currentText());
    int textHeight = metrics.height();

    if (mTextAlignment == Qt::AlignCenter) {
        qDebug() << "center";
        int hw = rect.width() / 2;
        int hh = rect.height() / 2;
        int htw = textWidth / 2;
        int hth = textHeight / 2;

        mTextRect.setLeft(rect.x() + hw - htw);
        mTextRect.setTop(rect.y() + hh - hth);
        mTextRect.setWidth(textWidth);
        mTextRect.setHeight(textHeight);
    }


    qDebug() << mTextRect << textWidth << textHeight;*/

}

Padding TextBox::textPadding()
{
    return mTextPadding;
}

int TextBox::textPadding(const QString & side)
{
    return mTextPadding.padding(side);
}

void TextBox::setTextPadding(const Padding& padding)
{
    if (mTextPadding.top() != padding.top())
        mTextRect.setTop(sceneRect().top()+padding.top());
    if (mTextPadding.left() != padding.left())
        mTextRect.setLeft(sceneRect().left()+padding.left());

    mTextPadding = padding;
    emit dataChanged();
}

void TextBox::setTextPadding(const QString & side, int value)
{
    Padding padding = mTextPadding;
    padding.setPadding(side, value);
    setTextPadding(padding);
}

QString TextBox::placeholderText() const {
    return mPlaceholderText;
}

void TextBox::setPlaceholderText(const QString& text)
{
    mPlaceholderText = text;
    emit dataChanged();
}

void TextBox::paint(QPainter & painter)
{
    Object::paint(painter);

    QRect rect(sceneRect());
    rect.setWidth(contentWidth());
    rect.setHeight(contentHeight());
    QPen pen(mTextColor);
    painter.save();
    QFont font(Object::fontFamily());
    font.setPixelSize(Object::fontSize());
    painter.setFont(font);
    painter.setPen(pen);
    painter.drawText(rect, mTextAlignment | Qt::TextWordWrap, currentText());
    painter.restore();
}

QVariantMap TextBox::toJsonObject()
{
    QVariantMap object = Object::toJsonObject();
    QVariantList color;
    color << mTextColor.red() << mTextColor.green() << mTextColor.blue()
             << mTextColor.alpha();

    object.insert("textColor", color);
    object.insert("text", mText);
    object.insert("textPadding", mTextPadding.toJsonObject());
    object.insert("textAlignment", textAlignmentAsString());

    filterResourceData(object);

    return object;
}


QString TextBox::currentText()
{
    if (! mPlaceholderText.isEmpty())
        return mPlaceholderText;
    return mText;
}

Qt::Alignment TextBox::textAlignment()
{
    return mTextAlignment;
}

QString TextBox::textAlignmentAsString()
{
    QString textAlignment("");
    if (mTextAlignment.testFlag(Qt::AlignHCenter))
        textAlignment += "|HCenter";
    else if (mTextAlignment.testFlag(Qt::AlignLeft))
        textAlignment += "|Left";
    else if (mTextAlignment.testFlag(Qt::AlignRight))
        textAlignment += "|Right";

    if (mTextAlignment.testFlag(Qt::AlignVCenter))
        textAlignment += "|VCenter";
    else if (mTextAlignment.testFlag(Qt::AlignTop))
        textAlignment += "|Top";
    else if (mTextAlignment.testFlag(Qt::AlignBottom))
        textAlignment += "|Bottom";

    if (textAlignment.startsWith("|"))
        textAlignment = textAlignment.mid(1);

    return textAlignment;
}

void TextBox::setTextAlignment(Qt::Alignment alignment)
{
    mTextAlignment = alignment;
    emit dataChanged();
}



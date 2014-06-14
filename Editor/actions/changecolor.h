/* Copyright (C) 2012-2014 Carlos Pais
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

#ifndef CHANGECOLOR_H
#define CHANGECOLOR_H

#include "action.h"
#include "changecolor_editor_widget.h"

class ChangeColorEditorWidget;

class ChangeColor : public Action
{
    Q_OBJECT

public:
    static ActionInfo Info;

public:
    explicit ChangeColor(QObject *parent = 0);
    ChangeColor(const QVariantMap&, QObject *parent = 0);
    static ChangeColorEditorWidget* changeColorEditorWidget();
    static void setChangeColorEditorWidget(ChangeColorEditorWidget*);
    virtual ActionEditorWidget* editorWidget();
    QVariantMap toJsonObject();
    virtual QString displayText() const;

    QColor color() const;
    void setColor(const QColor&);

    int opacity() const;
    void setOpacity(int);

    void setChangeObjectColor(bool);
    bool changeObjectColor();

    void setChangeObjectBackgroundColor(bool);
    bool changeObjectBackgroundColor();

signals:
    
public slots:
    
private:
    QColor mColor;
    bool mChangeObjectColor;
    bool mChangeObjectBackgroundColor;

    void init();
};

#endif // CHANGECOLOR_H

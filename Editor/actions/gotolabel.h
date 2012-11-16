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

#ifndef GOTOLABEL_H
#define GOTOLABEL_H

#include "action.h"
#include "label.h"
#include "gotolabel_editor_widget.h"

class GoToLabelEditorWidget;

class GoToLabel : public Action
{
    Q_OBJECT

    Label * mTargetLabel;
    QString mTargetLabelName;

public:
    static ActionInfo Info;

public:
    explicit GoToLabel(const QString& name="", QObject *parent = 0);
    GoToLabel(const QVariantMap&, QObject *parent);
    static GoToLabelEditorWidget* goToLabelEditorWidget();
    static void setGoToLabelEditorWidget(GoToLabelEditorWidget*);
    virtual ActionEditorWidget* editorWidget();
    virtual QVariantMap toJsonObject();
    void setTargetLabel(Label*);
    void setTargetLabel(const QString&);
    QString targetLabelName();
    virtual void initFrom(Action*);
    virtual Action* copy();
    bool isValidLabel(const QString&);
    bool hasValidLabel();

    
signals:
    
public slots:
    
private:
    void init();
};

#endif // GOTOLABEL_H

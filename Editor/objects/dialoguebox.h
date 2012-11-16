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

#ifndef DIALOGUE_BOX_H
#define DIALOGUE_BOX_H

#include "textbox.h"
#include "objectgroup.h"

class DialogueBox : public ObjectGroup
{
    Q_OBJECT

    public:
        explicit DialogueBox(QObject* parent=0);
        DialogueBox(const QVariantMap& data, QObject* parent=0);
        ~DialogueBox();
        void setTextColor(const QColor &) {}
        void setText(const QString &, const QString &);
        void setSpeakerName(const QString&);

        virtual void paint(QPainter &);
        virtual void move(int , int);
        void setSpeakerColor(const QColor&);
        virtual QVariantMap toJsonObject();
        /*static DialogueEditorWidget* dialogueEditorWidget();
        static void setDialogueEditorWidget(DialogueEditorWidget *);
        virtual ObjectEditorWidget* editorWidget();*/

protected:
        TextBox* textBox(const QString&);

    private:
        void init();
};


#endif

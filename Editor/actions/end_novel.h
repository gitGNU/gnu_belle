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

#ifndef END_NOVEL_H
#define END_NOVEL_H

#include "action.h"

class EndNovel : public Action
{
    Q_OBJECT

public:
    static ActionInfo Info;

public:
    explicit EndNovel(QObject *parent = 0);
    ActionEditorWidget* editorWidget();
    
signals:
    
public slots:
    
};

#endif // END_NOVEL_H

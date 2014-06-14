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

#ifndef STOP_StopSound_H
#define STOP_StopSound_H

#include "action.h"
#include "stopsound_editorwidget.h"

class StopSoundEditorWidget;

class StopSound : public Action
{
    Q_OBJECT

   QString mSound;
   double mFadeTime;

public:
    static ActionInfo Info;

public:
    explicit StopSound(QObject *parent = 0);
    StopSound(const QVariantMap&, QObject *parent = 0);

    static StopSoundEditorWidget* stopSoundEditorWidget();
    static void setStopSoundEditorWidget(StopSoundEditorWidget*);
    virtual ActionEditorWidget* editorWidget();

    void setSound(const QString&);
    QString sound();

    double fadeTime();
    void setFadeTime(double);

    QVariantMap toJsonObject();

signals:

public slots:

private:
    void init();

};

#endif // STOP_StopSound_H

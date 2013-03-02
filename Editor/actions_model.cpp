/* Copyright (C) 2012, 2013 Carlos Pais 
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

#include "actions_model.h"

#include <QDebug>

#include "scene_manager.h"
#include "action.h"
#include "mimedata.h"

ActionsModel::ActionsModel(QObject *parent) :
    QStandardItemModel(parent)
{
    mCurrentAction = 0;
}

void ActionsModel::appendAction(Action* action)
{
    insertAction(rowCount(), action);
}

void ActionsModel::insertAction(int row, Action* action)
{   
    connect(action, SIGNAL(dataChanged()), this, SLOT(updateView()));
    QStandardItem *item = new QStandardItem;
    item->setEditable(false);
    insertRow(row, item);
}

bool ActionsModel::removeRow(int row, const QModelIndex &parent)
{
    Scene* scene = SceneManager::currentScene();
    if (scene) {
        QList<Action*> actions = scene->actions();
        if (row >= 0 && row < actions.size()) {
            disconnect(actions[row], SIGNAL(dataChanged()), this, SLOT(updateView()));
            if (actions[row] == mCurrentAction)
                mCurrentAction = 0;
        }
    }

    return QStandardItemModel::removeRow(row, parent);
}

Action* ActionsModel::actionForIndex(const QModelIndex & index) const
{
    Scene* scene = SceneManager::currentScene();
    if (! scene)
        return 0;

    if (index.row() >= 0 && index.row() < scene->actions().size())
        return scene->actions().at(index.row());

    return 0;
}

void ActionsModel::updateView()
{
    Action* action = qobject_cast<Action*>(sender());
    if (! action)
        return;

    Scene* scene = SceneManager::currentScene();
    if (! scene)
        return;

    int index = scene->actions().indexOf(action);
    if (index == -1)
        return;

    QModelIndex modelIndex = this->index(index, 0);

    //updates the view(s) connected
    emit dataChanged(modelIndex, modelIndex);
}

void ActionsModel::setActions(const QList<Action *> & actions)
{
    for(int i=rowCount()-1; i >= 0; --i) {
        removeRow(i);
    }

    for(int i=0; i < actions.size(); i++)
        appendAction(actions[i]);

}

bool ActionsModel::dropMimeData (const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
    if (action == Qt::IgnoreAction)
       return true;

    const MimeData *mimeData = dynamic_cast<const MimeData*> (data);
    if (! mimeData)
        return true;

    Scene* scene = SceneManager::currentScene();
    if (! scene)
        return false;

    int destRow = -1;
    if (parent.isValid())
       destRow = parent.row();

    if (destRow < 0)
       destRow = row;

    if (destRow < 0)
        destRow = this->rowCount()-1;

    QList<QObject*> objects = mimeData->objects();
    QList<Action*> actions = scene->actions();

    foreach(QObject* obj, objects) {
        Action *action = qobject_cast<Action*>(obj);
        int index = actions.indexOf(action);
        if (index == -1)
            continue;

        if (destRow >= index ) {
            actions.insert(destRow+1, action);
            insertRow(destRow+1);
            actions.removeAt(index);
        }
        else {
            actions.removeAt(index);
            actions.insert(destRow, action);
            insertRow(destRow);
        }
    }

    scene->setActions(actions);

    return true;
}

QStringList ActionsModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-objectpointers";
    return types;
}

QMimeData* ActionsModel::mimeData ( const QModelIndexList & indexes ) const
{
    MimeData * mimeData = new MimeData;

    Scene* scene = SceneManager::currentScene();
    if (! scene)
        return 0;

   /* QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {

        if (! index.isValid() || index.row() < 0 || index.row() >= mActions.size())
            continue;

        stream << index.row();
    }

    byteArray.append((char*) mActions[indexes[0].row()], sizeof(mActions[indexes[0].row()]));
    qDebug() << "byte array:" << byteArray;
    mimeData->setData("application/x-objectpointers", byteArray);
    qDebug() << "mimedata:" << mimeData->data("application/x-objectpointers").data();
    char *data = mimeData->data("application/x-objectpointers").data();
    Action *action = (Action*) data;
    qDebug() << "action" << action;*/
    QList<Action*> actions = scene->actions();
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid())
            mimeData->appendObject(actions[index.row()]);
    }

    mimeData->setData("application/x-objectpointers", QByteArray());

    return mimeData;
}

void ActionsModel::setCurrentAction(const QModelIndex & index)
{
    Action* action = actionForIndex(index);

    if (mCurrentAction)
        mCurrentAction->focusOut();

    if (action)
        action->focusIn();

    mCurrentAction = action;
}

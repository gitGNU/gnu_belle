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

#include "branch.h"

#include <QDebug>

static BranchEditorWidget* mEditorWidget = 0;
ActionInfo Branch::Info;

Branch::Branch(QObject *parent) :
    Action(parent)
{
    init();
}

Branch::Branch(const QVariantMap& data, QObject *parent) :
    Action(data, parent)
{
    init();

    QVariantList actions;

    if (data.contains("trueActions") && data.value("trueActions").type() == QVariant::List) {
        actions = data.value("trueActions").toList();
        foreach(const QVariant& actionData, actions) {
            if (actionData.type() != QVariant::Map)
                continue;
            Action* action = ActionInfoManager::typeToAction(actionData.toMap(), this);
            appendAction(action, true);
        }
    }

    if (data.contains("falseActions") && data.value("falseActions").type() == QVariant::List) {
        actions = data.value("falseActions").toList();
        foreach(const QVariant& actionData, actions) {
            if (actionData.type() != QVariant::Map)
                continue;
            Action* action = ActionInfoManager::typeToAction(actionData.toMap(), this);
            appendAction(action, false);
        }
    }


    if (data.contains("condition") && data.value("condition").type() == QVariant::String) {
        setCondition(data.value("condition").toString());
    }
}

void Branch::init()
{
    setType(Info.type);
    setName(Info.name);
    setIcon(Info.icon);
    mCondition = "";
}

void Branch::setBranchEditorWidget(BranchEditorWidget * widget)
{
   mEditorWidget = widget;
}

BranchEditorWidget* Branch::branchEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* Branch::editorWidget()
{
    return mEditorWidget;
}

QVariantMap Branch::toJsonObject()
{
    QVariantMap data = Action::toJsonObject();
    data.insert("condition", mCondition);

    QVariantList actions;
    if (! mTrueActions.isEmpty()){
        foreach(Action* action, mTrueActions)
            actions.append(action->toJsonObject());
    }
    data.insert("trueActions", actions);
    actions.clear();

    if (! mFalseActions.isEmpty()){
        foreach(Action* action, mFalseActions)
            actions.append(action->toJsonObject());
    }
    data.insert("falseActions", actions);
    actions.clear();

    return data;
}

QString Branch::condition() const
{
    return mCondition;
}

void Branch::setCondition(const QString & condition)
{
    mCondition = condition;
    QString trueAction(tr("Do Nothing"));
    QString falseAction(tr("Do Nothing"));

    if (mTrueActions.size())
        trueAction = mTrueActions[0]->toString();

    if (mFalseActions.size())
        falseAction = mFalseActions[0]->toString();

    QString text = QString("%1 %2: %3\n%4: %5").arg(tr("If")).arg(condition).arg(trueAction).arg(tr("Else")).arg(falseAction);
    setDisplayText(text);
}

QList<Action*> Branch::actions(bool cond) const
{
    if (cond)
        return mTrueActions;
    return mFalseActions;
}

void Branch::appendAction(Action* action, bool cond)
{
    if (cond)
        mTrueActions.append(action);
    else
        mFalseActions.append(action);
}

Action* Branch::action(int index, bool cond) const
{
    if (cond) {
        if (index < mTrueActions.size())
            return mTrueActions[index];
    }
    else if (index < mFalseActions.size())
        return mFalseActions[index];

    return 0;
}

void Branch::removeAction(int index, bool cond, bool del){
    Action* action = 0;

    if (cond) {
        if (index < mTrueActions.size())
            action = mTrueActions.takeAt(index);
    }
    else if (index < mFalseActions.size())
        action = mFalseActions.takeAt(index);

    if (action && del)
        action->deleteLater();
}

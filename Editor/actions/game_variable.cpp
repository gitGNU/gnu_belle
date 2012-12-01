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

#include "game_variable.h"

#include "utils.h"

#include <QDebug>

static  GameVariableEditorWidget* mEditorWidget = 0;
ActionInfo GameVariable::Info;

GameVariable::GameVariable(QObject *parent) :
    Action(parent)
{
    init();
}

GameVariable::GameVariable(const QVariantMap& data, QObject *parent) :
    Action(data, parent)
{
    init();

    if (data.contains("variable") && data.value("variable").type() == QVariant::String) {
        mVariable = data.value("variable").toString();
    }

    if (data.contains("value") && data.value("value").type() == QVariant::String) {
        mValue = data.value("value").toString();
    }

    if (data.contains("operator") && data.value("operator").type() == QVariant::String) {
        mOperatorIndex = mOperators.indexOf(data.value("operator").toString());
    }
}

void GameVariable::init()
{
    setType(Info.type);
    setName(Info.name);
    setIcon(Info.icon);

    mOperators.append("assign");
    mOperators.append("add");
    mOperators.append("subtract");
    mOperators.append("multiply");
    mOperators.append("divide");
    mOperators.append("append");

    mOperatorIndex = 0;
}

void GameVariable::setGameVariableEditorWidget(GameVariableEditorWidget * widget)
{
   mEditorWidget = widget;
}

GameVariableEditorWidget* GameVariable::gameVariableEditorWidget()
{
   return mEditorWidget;
}

ActionEditorWidget* GameVariable::editorWidget()
{
    return mEditorWidget;
}

int GameVariable::operatorIndex()
{
    return mOperatorIndex;
}

void GameVariable::setOperatorIndex(int index)
{
    if (index >= 0 && index < mOperators.size()) {
        mOperatorIndex = index;
        emit dataChanged();
    }
}

QString GameVariable::variable()
{
    return mVariable;
}

void GameVariable::setVariable(const QString & var)
{
    mVariable = var;
    emit dataChanged();
}

QString GameVariable::value()
{
    return mValue;
}

void GameVariable::setValue(const QString & val)
{
    mValue = val;
    emit dataChanged();
}

QString GameVariable::displayText() const
{
    QString variable = tr("Nothing");
    QString value = variable;
    QString op = mOperators[mOperatorIndex];

    if (! mVariable.isEmpty())
        variable = "$" + mVariable;
    if (! mValue.isEmpty()) {
        value = mValue;
        if (! Utils::isNumber(value))
            value = QString("\"%1\"").arg(value);
    }
    if (mEditorWidget){
        QStringList items = mEditorWidget->operatorsText();
        if (mOperatorIndex < items.size())
            op = items[mOperatorIndex];
    }

    return QString("%1 %2 %3 %4").arg(op)
                                 .arg(value)
                                 .arg(tr("to"))
                                 .arg(variable);
}

QVariantMap GameVariable::toJsonObject()
{
    QVariantMap data = Action::toJsonObject();

    data.insert("variable", mVariable);
    data.insert("operator", mOperators[mOperatorIndex]);
    data.insert("value", mValue);

    return data;
}

#ifndef RUNSCRIPT_H
#define RUNSCRIPT_H

#include "runscript_editorwidget.h"
#include "action.h"

class RunScriptEditorWidget;
class Action;

class RunScript : public Action
{
    Q_OBJECT

    QString mScript;
    QString mComment;

public:
    static ActionInfo Info;

public:
    explicit RunScript(QObject *parent = 0);
    RunScript(const QVariantMap& data, QObject *parent=0);
    static RunScriptEditorWidget* runScriptEditorWidget();
    static void setRunScriptEditorWidget(RunScriptEditorWidget*);
    virtual ActionEditorWidget* editorWidget();
    virtual QVariantMap toJsonObject();
    QString script() const;
    void setScript(const QString&);
    QString comment() const;
    void setComment(const QString&);
    virtual QString displayText() const;
    
signals:
    
public slots:

private:
    void init();
    
};

#endif // RUNSCRIPT_H

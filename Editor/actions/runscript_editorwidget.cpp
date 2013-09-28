#include "runscript_editorwidget.h"

RunScriptEditorWidget::RunScriptEditorWidget(QWidget *parent) :
    ActionEditorWidget(parent)
{
    mScriptEdit = new QTextEdit(this);
    QFont font;
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);
    mScriptEdit->setFont(font);
    QFontMetrics metrics(font);
    mScriptEdit->setTabStopWidth(TAB_WIDTH * metrics.width(' '));

    mCommentEdit = new QLineEdit(this);

    this->appendRow(tr("Script"), mScriptEdit);
    this->appendRow(tr("Comment"), mCommentEdit);
    this->resizeColumnToContents(0);

    connect(mScriptEdit, SIGNAL(textChanged()), this, SLOT(onScriptTextChanged()));
    connect(mCommentEdit, SIGNAL(textEdited(const QString&)), this, SLOT(onCommentTextEdited(const QString&)));

}

void RunScriptEditorWidget::updateData(Action *action)
{
    mCurrentAction = 0;
    RunScript* runScript = qobject_cast<RunScript*>(action);
    if (! runScript)
        return;


    mScriptEdit->setPlainText(runScript->script());
    mCommentEdit->setText(runScript->comment());
    mCurrentAction = runScript;
}

void RunScriptEditorWidget::onScriptTextChanged()
{
    if (mCurrentAction)
        mCurrentAction->setScript(mScriptEdit->toPlainText());
}

void RunScriptEditorWidget::onCommentTextEdited(const QString&)
{
    if (mCurrentAction)
        mCurrentAction->setComment(mCommentEdit->text());
}

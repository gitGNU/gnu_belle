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

#include "belle.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QCheckBox>
#include <QHash>
#include <QString>
#include <QFileDialog>
#include <QString>
#include <QUrl>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTextCodec>
#include <QProcess>

#include "object.h"
#include "add_character_dialog.h"
#include "dialogue.h"
#include "properties_widget.h"
#include "textbox_editor_widget.h"
#include "character.h"
#include "action_catalog_widget.h"
#include "actions_model.h"
#include "change_visibility.h"
#include "wait.h"
#include "image.h"

#include "button.h"
#include "json.h"
#include "resources_view.h"
#include "condition_dialog.h"
#include "label.h"
#include "aboutdialog.h"
#include "slide.h"
#include "novel_properties_dialog.h"
#include "utils.h"
#include "engine.h"
#include "simple_http_server.h"

Belle::Belle(QWidget *widget)
    : QMainWindow(widget)
{
    mUi.setupUi( this );

    mHttpServer.setServerPort(8000);
    mDisableClick = false;
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    //setup scenes
    Scene::setEditorWidget(new SceneEditorWidget);
    //init scene manager instance
    SceneManager::setInstance(new SceneManager(WIDTH, HEIGHT, this));
    //connect(SceneManager::instance(), SIGNAL(selectionChanged(Object*)), this, SLOT(onSelectedObjectChanged(Object*)));
    connect(SceneManager::instance(), SIGNAL(sceneRemoved(int)), this, SLOT(onSceneRemoved(int)));
    SceneManager::setClipboard(new Clipboard(SceneManager::instance()));

    //setup default data
    QVariantMap data;
    data.insert("title", tr("Untitled"));
    data.insert("width", WIDTH);
    data.insert("height", HEIGHT);
    data.insert("textSpeed", 50);
    data.insert("fontSize", 18);
    data.insert("fontFamily", QFontInfo(QFont()).family());
    setNovelProperties(data);
    Engine::guessPath();

    mUi.scenesWidget->setIconSize(QSize(64, 48));

    //create editors
    Object::setObjectEditorWidget(new ObjectEditorWidget());
    TextBox::setTextEditorWidget(new TextPropertiesWidget());
    Character::setCharacterEditorWidget(new CharacterPropertiesWidget());
    ObjectGroup::setObjectGroupEditorWidget(new ObjectGroupEditorWidget());
    ActionInfoManager::init();

    mActionsView = new ActionsView(this);
    ActionsModel * actionsModel = qobject_cast<ActionsModel*> (mActionsView->model());
    QLayout* layout = mUi.actionsViewWidget->layout();

    if (layout)
        layout->addWidget(mActionsView);

    QLayout *vLayout = centralWidget()->layout();
    QScrollArea * scrollArea = new QScrollArea(mUi.centralwidget);
    mDrawingSurfaceWidget = new DrawingSurfaceWidget(SceneManager::instance());
    connect(mDrawingSurfaceWidget, SIGNAL(selectionChanged(Object*)), this, SLOT(onSelectedObjectChanged(Object*)));
    scrollArea->setWidget(mDrawingSurfaceWidget);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->viewport()->installEventFilter(mDrawingSurfaceWidget);
    scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    vLayout->addWidget(scrollArea);

    //connect(mDrawingSurfaceWidget, SIGNAL(paintFinished()), this, SLOT(updateSceneIcon()));
    connect(mActionsView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onActionsViewClicked(const QModelIndex&)));

    //menu bar connections
    connect(mUi.propertiesAction, SIGNAL(triggered()), this, SLOT(onPropertiesTriggered()));

    //add available resource items
    mUi.twObjects->addTopLevelItem(new QTreeWidgetItem(mUi.twObjects, QStringList()<< tr("Character")));
    mUi.twObjects->addTopLevelItem(new QTreeWidgetItem(mUi.twObjects, QStringList()<< tr("Text Box")));
    mUi.twObjects->addTopLevelItem(new QTreeWidgetItem(mUi.twObjects, QStringList()<< tr("Image")));
    mUi.twObjects->addTopLevelItem(new QTreeWidgetItem(mUi.twObjects, QStringList()<< tr("Dialogue Box")));
    mUi.twObjects->addTopLevelItem(new QTreeWidgetItem(mUi.twObjects, QStringList()<< tr("Button")));

    mUi.twObjects->topLevelItem(0)->setIcon(0, QIcon(":/media/user.png"));
    mUi.twObjects->topLevelItem(1)->setIcon(0, QIcon(":/media/text.png"));
    mUi.twObjects->topLevelItem(2)->setIcon(0, QIcon(":/media/image.png"));
    mUi.twObjects->topLevelItem(3)->setIcon(0, QIcon(":/media/talk-baloon.png"));
    mUi.twObjects->topLevelItem(4)->setIcon(0, QIcon(":/media/button.png"));

    //connect(mUi.scenesWidget, SIGNAL(itemDoubleClicked (QTreeWidgetItem *, int)), this, SLOT(onmUi.scenesWidgetDoubleClicked(QTreeWidgetItem *, int)));
    connect(mUi.scenesWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(onScenesWidgetItemChanged(QTreeWidgetItem*,int)));
    connect(mUi.scenesWidget, SIGNAL(itemClicked (QTreeWidgetItem *, int)), this, SLOT(onSceneItemClicked(QTreeWidgetItem*, int)));
    connect(mUi.scenesWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onScenesWidgetCustomContextMenuRequested(const QPoint&)));
    connect(mUi.addSceneBtn, SIGNAL(clicked()), this, SLOT(addScene()));
    connect(mUi.delSceneBtn, SIGNAL(clicked()), this, SLOT(deleteScene()));
    connect(mUi.twObjects, SIGNAL(itemDoubleClicked (QTreeWidgetItem *, int)), this, SLOT(onTwObjectsDoubleClicked(QTreeWidgetItem *, int)));
    connect(mUi.runAction, SIGNAL(triggered()), this, SLOT(onRunTriggered()));

    //resources viewer
    mUi.resourcesTabWidget->setCurrentIndex(1);
    widget = mUi.resourcesTabWidget->currentWidget();
    mResourcesView = 0;
    if(widget) {
        layout = widget->layout() ? widget->layout() : new QVBoxLayout(widget);
        mResourcesView = new ResourcesView(widget);
        mResourcesView->setHeaderHidden(true);
        layout->addWidget(mResourcesView);

        connect(mResourcesView, SIGNAL(editResource(Object*)), this, SLOT(onEditResource(Object*)));
        connect(mResourcesView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onResourcesDoubleClicked(const QModelIndex&)));
    }

    mUi.resourcesTabWidget->setCurrentIndex(0);

    addScene();

    int width = mUi.actionCatalogDockWidget->width()/2;
    width += mUi.resourcesDockWidget->width() / 2;
    mUi.propertiesDockWidget->setMinimumWidth(width);

    vLayout = mUi.actionCatalogDockWidget->layout();
    if (vLayout) {
        ActionCatalogWidget *actionCatalog = new ActionCatalogWidget(mUi.actionCatalogDockWidget);
        if (actionsModel) {
            //connect(this, SIGNAL(newAction(Action*)), actionsModel, SLOT(appendAction(Action*)));
            //connect(actionCatalog, SIGNAL(newAction(Action*)), actionsModel, SLOT(appendAction(Action*)));
            connect(actionCatalog, SIGNAL(newAction(Action*)), this, SLOT(onNewAction(Action*)));
        }

        connect(actionCatalog, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onActionCatalogClicked(const QModelIndex&) ));
        vLayout->addWidget(actionCatalog);
    }


    //connect actions' signals with the respective slots
    connect(mUi.openProjectAction, SIGNAL(triggered()), this, SLOT(openFileOrProject()));
    connect(mUi.aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(mUi.exportProject, SIGNAL(triggered()), this, SLOT(exportProject()));

    //scene's buttons
    connect(mUi.upSceneBtn, SIGNAL(clicked()), this, SLOT(onSceneUpped()));
    connect(mUi.downSceneBtn, SIGNAL(clicked()), this, SLOT(onSceneDowned()));

    //scene's treewidget actions
    mCopyScene = new QAction(QIcon(":/media/editcopy.png"), tr("Copy"), mUi.scenesWidget);
    mCopyScene->setShortcut(QKeySequence::Copy);
    mCopyScene->setShortcutContext(Qt::WidgetShortcut);
    mUi.scenesWidget->addAction(mCopyScene);
    connect(mCopyScene, SIGNAL(triggered()), this, SLOT(copyScene()));

    mCutScene = new QAction(QIcon(":/media/editcut.png"), tr("Cut"), mUi.scenesWidget);
    mCutScene->setShortcut(QKeySequence::Cut);
    mCutScene->setShortcutContext(Qt::WidgetShortcut);
    mUi.scenesWidget->addAction(mCutScene);
    connect(mCutScene, SIGNAL(triggered()), this, SLOT(cutScene()));

    mPasteScene = new QAction(QIcon(":/media/editpaste.png"), tr("Paste"), mUi.scenesWidget);
    mPasteScene->setShortcut(QKeySequence::Paste);
    mPasteScene->setShortcutContext(Qt::WidgetShortcut);
    mUi.scenesWidget->addAction(mPasteScene);
    connect(mPasteScene, SIGNAL(triggered()), this, SLOT(pasteScene()));

    mDeleteScene = new QAction(QIcon(":/media/delete.png"), tr("Delete"), mUi.scenesWidget);
    mDeleteScene->setShortcut(QKeySequence::Delete);
    mDeleteScene->setShortcutContext(Qt::WidgetShortcut);
    mUi.scenesWidget->addAction(mDeleteScene);
    connect(mDeleteScene, SIGNAL(triggered()), this, SLOT(deleteScene()));

    restoreSettings();
}

void Belle::saveSettings()
{
    //save settings
    mSettings->beginGroup("Window");
    qDebug() << mUi.showActionsAction->isChecked();
    mSettings->setValue("Geometry", this->saveGeometry());
    mSettings->setValue("State", this->saveState());
    mSettings->endGroup();
    qDebug() << mUi.showActionsAction->isChecked();
    mSettings->beginGroup("Project");
    if (Engine::isValid())
        mSettings->setValue("enginePath", Engine::path());
    if (! Engine::browserPath().isEmpty())
        mSettings->setValue("browser", Engine::browserPath());
    mSettings->endGroup();
}

void Belle::restoreSettings()
{
    //load settings
    mSettings = new QSettings("Belle", "Belle", this);

    if (mSettings->contains("Window/Geometry"))
        this->restoreGeometry(mSettings->value("Window/Geometry").toByteArray());
    if (mSettings->contains("Window/State"))
        this->restoreState(mSettings->value("Window/State").toByteArray());
    if (mSettings->contains("Project/enginePath"))
        Engine::setPath(mSettings->value("Project/enginePath").toString());
    if (mSettings->contains("Project/browser"))
        Engine::setBrowserPath(mSettings->value("Project/browser").toString());
}

bool Belle::eventFilter(QObject *obj, QEvent *ev)
{
    return QMainWindow::eventFilter(obj, ev);
}

Belle::~Belle()
{
    if (TextBox::textEditorWidget())
        TextBox::textEditorWidget()->deleteLater();

    if (Object::objectEditorWidget())
        Object::objectEditorWidget()->deleteLater();

    if (Character::characterEditorWidget())
        Character::characterEditorWidget()->deleteLater();

    if (ObjectGroup::objectGroupEditorWidget())
        ObjectGroup::objectGroupEditorWidget()->deleteLater();

    if (Scene::editorWidget())
        Scene::editorWidget()->deleteLater();

    ActionInfoManager::destroy();
    ResourceManager::destroy();

    saveSettings();
}

void Belle::onEditResource(Object* object)
{
    if (! object)
        return;

    mDrawingSurfaceWidget->setObject(object);

    removeWidgetsInPropertiesWidget();
    if(object->editorWidget()) {
        object->editorWidget()->updateData(object);
        addWidgetToPropertiesWidget(object->editorWidget());
    }

    /*if (mDisableClick) {
        mDisableClick = false;
        return;
    }

    const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(index.model());
    if (! model)
        return;

    QStandardItem* item = model->itemFromIndex(index);
    if (! item)
        return;

    Object* obj = ResourceManager::resource(item->text());
    if (! obj)
        return;

    mDrawingSurfaceWidget->setObject(obj);

    removeWidgetsInPropertiesWidget();
    if(obj->editorWidget()) {
        obj->editorWidget()->updateData(obj);
        addWidgetToPropertiesWidget(obj->editorWidget());
    }*/
}

void Belle::onResourcesDoubleClicked(const QModelIndex& index)
{
    if (! SceneManager::currentScene())
        return;

    const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(index.model());
    if (! model)
        return;

    Scene* scene = SceneManager::currentScene();
    if (! scene)
        return;

    Object* resource = mResourcesView->object(index);
    if (resource) {
        QVariantMap data(resource->toJsonObject());
        Object * object = ResourceManager::instance()->createResource(data, false);
        if (object) {
            object->setResource(resource);
            scene->appendObject(object, true);
        }
    }
}

void Belle::onActionCatalogClicked(const QModelIndex& index)
{
   //mActionsView->appendAction();
}

void Belle::onScenesWidgetItemChanged(QTreeWidgetItem* item, int column)
{
    int index = mUi.scenesWidget->indexOfTopLevelItem(item);
    Scene* currScene = SceneManager::scene(index);

    if (! currScene)
        return;

    if (SceneManager::isValidSceneName(item->text(0)))
        currScene->setObjectName(item->text(0));
    else
        item->setText(0, currScene->objectName());
}

void Belle::updateActions()
{
    if (SceneManager::instance()->currentScene()) {
        ActionsModel *model = qobject_cast<ActionsModel*> (mActionsView->model());
        if (model) {
            model->setActions(SceneManager::instance()->currentScene()->actions());
        }
    }
}

void Belle::addScene(Scene* scene)
{
    //update previous scene icon
    if (SceneManager::currentScene())
        updateSceneIcon();
    if (! scene)
        scene = SceneManager::instance()->createNewScene();
    else
        SceneManager::instance()->addScene(scene);

    createSceneTreeItem(scene);
    updateSceneEditorWidget();
    updateActions();
    mDrawingSurfaceWidget->update();
}

QTreeWidgetItem* Belle::createSceneTreeItem(Scene* scene)
{
    QTreeWidgetItem * item = new QTreeWidgetItem(mUi.scenesWidget, QStringList() << scene->objectName());
    mUi.scenesWidget->blockSignals(true);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setIcon(0, scene->icon());
    mUi.scenesWidget->blockSignals(false);
    mUi.scenesWidget->setCurrentItem(item);
    mUi.scenesWidget->editItem(item);

    return item;
}

void Belle::onSceneRemoved(int index)
{
    if (index >= 0 && index < mUi.scenesWidget->topLevelItemCount())
        mUi.scenesWidget->takeTopLevelItem(index);
}

void Belle::deleteScene()
{
    if (SceneManager::instance()->count() == 1)
        return;

    int index = mUi.scenesWidget->indexOfTopLevelItem(mUi.scenesWidget->currentItem());

    if (index != -1)
        SceneManager::instance()->removeSceneAt(index, true);
}

void Belle::onSceneItemClicked(QTreeWidgetItem *item, int column)
{
    mDrawingSurfaceWidget->setObject(0);
    updateSceneIcon(); //update current scene icon
    int prevIndex = SceneManager::currentSceneIndex();
    SceneManager::instance()->setCurrentSceneIndex(mUi.scenesWidget->indexOfTopLevelItem(item));
    if (SceneManager::currentScene())
        SceneManager::currentScene()->show();
    updateSceneEditorWidget();
    if (prevIndex != SceneManager::currentSceneIndex())
        updateActions();
    mDrawingSurfaceWidget->update();
}

void Belle::updateSceneIcon(Scene* scene)
{
    scene =  scene ? scene : SceneManager::currentScene();
    //update previous icon
    if (scene) {
        scene->hide(); //focus out first to update pixmap
        QTreeWidgetItem *prevItem = mUi.scenesWidget->topLevelItem(SceneManager::indexOf(scene));
        if (prevItem)
            prevItem->setIcon(0, scene->icon());
    }
}

void Belle::updateSceneEditorWidget(Scene* scene)
{
    scene =  scene ? scene : SceneManager::currentScene();

    if (scene) {
        switchWidgetInPropertiesWidget(Scene::editorWidget());
        if (Scene::editorWidget()) {
            Scene::editorWidget()->updateData(scene);
        }
    }
}

void Belle::onTwObjectsDoubleClicked(QTreeWidgetItem *item, int column)
{
    Scene *scene = SceneManager::instance()->currentScene();
    if (! scene)
        return;

    Object *resource = 0;
    Object *object = 0;
    int accepted = 0;
    QString filter = tr("Images(*.png *.xpm *.jpg *.jpeg *.gif)");
    QString path;
    AddCharacterDialog *dialog = 0;

    switch (item->treeWidget()->indexOfTopLevelItem(item))
    {
    //character
    case 0:
        dialog = new AddCharacterDialog();
        accepted = dialog->exec();
        if (accepted) {
            resource = new Character(dialog->name(), dialog->statesAndImagePaths(), ResourceManager::instance());
            ResourceManager::instance()->addResource(resource);
        }
        break;

        //TextBox
    case 1:
        resource = new TextBox(tr("Text goes here..."), ResourceManager::instance());
        ResourceManager::instance()->addResource(resource);
        break;

        //Image
    case 2:
        path = QFileDialog::getOpenFileName(this, tr("Choose Image"), QDir::currentPath(), filter);
        if (path.isEmpty())
            break;
        resource = new Image(path, ResourceManager::instance());
        ResourceManager::instance()->addResource(resource);
        break;

       //Dialogue Box
    case 3:
        resource = new DialogueBox(ResourceManager::instance());
        ResourceManager::instance()->addResource(resource);
        break;

        //Button
    case 4:
        resource = new Button(ResourceManager::instance());
        ResourceManager::instance()->addResource(resource);
        break;
    }

    if (resource)
        connect(resource, SIGNAL(dataChanged()), mDrawingSurfaceWidget, SLOT(update()));

    if (mResourcesView && resource) {
        mUi.resourcesTabWidget->setCurrentIndex(1);
        mResourcesView->select(resource->objectName());
    }

    if (dialog)
        dialog->deleteLater();
}

void Belle::onTwObjectsClicked(QTreeWidgetItem *, int)
{
    /*QLayout *layout = mUi.dockWidgetContents_5->layout();
    WidgetGroup * gwidget = new WidgetGroup(Qt::Vertical, mUi.dockWidgetContents_5);

    gwidget->beginLayout(Qt::Horizontal);
    gwidget->addWidget(new QLabel("Name: ", gwidget), true, QSizePolicy::Fixed);
    gwidget->addWidget(new QLineEdit(gwidget));
    gwidget->endLayout();
    gwidget->beginLayout(Qt::Horizontal);
    gwidget->addWidget(new QLabel("State: ", gwidget), true, QSizePolicy::Fixed);
    gwidget->addWidget(new QLineEdit(gwidget));
    gwidget->addWidget(new QPushButton(tr("Select Image"), gwidget), true, QSizePolicy::Fixed);
    gwidget->endLayout();
    layout->addWidget(gwidget);*/
}

void Belle::onSelectedObjectChanged(Object* obj)
{
    if (obj) {
        switchWidgetInPropertiesWidget(obj->editorWidget());
        if (obj->editorWidget())
            obj->editorWidget()->updateData(obj);
    }
    else
        switchWidgetInPropertiesWidget(0);
}


void Belle::onActionsViewClicked(const QModelIndex& index)
{
    if (! index.isValid())
        return;

    const ActionsModel *model = qobject_cast<const ActionsModel*>(index.model());
    if (! model)
        return;

    Action* action = model->actionForIndex(index);
    if (! action)
        return;

    removeWidgetsInPropertiesWidget();
    if (action->editorWidget()) {
        action->editorWidget()->updateData(action);
        addWidgetToPropertiesWidget(action->editorWidget());
    }

}


void Belle::onNewAction(Action * action)
{
    if (SceneManager::instance()->currentScene()) {
        SceneManager::instance()->currentScene()->appendAction(action);
        ActionsModel * actionsModel = qobject_cast<ActionsModel*> (mActionsView->model());
        if (actionsModel)
            actionsModel->appendAction(action);
    }

}

void Belle::switchWidgetInPropertiesWidget(QWidget* widget)
{
    QWidget* currWidget = widgetInPropertiesWidget();
    if (widget == currWidget)
        return;

    removeWidgetsInPropertiesWidget();
    addWidgetToPropertiesWidget(widget);
}

void Belle::addWidgetToPropertiesWidget(QWidget* widget)
{
    QLayout *layout = mUi.dockWidgetContents_5->layout();
    if (layout && widget) {
        layout->addWidget(widget);
        if (widget->isHidden())
            widget->show();
        //widget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    }
}

QWidget* Belle::widgetInPropertiesWidget()
{
    QLayout *layout = mUi.dockWidgetContents_5->layout();
    if(layout) {
        QLayoutItem *item = layout->itemAt(0);
        if (item)
            return item->widget();
    }

    return 0;
}

void Belle::removeWidgetsInPropertiesWidget()
{
    QLayout *layout = mUi.dockWidgetContents_5->layout();
    if(layout) {
        QLayoutItem *item;
        while((item = layout->takeAt(0)) != 0) {
            if (item && item->widget()) {
                item->widget()->setParent(0);
                item->widget()->hide();
            }
        }
    }
}

void Belle::onRunTriggered()
{
    if (! checkEnginePath())
        return;

    QString exportedTo = exportProject(QDir::tempPath(), true);

    if (! exportedTo.isEmpty()) {

        mHttpServer.setServerDirectory(exportedTo);
        bool started = mHttpServer.start();
        if (! started) {
            QMessageBox::critical(this, tr("Couldn't start the server"), tr("The server couldn't find any free ports on your system, which is very odd."));
            return;
        }

        //Get browser specified by the user
        QString browserPath = Engine::browserPath();
        if (! browserPath.isEmpty())
            QProcess::startDetached(browserPath, QStringList() << mHttpServer.serverUrl());
        else //open file (html) with default application
            QDesktopServices::openUrl(QUrl(mHttpServer.serverUrl()));
    }
}

QString Belle::exportProject(const QString& _path, bool toRun)
{
    if (! Engine::isValid()) {
        QMessageBox::critical(this, tr("Invalid engine directory"), tr("Please, first set a valid engine directory through the menu Novel>Properties"));
        return "";
    }

    QString path(_path);
    if (path.isEmpty())
         path = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"));
    if (path.isEmpty())
        return "";

    QString fileName(GAME_FILENAME);
    QDir engineDir (Engine::path());
    QString title = mNovelData.value("title").toString();
    QDir projectDir(path);

    title = title.isEmpty() ? tr("Untitled") : title;

    if (! toRun || (toRun && mCurrentRunDirectory.isEmpty())) {
        title = Utils::newFileName(projectDir.absoluteFilePath(title));
        projectDir.mkdir(title);
        projectDir.cd(title);
    }

    if (toRun) {
        if (mCurrentRunDirectory.isEmpty())
            mCurrentRunDirectory = projectDir.absolutePath();
        projectDir = QDir(mCurrentRunDirectory);
    }

    //copy images and fonts in use
    ResourceManager::exportResources(projectDir);

    //copy all engine files
    QStringList fileNames = engineDir.entryList(QStringList() << "*.js" << "*.html" << "*.css", QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(const QString&fileName, fileNames) {
        QFile::copy(engineDir.absoluteFilePath(fileName), projectDir.absoluteFilePath(fileName));
    }

    //export gameFile
    exportGameFile(projectDir.absoluteFilePath(fileName));

    return projectDir.absolutePath();
    //Utils::safeCopy(QDir::current().absoluteFilePath(fileName), projectDir.absoluteFilePath(fileName));
}

void Belle::openFileOrProject()
{
    QString filters(tr("JSON Files(*.json)"));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select project file"), QDir::currentPath(), filters);

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (! file.open(QFile::ReadOnly))
        return;

    QString contents = file.readAll();
    file.close();

    bool ok;
    QVariant data = QtJson::Json::parse(contents, ok);

    if (! ok) {
        QMessageBox::warning(this, tr("ERROR"),
                            tr("There was a problem reading the choosen game file."));
        return;
    }

    if (data.type() != QVariant::Map)
        return;

    SceneManager::instance()->removeScenes(true);
    ResourceManager::instance()->removeResources(true);
    ResourceManager::setRelativePath(QFileInfo(fileName).absolutePath());
    mCurrentRunDirectory = "";

    QVariantMap object = data.toMap();
    setNovelProperties(object);
    ResourceManager::importResources(object);

    if (object.contains("scenes") && object.value("scenes").type() == QVariant::List) {
        QVariantList scenes = object.value("scenes").toList();
        Scene *scene = 0;

        for(int i=0; i < scenes.size(); i++) {
            if (scenes[i].type() == QVariant::Map) {
                scene = new Scene(scenes[i].toMap(), SceneManager::instance());
                addScene(scene);
            }
        }

        SceneManager::instance()->setCurrentSceneIndex(0);
        ResourceManager::setRelativePath("");
        updateActions();
    }
}

void Belle::exportGameFile(const QString& path)
{
    QString filepath(path);

    if (filepath.isNull() || filepath.isEmpty()) {
        QString dirpath = QFileDialog::getExistingDirectory(this, tr("Choose the directory to export the game file to"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);

        if (dirpath.isEmpty())
            return;

        filepath = QDir(dirpath).absoluteFilePath(GAME_FILENAME);
    }

    QFile file(filepath);

    if (! file.open(QFile::WriteOnly))
        return;

    QVariantMap jsonFile;
    QString font = Utils::font(mNovelData.value("fontSize").toInt(), mNovelData.value("fontFamily").toString());
    QMapIterator<QString, QVariant> it(mNovelData);
    QStringList customFonts = ResourceManager::customFonts();
    jsonFile.insert("customFonts", customFonts);

    while(it.hasNext()){
        it.next();
        jsonFile.insert(it.key(), it.value());
    }

    //remove "fontSize" and "fontFamily" attributes, they are going to be replaced by the "font" attribute
    jsonFile.insert("font", font);
    jsonFile.remove("fontSize");
    jsonFile.remove("fontFamily");
    QString display = ResourceManager::display();
    if (display == "DOM")
        jsonFile.insert("display", ResourceManager::display());

    QVariantMap res;
    for (int i=0; i < ResourceManager::instance()->resources().size(); i++) {
        res.insert(ResourceManager::instance()->resources().at(i)->objectName(), ResourceManager::instance()->resources().at(i)->toJsonObject());
    }
    jsonFile.insert("resources", res);

    QVariantList scenes;
    for (int i=0; i < SceneManager::instance()->size(); i++) {
        scenes.append(SceneManager::instance()->scene(i)->toJsonObject());
    }

    jsonFile.insert("scenes", scenes);

    file.write(QtJson::Json::serialize(jsonFile));
    file.close();
}

void Belle::showAboutDialog()
{
    QDialog *dialog = new AboutDialog();
    dialog->exec();
    dialog->deleteLater();
}

void Belle::onScenesWidgetCustomContextMenuRequested(const QPoint& point)
{
    QMenu menu;

    if (mUi.scenesWidget->itemAt(point)) {
        menu.addAction(mCopyScene);
        menu.addAction(mCutScene);
    }

    if (SceneManager::clipboard()->hasScenes())
        menu.addAction(mPasteScene);

    if (SceneManager::scenes().count() > 1)
        menu.addAction(mDeleteScene);

    menu.exec(mUi.scenesWidget->mapToGlobal(point));
}

void Belle::copyScene()
{
    QList<QObject*> scenes;
    int index = 0;
    Scene* scene = 0;

    foreach(QTreeWidgetItem* item , mUi.scenesWidget->selectedItems()) {
        index = mUi.scenesWidget->indexOfTopLevelItem(item);
        scene = SceneManager::scene(index);
        if (scene)
            scenes.append(scene);
    }

    SceneManager::instance()->clipboard()->add(scenes, Clipboard::Copy);
}

void Belle::cutScene()
{
    SceneManager::instance()->clipboard()->add(SceneManager::currentScene(), Clipboard::Cut);
}

void Belle::pasteScene()
{
    Clipboard::Operation op = SceneManager::clipboard()->operation();
    QList<Scene*> scenes = SceneManager::clipboard()->scenes();

    if (scenes.isEmpty())
        return;

    int index = mUi.scenesWidget->indexOfTopLevelItem(mUi.scenesWidget->currentItem());

    foreach(Scene* scene, scenes) {

        if (op == Clipboard::Copy) {
            SceneManager::instance()->insertScene(index, scene->copy());
        }
        else if (op == Clipboard::Cut) {
            SceneManager::instance()->insertScene(index, scene);
        }
        else
            continue;

        index += 1;
    }

    updateScenesWidget(index-1, true, true);
    SceneManager::clipboard()->clear();
}


void Belle::onSceneUpped()
{
    Scene * scene = SceneManager::currentScene();
    int currIndex = SceneManager::currentSceneIndex();

    if (! scene || currIndex == 0)
        return;

    SceneManager::instance()->insertScene(currIndex-1, scene);

    updateScenesWidget(currIndex-1, true);
}

void Belle::onSceneDowned()
{
    Scene * scene = SceneManager::currentScene();
    int currIndex = SceneManager::currentSceneIndex();
    if (! scene || currIndex == SceneManager::size()-1)
        return;

    SceneManager::instance()->insertScene(currIndex+1, scene);

    updateScenesWidget(currIndex+1, true);
}

void Belle::updateScenesWidget(int currIndex, bool select, bool edit)
{
    mUi.scenesWidget->clear();
    QList<Scene*> scenes = SceneManager::scenes();

    for(int i=0; i < scenes.size(); i++) {
       createSceneTreeItem(scenes[i]);
    }

    if (currIndex >= 0 && currIndex < scenes.size()) {
        QTreeWidgetItem* item = mUi.scenesWidget->topLevelItem(currIndex);
        if (select)
            mUi.scenesWidget->setCurrentItem(item);
        if (edit)
            mUi.scenesWidget->editItem(item);
    }

}

void Belle::onPropertiesTriggered()
{
    NovelPropertiesDialog dialog(mNovelData);

    dialog.exec();

    if (dialog.result() == QDialog::Accepted) {
        QVariantMap data = dialog.novelData();
        setNovelProperties(data);

        Engine::setPath(dialog.enginePath());
        Engine::setBrowserPath(dialog.browserPath());
    }
}

void Belle::changeProjectTitle(const QString & name)
{
     setWindowTitle("Belle - " + name);
}


bool Belle::checkEnginePath()
{
    QString path("");

    if (! Engine::isValid())
        path = QFileDialog::getExistingDirectory(this, tr("Choose engine directory"), QDir::currentPath());
    else
        path = Engine::path();

    if (! Engine::isValidPath(path)) {
        QMessageBox::critical(this, tr("Invalid engine directory"), tr("The directory you choose doesn't seem to be a valid engine directory."));
        return false;
    }

    Engine::setPath(path);
    return true;
}

void Belle::setNovelProperties(const QVariantMap& _data)
{
    QVariantMap data;
    QMapIterator<QString, QVariant> it(_data);
    while(it.hasNext()) {
        it.next();
        if (it.value() != mNovelData.value(it.key()))
            data.insert(it.key(), it.value());
    }

    if (data.contains("title") && data.value("title").type() == QVariant::String) {
        changeProjectTitle(data.value("title").toString());
        mNovelData.insert("title", data.value("title").toString());
    }

    if (data.contains("width") && data.value("width").canConvert(QVariant::Int)) {
        mNovelData.insert("width", data.value("width").toInt());
        if (SceneManager::instance())
            SceneManager::instance()->setSceneWidth(data.value("width").toInt());
    }

    if (data.contains("height") && data.value("height").canConvert(QVariant::Int)) {
        mNovelData.insert("height", data.value("height").toInt());
        if (SceneManager::instance())
            SceneManager::instance()->setSceneHeight(data.value("height").toInt());
    }

    if (data.contains("textSpeed") && data.value("textSpeed").canConvert(QVariant::Int)) {
        mNovelData.insert("textSpeed", data.value("textSpeed").toInt());
    }

    if (data.contains("font") && data.value("font").type() == QVariant::String) {
        int fontSize = Utils::fontSize(data.value("font").toString());
        QString fontFamily = Utils::fontFamily(data.value("font").toString());
        Object::setDefaultFontSize(fontSize);
        Object::setDefaultFontFamily(fontFamily);
        mNovelData.insert("fontSize", fontSize);
        mNovelData.insert("fontFamily", fontFamily);
    }

    if (data.contains("fontSize") && data.value("fontSize").canConvert(QVariant::Int)) {
        Object::setDefaultFontSize(data.value("fontSize").toInt());
        mNovelData.insert("fontSize", data.value("fontSize").toInt());
    }

    if (data.contains("fontFamily") && data.value("fontFamily").type() == QVariant::String) {
        Object::setDefaultFontFamily(data.value("fontFamily").toString());
        mNovelData.insert("fontFamily", data.value("fontFamily").toString());
    }
}

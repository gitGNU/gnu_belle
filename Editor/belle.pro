TEMPLATE = app

# Input.
isEmpty(PREFIX) {
 PREFIX = /usr
}
INCLUDEPATH += actions objects
DEPENDPATH += actions objects

TARGET = belle
TARGET.path = $$PREFIX/
CONFIG+=debug
QT += network

FORMS += mainwindow.ui\
    novel_properties_dialog.ui \
    about.ui \
    add_character.ui

HEADERS      += belle.h\
                scene_manager.h\
                scene.h\
                objects/object.h\
                curtain_widget.h \
    boxlayout.h \
    extendedlineedit.h \
    objects/textbox.h \
    objects/textbox_editor_widget.h\
    properties_widget.h \
    object_editor_widget.h \
    color_pushbutton.h \
    character_properties_widget.h \
    character.h \
    action.h \
    actions_view.h \
    actions_model.h \
    actions/wait.h \
    action_editor_widget.h \
    wait_editor_widget.h \
    actions/dialogue.h \
    actions/dialogue_editor_widget.h \
    dialoguebox.h \
    resource_manager.h \
    actions/slide.h \
    actions/slide_editor_widget.h \
    mimedata.h \
    actions/fade.h \
    actions/fade_editor_widget.h \
    action_catalog_widget.h \
    resources_combobox.h \
    actions/change_visibility.h \
    actions/change_visibility_editor_widget.h \
    actions/label.h \
    actions/gotolabel.h \
    actions/label_editor_widget.h \
    actions/gotolabel_editor_widget.h \
    objects/button.h \
    combobox.h \
    add_action_dialog.h \
    action_info_manager.h\
    json.h \
    objects/image.h \
    utils.h \
    resources_view.h \
    actions/branch.h \
    actions/branch_editor_widget.h \
    lineedit.h \
    interaction.h \
    actions/get_user_input.h \
    actions/get_user_input_editor_widget.h \
    actions/changecolor.h \
    actions/changecolor_editor_widget.h \
    condition_widget.h \
    condition_dialog.h \
    objects/objectgroup.h \
    objects/objectgroup_editor_widget.h \
    choosefilebutton.h \
    actions/playsound.h \
    actions/playsound_editorwidget.h \
    actions/stopsound.h \
    actions/stopsound_editorwidget.h \
    actions/gotoscene.h \
    actions/gotoscene_editorwidget.h \
    actions/show_menu.h \
    actions/show_menu_editorwidget.h \
    condition_text_edit.h \
    objects/menu.h \
    aboutdialog.h \
    padding.h \
    actions/show.h \
    actions/hide.h \
    actions/show_editor_widget.h \
    actions/hide_editor_widget.h \
    actions/end_novel.h \
    actions/change_game_variable.h \
    actions/change_game_variable_editor_widget.h \
    actions/change_background.h \
    actions/change_background_editorwidget.h \
    action_info.h \
    clipboard.h \
    novel_properties_dialog.h \
    scene_editor_widget.h \
    engine.h \
    drawing_surface_widget.h \
    add_character_dialog.h \
    animationimage.h \
    choosefontwidget.h \
    simple_http_server.h
                

SOURCES      += main.cpp\
                belle.cpp\
                scene_manager.cpp\
                scene.cpp\
                curtain_widget.cpp \
    boxlayout.cpp \
    extendedlineedit.cpp \
    properties_widget.cpp \
    object_editor_widget.cpp \
    color_pushbutton.cpp \
    objects/character_properties_widget.cpp \
    action.cpp \
    actions_view.cpp \
    actions_model.cpp \
    actions/wait.cpp \
    actions/action_editor_widget.cpp \
    actions/wait_editor_widget.cpp \
    actions/dialogue.cpp \
    actions/dialogue_editor_widget.cpp \
    dialoguebox.cpp \
    resource_manager.cpp \
    actions/slide.cpp \
    actions/slide_editor_widget.cpp \
    mimedata.cpp \
    actions/fade.cpp \
    actions/fade_editor_widget.cpp \
    action_catalog_widget.cpp \
    resources_combobox.cpp \
    objects/textbox.cpp \
    objects/textbox_editor_widget.cpp \
    actions/change_visibility.cpp \
    actions/change_visibility_editor_widget.cpp \
    actions/label.cpp \
    actions/gotolabel.cpp \
    actions/label_editor_widget.cpp \
    actions/gotolabel_editor_widget.cpp \
    objects/button.cpp \
    combobox.cpp \
    add_action_dialog.cpp \
    action_info_manager.cpp\
    json.cpp \
    objects/image.cpp \
    objects/character.cpp \
    objects/object.cpp \
    resources_view.cpp \
    actions/branch.cpp \
    actions/branch_editor_widget.cpp \
    lineedit.cpp \
    actions/get_user_input.cpp \
    actions/get_user_input_editor_widget.cpp \
    actions/changecolor.cpp \
    actions/changecolor_editor_widget.cpp \
    condition_widget.cpp \
    condition_dialog.cpp \
    objects/objectgroup.cpp \
    objects/objectgroup_editor_widget.cpp \
    choosefilebutton.cpp \
    actions/playsound.cpp \
    actions/playsound_editorwidget.cpp \
    actions/stopsound.cpp \
    actions/stopsound_editorwidget.cpp \
    actions/gotoscene.cpp \
    actions/gotoscene_editorwidget.cpp \
    actions/show_menu.cpp \
    actions/show_menu_editorwidget.cpp \
    condition_text_edit.cpp \
    objects/menu.cpp \
    aboutdialog.cpp \
    padding.cpp \
    actions/show.cpp \
    actions/hide.cpp \
    actions/show_editor_widget.cpp \
    actions/hide_editor_widget.cpp \
    actions/end_novel.cpp \
    actions/change_game_variable.cpp \
    actions/change_game_variable_editor_widget.cpp \
    actions/change_background.cpp \
    actions/change_background_editorwidget.cpp \
    action_info.cpp \
    clipboard.cpp \
    novel_properties_dialog.cpp \
    scene_editor_widget.cpp \
    engine.cpp \
    drawing_surface_widget.cpp \
    add_character_dialog.cpp \
    animationimage.cpp \
    choosefontwidget.cpp \
    simple_http_server.cpp

RESOURCES += media.qrc

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


var belle = belle || {};

(function(belle) {

var timeout = 50; //50 ms
var display = belle.display;
var objects = belle.objects;
var actions = belle.actions;
var loaded = false;
var game = {
    "resources" : {},
    "actions": [],
    "customFonts" : [],
    "currentScene" : null,
    "nextScene" : 0,
    "scenes" : [],
    "ready" : false,
    "nextAction" : 0,
    "width": 640,
    "height" : 480,
    "currentWidth": 640,
    "currentHeight": 480,
    "variables" : {},
    "end" : false,
    "textSpeed" : 10,
    "textDelay" : 100,
    "filename" : "game.json",
    "directory" : ""
}

var addObject = function (object)
{
    if (game.currentScene)
        game.currentScene.addObject(object);
}

var createObject = function (data)
{
    var type = data["type"];
    if (! type) {
        var resource = data["resource"];
        if ("resource" in data && resource in game.resources) {
            type = game.resources[resource].data.type;
        }
        else
            return null;
    }

    var _Object = belle.objects[type];
    
    if (! _Object) {
        log("'" + type + "' is not a valid object type.");
        return null;
    }

    return new _Object(data);
}

function getObjectPrototype(type)
{
    return objects[type];
}

function getActionPrototype(type)
{
    return actions[type];
}

function getObject(name, scene) {
    if (scene && name) {
       var objects = scene.objects;
        
       for (var i = 0; i !== objects.length; i++) {
           if (objects[i].name === name)
               return objects[i];
       }
    }
    
    return null;
}

function getResource(name)
{
    if (name && name in game.resources)
        return game.resources[name];
        
    return null;
}

function containsVariable (variable) {
    if (variable.startsWith("$"))
        variable = variable.slice(1);
    
    return (variable in game.variables);
}

function value (variable) {
    if (! containsVariable(variable))
        return "";
        
    if (variable.startsWith("$"))
        variable = variable.slice(1);
    
    return game.variables[variable];
}

function insertVariable (variable, value) {
    game.variables[variable] = value;
}

var replaceVariables = function(text)
{
    if (! text)
        return text;
    
    if (! text.contains("$"))
        return text;
    
    var validChar = /^[a-zA-Z]+[0-9]*$/g;
    var variable = "";
    var variables = [];
    var values = [];
    var appendToVariable = false;
    
    //Parse text to determine variables, which start from "$" 
    //until the end of string or until any other character that is not a letter nor a digit.
    for(var i=0; i !== text.length; i++) {
      
      if (text.charAt(i).search(validChar) == -1) {
        appendToVariable = false;          
        if (variable)
          variables.push(variable);
        variable = "";
        if(text.charAt(i) == "$")
          appendToVariable = true;
      }
        
      if (appendToVariable)
        variable += text.charAt(i);
    }
    
    //replace variables with the respective values and append them to the values list
    for(var i=0; i != variables.length; i++) {
        if (belle.containsVariable(variables[i]))
          values.push(belle.value(variables[i]));
        else //if the variable is not found, still add an empty value to the values so we have an equal number of elements in both lists
          values.push("");
    }
    
    //replace variables with the values previously extracted
    for(var i=0; i != values.length; i++) {
      text = text.replace(variables[i], values[i]);
    }
    
    return text;
}

function setGameFile(gamefile)
{
    game.filename = gamefile;
}

function setGameDirectory(dir)
{
    if (dir.length && dir[dir.length-1] !== "/")
        dir += "/";
    game.directory = dir;
}

function load()
{
    actions.init();
    display.loading();
    importgameData(game.directory+game.filename);
}

window.onload = function () {
    if (! belle) {
        alert("Couldn't start the engine. There was a problem reading one of the files.");
        return;
    }
  
    //load();
}

function initializeData(data)
{
    var obj;
    var objName;
    
    for (var member in data ) {
        if (member == "resources" || member == "scenes")
            continue;
        
        game[member] = data[member];
    }
    
    if (game.textSpeed < 0)
        game.textSpeed = 0;
    else if (game.textSpeed > 100)
        game.textSpeed = 100;
    
    game.textDelay = 1000 / game.textSpeed;
    document.title = "Belle - " + game.title;
    
    if ("display" in data && data["display"] == "DOM")
        belle.display.usingDOM = true;
    
    //init resources
    if (data["resources"]) {
        var resources = data["resources"];
        for (var name in resources) {
            obj = createObject(resources[name]);
 
             if (obj)
                game.resources[name] = obj;
             
        }
    }
    
    //load custom fonts
    /*game.customFonts = [];
    if ("customFonts" in data) {
        var fonts = data["customFonts"];
        for (var i=0; i < fonts.length; i++) {
            //store fonts' names
            if (isFontAvailable(fonts[i]))
                game.customFonts.push(getFontName(fonts[i])); 
        }
    }*/

    //init scenes
    if (data["scenes"]) {
        var scenes = data["scenes"];
        var scene;
        
        for(var i=0; i < scenes.length; i++) {
            scene = scenes[i];
            
            if (! belle.objects[scene.type]) {
                log("Invalid Scene type: " + scene.type);
                continue;
            }
            
            var sceneObject = new belle.objects[scene.type](scene);
            sceneObject.objects = [];
            sceneObject.actions = [];
            
            //instanciate objects
            if (scene.objects && scene.objects.length > 0) {
                for(var j=0; j !== scene.objects.length; j++) {
                    var object = scene.objects[j];
                    object.__parent = sceneObject;
                    obj = createObject(object);
                    if (obj) 
                        sceneObject.objects.push(obj);
                }
            }
            
            //instanciate actions
            if (scene.actions && scene.actions.length > 0) {
                var actions = scene.actions;
                for (var j=0; j < actions.length; j++) {
                    if (! belle.actions[actions[j].type]) {
                        log("Invalid Action type: " + actions[j].type);
                        continue;
                    }
                    
                    actions[j].__scene = sceneObject;
                    var actionObject = new belle.actions[actions[j].type](actions[j]);
                    sceneObject.actions.push(actionObject);
                }
            }
            
            game.scenes.push(sceneObject);
        }
    }
    
    if (game.scenes && game.scenes.length > 0 ) {
        game.currentScene = game.scenes[0];
        game.actions = game.currentScene.actions;
        game.nextScene = 1;
        game.nextAction = 0; //first action
    }
    
    isgameDataReady();
}

function isgameDataReady() {
    
    var ready = true;
    var scenes = game.scenes;
    var resources = game.resources;
    var actions;
    var objects;
    var objectsLoaded = 0;
    
    /*for(var i=0; i < game.customFonts.length; i++) {
        if (! isFontLoaded(game.customFonts[i])) {
            ready = false;
            break;
        }
    }*/
    
    for(var name in game.resources) {
        if (! resources[name].isReady()) {
            ready = false;
            break;
        }
        objectsLoaded++;
    }
    
    for(var i=0; i < scenes.length; i++) {
        if (! scenes[i].isReady()) {
            ready = false;
            break;
        }
        objectsLoaded++;
        
        actions = scenes[i].actions;
        for(var j=0; j < actions.length; j++){
            if (! actions[j].isReady()) {
                ready = false;
                break;
            }
            if (actions[j].object)
                objectsLoaded++;
        }
        
        objects = scenes[i].objects;
        for(var j=0; j < objects.length; j++){
            if (! objects[j].isReady()) {
                ready = false;
                break;
            }
            objectsLoaded++;
        }
    }

    if (! ready)
        setTimeout(isgameDataReady, 100);
    else {
        display.init();
        setTimeout(gameLoop, 1);
    }
}

function importgameData(path) {
    var xobj = new XMLHttpRequest();
    if (xobj.overrideMimeType)
        xobj.overrideMimeType("application/json");
    
    xobj.onreadystatechange = function() {
        if(xobj.readyState == 4){
            var gameData = JSON.parse(xobj.responseText);
            log("Game data loaded!");
            initializeData(gameData);
        }
    };
    
    xobj.open("GET", path, true);
    xobj.send(null);
}

//game's main loop
function gameLoop ()
{   
   
    if (game.end) {
        alert("The End");
        return;
    }
    
    if (game.currentAction == null || game.currentAction.isFinished()) {
      
        if (game.currentAction && game.currentAction.wait) {
            game.currentAction = game.currentAction.wait;
            game.currentAction.execute();
        }
        else if (game.nextAction >= game.actions.length) {
            if (game.nextScene >= game.scenes.length) {
                alert("The End");
                return;
            }
            
            display.removeObjects(game.currentScene);
            game.currentScene = game.scenes[game.nextScene];
            display.addObjects(game.currentScene);

            if (! display.usingDOM) {
                display.clear();
                if (game.currentScene)
                    game.currentScene.paint(display.bgContext);
            }
            
            game.actions = game.currentScene.actions;
            game.nextScene++;
            game.nextAction = 0;
            game.currentAction = null;
        }
        else {
            game.currentAction = game.currentScene.actions[game.nextAction];
            game.currentAction.execute();
            game.nextAction++;
        }
    }
    
    if (! display.usingDOM && display.needsRedraw()) {  
        display.draw();
    }
    
    setTimeout(gameLoop, timeout);
}

//Expose public properties
belle.setGameFile = setGameFile;
belle.setGameDirectory = setGameDirectory;
belle.load = load;
belle.addObject = addObject;
belle.createObject = createObject;
belle.getObject = getObject;
belle.getResource = getResource;
belle.containsVariable = containsVariable;
belle.value = value;
belle.replaceVariables = replaceVariables;
belle.insertVariable = insertVariable;
belle.game = game;
belle.getObjectPrototype = getObjectPrototype;
belle.getActionPrototype = getActionPrototype;

}(belle));

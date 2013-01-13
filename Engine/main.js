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

var showFps = true;
var Novel = {
    "resources" : {},
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
    "scale" : 0,
    "drawing" : false,
    "variables" : {},
    "context" : null,
    "bgContext": null,
    "drawTimeout" : 50,
    "end" : false,
    "forceRedraw": false,
    "scaleWidthFactor" : 1,
    "scaleHeightFactor": 1,
    "textSpeed" : 10,
    "textDelay" : 100,
    "usingDOM": false
}

Novel.containsVariable = function (variable) {
    if (variable.startsWith("$"))
        variable = variable.slice(1);
    
    return (variable in Novel.variables);
}

Novel.value = function (variable) {
    if (! Novel.containsVariable(variable))
        return "";
        
    if (variable.startsWith("$"))
        variable = variable.slice(1);
    
    return Novel.variables[variable];
}

window.onload = function () {
  alert("start");

  if (! window["Belle"]) {
    alert("Couldn't start the engine. There was a problem reading one of the files.");
    return;
  }
  
  //activate DOM mode if canvas is not supported
  if (! isCanvasSupported())
      Novel.usingDOM = true;

  importGameData("game.json");
}

function initializeData(data)
{
    var obj;
    var objName;
    
    for (var member in data ) {
        if ( member == "resources" || member == "scenes")
            continue;
        
        Novel[member] = data[member];
    }
    
    if (Novel.textSpeed < 0)
        Novel.textSpeed = 0;
    else if (Novel.textSpeed > 100)
        Novel.textSpeed = 100;
    
    Novel.textDelay = 1000 / Novel.textSpeed;
    
    document.title = "Belle - " + Novel.title;

    //init resources
    if (data["resources"]) {
        var resources = data["resources"];
        for (var name in resources) {
            obj = createResource(resources[name]);
 
             if (obj)
                Novel.resources[name] = obj;
        }
    }
    
    //load custom fonts
    /*Novel.customFonts = [];
    if ("customFonts" in data) {
        var fonts = data["customFonts"];
        for (var i=0; i < fonts.length; i++) {
            //store fonts' names
            if (isFontAvailable(fonts[i]))
                Novel.customFonts.push(getFontName(fonts[i])); 
        }
    }*/

    //init scenes
    if (data["scenes"]) {
        var scenes = data["scenes"];
        var scene;
        
        for(var i=0; i < scenes.length; i++) {
            scene = scenes[i];
            
            if (! Belle[scene.type]) {
                _console.log("Invalid Scene type: " + scene.type);
                continue;
            }
            
            var sceneObject = new Belle[scene.type](scene);
            sceneObject.objects = [];
            sceneObject.actions = [];
            
            //instanciate objects
            if (scene.objects && scene.objects.length > 0) {
                for(var j=0; j !== scene.objects.length; j++) {
                    var object = scene.objects[j];
                    object.__parent = sceneObject;
                    obj = createResource(object);
                    if (obj)
                        sceneObject.objects.push(obj);
                }
            }
            
            //instanciate actions
            if (scene.actions && scene.actions.length > 0) {
                var actions = scene.actions;
                for (var j=0; j < actions.length; j++) {
                    if (! window[actions[j].type]) {
                        _console.error("Invalid Action type: " + actions[j].type);
                        continue;
                    }
                    
                    actions[j].__scene = sceneObject;
                    var actionObject = new window[actions[j].type](actions[j]);
                    sceneObject.actions.push(actionObject);
                }
            }
            
            Novel.scenes.push(sceneObject);
        }
    }
    
    if (Novel.scenes && Novel.scenes.length > 0 ) {
        Novel.currentScene = Novel.scenes[0];
        Novel.actions = Novel.currentScene.actions;
        Novel.nextScene = 1;
        Novel.nextAction = 0; //first action
    }
    
    isGameDataReady();
}

function isGameDataReady() {
    
    var ready = true;
    var scenes = Novel.scenes;
    var resources = Novel.resources;
    var actions;
    var objects;
    
    /*for(var i=0; i < Novel.customFonts.length; i++) {
        if (! isFontLoaded(Novel.customFonts[i])) {
            ready = false;
            break;
        }
    }*/

    for(var property in Novel.resources) {
        if (! resources[property].isReady()) {
            ready = false;
            break;
        }
    }
    
    for(var i=0; i < scenes.length; i++) {
        if (! scenes[i].isReady()) {
            ready = false;
            break;
        }
        
        actions = scenes[i].actions;
        for(var j=0; j < actions.length; j++){
            if (! actions[j].isReady()) {
                ready = false;
                break;
            }
        }
        
        objects = scenes[i].objects;
        for(var j=0; j < objects.length; j++){
            if (! objects[j].isReady()) {
                ready = false;
                break;
            }
        }
    }
    
    if (! ready)
        setTimeout(isGameDataReady, 1000);
    else {
        //document.getElementById("loading").style.display = 'none';
        initDisplay();
        setTimeout(gameLoop, 1);
    }
}

function removeObjects(scene)
{
    if (! Novel.usingDOM)
        return;
    
    var container = document.getElementById("container"); 
    var objects = scene.objects;
    if (scene.element)
        container.removeChild(scene.element);
    
    for(var j=0; j < objects.length; j++){
        container.removeChild(objects[j].element);
    }
}

function addObjects(scene)
{
    if (! Novel.usingDOM)
        return;
    var container = document.getElementById("container");
    var objects = scene.objects;
    addObject(scene);
    for(var j=0; j < objects.length; j++){
        addObject(objects[j], container);
    }
}

function addObject(object, container)
{
    if (! Novel.usingDOM)
        return;
    
    if (! container)
        container = document.getElementById("container");
    if (object.visible)
        object.element.style.display = "block";
    container.appendChild(object.element);
}

function importGameData(path) {
    var xobj = new XMLHttpRequest();
    if (xobj.overrideMimeType)
        xobj.overrideMimeType("application/json");
    
    xobj.onreadystatechange = function() {
        if(xobj.readyState == 4){
            var gameData = JSON.parse(xobj.responseText);
            _console.log("Game data loaded!");
            initializeData(gameData);
        }
    };
    
    xobj.open("GET", path, true);
    xobj.send(null);
}

//game's main loop
function gameLoop ()
{   
    if (Novel.currentAction == null || Novel.currentAction.isFinished()) {
      
        if (Novel.end) {
            alert("The End");
            return;
        }else if (Novel.currentAction && Novel.currentAction.wait) {
            Novel.currentAction = Novel.currentAction.wait;
            Novel.currentAction.execute();
        }
        else if (Novel.nextAction >= Novel.actions.length) {
            if (Novel.nextScene >= Novel.scenes.length) {
                alert("The End");
                return;
            }
            removeObjects(Novel.currentScene);
            Novel.currentScene = Novel.scenes[Novel.nextScene];
            addObjects(Novel.currentScene);
            if (! Novel.usingDOM) {
                Novel.context.clearRect(0, 0, Novel.width, Novel.height);
                if (Novel.currentScene)
                    Novel.currentScene.paint(Novel.bgContext);
            }
            Novel.actions = Novel.currentScene.actions;
            Novel.nextScene++;
            Novel.nextAction = 0;
            Novel.currentAction = null;
        }
        else {
            Novel.currentAction = Novel.currentScene.actions[Novel.nextAction];
            var object = Novel.currentAction.object;
            if (object)
                object.visible = true;
            
            Novel.currentAction.execute();
            Novel.nextAction++;
        }
    }
    
    if (! Novel.usingDOM)
        if (needsRedraw() || showFps) {
            requestAnimationFrame(draw);
        }
    
    setTimeout(gameLoop, Novel.drawTimeout);
}

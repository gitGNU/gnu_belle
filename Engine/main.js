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
var stateMachine = belle.stateMachine;
var loaded = false;

belle.extend = function (sub, base) 
{
    // Copy the prototype from the base to setup inheritance
    surrogateCtor.prototype = base.prototype;
    // Tricky huh?
    sub.prototype = new surrogateCtor();
    // Remember the constructor property was set wrong, let's fix it
    sub.prototype.constructor = sub;
}

belle.isInstance = function(object, constructor) {
  
  while (object != null) {
    if (object == constructor.prototype)
      return true;
    object = Object.getPrototypeOf(object);
  }
  return false;
}

belle.serialize = function(obj)
{
  var prop;
  
  if (! obj)
    return null;
  
  if (typeof obj == "number" || typeof obj == "string" || typeof value == "boolean")
    return obj;
  else if (obj instanceof Array) {
    prop = [];
    for(var i=0; i < obj.length; i++)
      prop[i] = utils.serialize(obj[i]);
  }
  else if (typeof obj == "object") {
    prop = {};
    //if derived from Belle's Object, use its serialize function
    if (typeof obj.serialize == "function"){
      prop = obj.serialize();
    }
    else {
      for(key in obj) {
	if (obj.hasOwnProperty(key)) {
	  prop[key] = utils.serialize(obj[key]);
	}
      }
    }
  }
  
  return prop;
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
        belle.log("'" + type + "' is not a valid object type.");
        return null;
    }

    var obj = new _Object(data);
    if (belle.display.DOM)
      obj.initElement();
    return obj;
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
    if (game.data) {
      display.loading(); 
      initializeData(game.data)
    }
    else
      alert("No game data found!");
}

window.onload = function () {
    if (! belle) {
        alert("Couldn't start the engine. There was a problem reading one of the files.");
        return;
    }
  
    //load();
}

function loadScenes(scenes)
{
    var scene;
    var _scenes = [];
    
    for(var i=0; i < scenes.length; i++) {
        scene = scenes[i];
        
        if (! belle[scene.type]) {
            belle.log("Invalid Scene type: " + scene.type);
            continue;
        }
        
        var _Scene = belle[scene.type];
        var sceneObject = new _Scene(scene);        
        _scenes.push(sceneObject);
    }
    
    return _scenes;
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
        belle.display.DOM = true;
    
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
    if ("scenes" in data) {
        game.scenes = loadScenes(data["scenes"]);
    }
    
    if ("pauseScreen" in data && "scenes" in data["pauseScreen"]) {
        game.pauseScreen.scenes = loadScenes(data.pauseScreen["scenes"]);
    }

    isGameDataReady();

}

function isGameDataReady() {
    
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
        setTimeout(isGameDataReady, 100);
    else {
        display.init();
	if (game.getScenes()) 
	  game.nextScene();
        setTimeout(gameLoop, 1);
    }
}

function importgameData(path) {
    var xobj = new XMLHttpRequest();
    if (xobj.overrideMimeType)
        xobj.overrideMimeType("application/json");
    
    xobj.onreadystatechange = function() {
        if(xobj.readyState == 4){
            var gameData = jQuery.parseJSON(xobj.responseText);
            belle.log("Game data loaded!");
            initializeData(gameData);
        }
    };
    
    xobj.open("GET", path, true);
    xobj.send(null);
}

//game's main loop
function gameLoop ()
{   
    var scene = game.getScene();
    
    if (scene && scene.isActive()) {
      var action = scene.getAction();
      
      if (! action || action.isFinished()) {
	  action = scene.nextAction();
	  if (! action && scene.isFinished())
	    scene = game.nextScene();
	  
	  if (! scene && game.isFinished()) //game is finished
	    return;
      }
    }
    
    display.draw();
    setTimeout(gameLoop, timeout);
}

function pause()
{
    if (! game.hasPauseScreen())
      return;
  
    if (game.paused) {
        display.hidePauseScreen();
	game.resume();
    }
    else {
        display.showPauseScreen();
	game.pause();
    }
}

function getGame()
{
    if (game.paused)
        return game.pauseScreen;
    return game;
}

function getSaveDate()
{
    var date = new Date();
    var months = [ "January", "February", "March", "April", "May", "June", 
                    "July", "August", "September", "October", "November", "December" ];
    var hour = date.getHours();
    var min = date.getMinutes();
    if (hour < 10)
        hour = '0' + hour;
    if (min < 10)
        min = '0' + min;
    date = date.getDate() + " " + months[date.getMonth()] + " " + date.getYear() + " " + hour + ":" + min;
    return date;
}

function saveGame(id)
{
    if (! game)
      return;

    if (! id)
      id = 0;
    
    var scene = game.getCurrentScene();
    var name = scene.name;
    var title = game.title;
    var gameData = $.jStorage.get(title, {});
    var savedGames = gameData["savedGames"] || [];
    var i = 0;

    //if string passed, search for an empty slot for the savegame
    if (typeof id == "string") {
        for(i=0; i < savedGames.length; i++) {
            if (savedGames[i] === null) {
                id = i;
                break;
            }
        }
        name = id;
        id = i;
    }
   
    var entry = game.serialize();
    entry.date = getSaveDate();
    entry.name = name;
       
    if (id >= savedGames.length)
        for(var i=savedGames.length; i <= id; i++)
            savedGames.push(null);
    
    savedGames[id] = entry;
    gameData["savedGames"] = savedGames;
    
    $.jStorage.set(title, gameData, {TTL: 0});
    
    return {name: entry.name, date: entry.date};
}

function loadGame(id)
{
    var title = game.title;
    var gameData = $.jStorage.get(title, {});
    var savedGames = gameData["savedGames"] || [];
    var entry = null;

    if (! savedGames) {
        alert("You don't have any saved games.");
        return;
    }
    
    if (typeof id == "number") {
        if (id >= 0 && id < savedGames.length)
            entry = savedGames[i];
    }
    else if (typeof id == "string") {
        for(var i=0; i < savedGames.length; i++) {
            var _entry = savedGames[i];
            if (_entry && _entry.name == id) {
               entry = _entry;
               break;
            }
        }
    }
   
    if (entry) {
        game.load(entry);
    }
    else
        alert('Game "'+ id +'" could not be loaded');      
} 

//Expose public properties
belle.setGameFile = setGameFile;
belle.setGameDirectory = setGameDirectory;
belle.load = load;
belle.addObject = addObject;
belle.createObject = createObject;
belle.getObject = getObject;
belle.getResource = getResource;
belle.game = game;
belle.getObjectPrototype = getObjectPrototype;
belle.getActionPrototype = getActionPrototype;
belle.pause = pause;
belle.saveGame = saveGame;
belle.loadGame = loadGame;

}(belle));

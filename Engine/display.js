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
belle.display = {};

(function(display) {

//private variables
var _drawFPS = false;
var drawTimeout = 50;
var fps = 0;
var elapsed = 0;
var before = 0;
var drawing = false;
var forceRedraw = false;
var scaleWidthFactor = 1;
var scaleHeightFactor = 1;
var showFps = false;
var width = 0;
var height = 0;
var loader = null;
var progress = null;
var container = null;

//public variables
display.view = "portrait";
display.usingDOM = false;
display.context = null;
display.bgContext = null;
display.canvas = null;
display.bgCanvas = null;

var requestAnimationFrame = (function(){
  return  window.requestAnimationFrame       || 
          window.webkitRequestAnimationFrame || 
          window.mozRequestAnimationFrame    || 
          window.oRequestAnimationFrame      || 
          window.msRequestAnimationFrame     || 
          function( callback ){
            window.setTimeout(callback, drawTimeout);
          };
})();

var scaleFont = function(font, scale)
{
    var size = 0;
    if (font && font.contains("px") && font.split("px").length >= 1) {
        size = font.split("px")[0];
        size = parseFloat(size);
    }
    
    if (size) {
        size *= scale;
        if (font.split("px").length >= 2)
            belle.game.font = size + "px" + font.split("px").slice(1);
    }
}

var scaleAll = function(scaleWidth, scaleHeight, reset)
{
    var game = belle.game;
    var width = game.width;
    var height = game.height;
    var container = document.getElementById('belle');
    if (! display.usingDOM) {
        display.canvas = display.canvas ? display.canvas : document.getElementById('canvas');
        display.bgCanvas = display.bgCanvas ? display.bgCanvas : document.getElementById('backgroundCanvas');      
        display.bgContext = display.bgContext ? display.bgContext : display.bgCanvas.getContext('2d');
        display.context = display.context ? display.context : display.canvas.getContext('2d');
    }

    scaleWidthFactor = scaleWidth;
    scaleHeightFactor = scaleHeight;

    //scale font before all objects in case some of the objects use text
    scaleFont(game.font, scaleWidth);
    
    //scale all objects if necessary
    var scene = null;
    for(var i=0; i < game.scenes.length; i++) {
      scene = game.scenes[i];
      scene.scale(scaleWidth, scaleHeight);
      for (var j=0; j < scene.objects.length; j++) {
          object = scene.objects[j];
          object.scale(scaleWidth, scaleHeight);
      }
      
      //special case for ShowMenu action, which has an object that isn't added to the scene
      for (var j=0; j < scene.actions.length; j++) {
          action = scene.actions[j];
          action.scale(scaleWidth, scaleHeight);
      }
    }

    width *= scaleWidth;
    height *= scaleHeight;
    game.width = width;
    game.height = height;
    if (! display.usingDOM) {
        display.bgCanvas.width = width; 
        display.bgCanvas.height = height;
        display.canvas.width = width;
        display.canvas.height = height;
    }
    container.style.width = width + "px";
    container.style.height = height + "px";
}


var init = function()
{       
    if (! isCanvasSupported())
      display.usingDOM = true;

    var game = belle.game;
    var width = game.width;
    var height = game.height;
    var paddingTop = 0;
    var paddingLeft = 0;
    var container = document.getElementById('belle');
    if (! display.usingDOM) {
        display.canvas = display.canvas ? display.canvas : document.getElementById('canvas');
        display.bgCanvas = display.bgCanvas ? display.bgCanvas : document.getElementById('backgroundCanvas');      
        display.bgContext = display.bgContext ? display.bgContext : display.bgCanvas.getContext('2d');
        display.context = display.context ? display.context : display.canvas.getContext('2d');
    }

    var _windowWidth = $(container).parent().width();
    _windowWidth = _windowWidth ? _windowWidth : game.width;
    var _windowHeight = $(container).parent().height();
    _windowHeight = _windowHeight ? _windowHeight : game.height;
    var scaleWidth = scaleWidthFactor;
    var scaleHeight = scaleHeightFactor;
    
    //reset any previous scaling
    if (scaleWidthFactor != 1 && scaleHeightFactor != 1) {
      scaleWidth = 10 / (scaleWidth * 10);
      scaleHeight = 10 / (scaleHeight * 10);
      scaleAll(scaleWidth, scaleHeight);
      scaleWidthFactor = 1;
      scaleHeightFactor = 1;
    }

    //if canvas size is bigger than screen, scale it
    if (game.width > _windowWidth)
        scaleWidth = _windowWidth / game.width;
    
    if (game.height > _windowHeight)
        scaleHeight = _windowHeight / game.height;
    
    if (scaleHeight < scaleWidth)
        scaleWidth = scaleHeight;
    else
        scaleHeight = scaleWidth;
       
    //scale all objects if necessary
    scaleAll(scaleWidth, scaleHeight);
    
    //if canvas size is smaller than screen, center it, otherwise no padding is added
    if (game.width < _windowWidth )
        paddingLeft = (_windowWidth - game.width) / 2;
    
    if (game.height < _windowHeight)
        paddingTop = (_windowHeight - game.height) / 2;
    
    container.style.top = paddingTop + "px";
    container.style.left = paddingLeft + "px";

    width = game.width;
    height = game.height;
    
    container.style.width = width + "px";
    container.style.height = height + "px";
    
    if (! display.usingDOM) {
        display.canvas.width = width;
        display.canvas.height = height;
        display.bgCanvas.width = width;
        display.bgCanvas.height = height;
        display.context.font = game.font;
    }
      
    _windowWidth += "px";
    _windowHeight += "px";
    
    if (game.currentScene) {
        game.currentScene.redrawBackground = true;
        addObjects(game.currentScene);
    }
    
    if (loader && loader.running)
        stopLoading();
}

var redraw = function()
{
    drawing = true;
    var game = belle.game;
    var objectsToDraw = game.currentScene.objects;
    var rect = null;
    var obj = null;
    var i, j;
    var length = 0;

    if (game.currentScene.redrawBackground)
        game.currentScene.paint(display.bgContext);

    for(i=objectsToDraw.length-1; i !== -1; --i) {
        if (! objectsToDraw[i].redraw)
            continue;
            
        for(j=objectsToDraw.length-1; j !== -1; --j) {
            if (objectsToDraw[j].redraw || i == j)
                continue;
  
            if (objectsToDraw[i].overlaps(objectsToDraw[j]))
                objectsToDraw[j].redraw = true;
            /*rect = objectsToDraw[i].overlapedRect(objectsToDraw[j]);
            if (rect)
                objectsToDraw[j].partsToRedraw.push(rect);*/
        }
    }
    
    length = objectsToDraw.length;
    for(j=0; j !== length; j++)
        if (objectsToDraw[j].redraw)
          objectsToDraw[j].clear(display.context);
 
    for(j=0; j !== length; j++) {
        
        obj = objectsToDraw[j];
        
        if (! obj.redraw)
            continue;
        
        obj.paint(display.context);
    }
    
    drawing = false;
}

var _draw = function()
{   
    drawing = true;
    redraw();
    
    if (_drawFPS) 
        drawFPS();
    
    drawing = false;
}

var draw = function()
{
    requestAnimationFrame(_draw);
}

var needsRedraw = function() 
{
    var game = belle.game;
    if (forceRedraw) {
        forceRedraw = false;
        return true;
    }

    if (game.currentScene && game.currentScene.redrawBackground) {
        return true;
    }
    
    var objects = game.currentScene.objects;
    for(var i=0; i !== objects.length; i++) {
        if (objects[i].needsRedraw()) {
            return true;
        }
    }
    
    if (showFps)
      return true;

    return false;
}

var drawFPS = function()
{
    if (! _drawFPS)
        return;
    
    if (elapsed >= 1000) {
        if (document.getElementById("fps")) {
            if (document.getElementById("fps").style.display != "block")
                document.getElementById("fps").style.display = "block";
            document.getElementById("fps").innerHTML = "FPS: " + fps;
        }
        fps = 0;
        elapsed = 0;
        before = new Date().getTime();
    }
    else {  
        if (! before)
            before = new Date().getTime();
        elapsed = new Date().getTime() - before;
        fps++;
    }
}

var removeObjects = function(scene)
{
    if (! display.usingDOM)
        return;
    
    var container = document.getElementById("belle"); 
    var objects = scene.objects;
    if (scene.element)
        container.removeChild(scene.element);
    
    for(var j=0; j < objects.length; j++){
        container.removeChild(objects[j].element);
    }
}

var addObjects = function(scene)
{
    
    if (! display.usingDOM)
        return;
    var container = document.getElementById("belle");
    var objects = scene.objects;
    
    addObject(scene);
    for(var j=0; j < objects.length; j++){
        addObject(objects[j], container);
    }
}

var addObject = function(object, container)
{
    if (! display.usingDOM)
        return;
    
    if (! container)
        container = document.getElementById("belle");
    if (object.visible)
        object.element.style.display = "block";
    container.appendChild(object.element);
}

var isCanvasSupported = function() {
  var elem = document.createElement("canvas");
  return !!(elem.getContext && elem.getContext('2d'));
}

var windowWidth = function() 
{
    return window.innerWidth || document.documentElement.clientWidth || document.documentElement.offsetWidth;
}

var windowHeight = function ()
{
    return window.innerHeight || document.documentElement.clientHeight || document.documentElement.offsetHeight;
}

var clear = function()
{
  if (display.context)
    display.context.clearRect(0, 0, display.canvas.width, display.canvas.height);
}

var updateLoading = function()
{
    if (! loader)
        return;
  
    var cwidth = container.width();
    var cheight = container.height();
    var width = loader.width();
    
    if (cwidth != loader.containerWidth || cheight != loader.containerHeight) {
        var height = loader.height();
        loader.css("left", parseInt((cwidth - width) / 2));
        loader.css("top",parseInt((cheight - height) / 2)); 
    }
    
    var left = parseInt(progress.css("left"));
    var staticBarWidth = loader.staticBarWidth;
  
    progress.width(staticBarWidth);
    if (left >= width)
        left = -staticBarWidth;

    left += 20;
    progress.css("left", left);
  
  if (loader.running)
    setTimeout(updateLoading, 50);
}

var loading = function()
{
    loader = loader || $("#belle #loader");
    progress = progress || $("#belle #loader #progress");
    
    if (! loader.length) {
        loader = $('<div id="loader"></div>');
        loader.css("text-align", "center");
        loader.css("border", "1px solid");
        progress = $('<div id="progress"></div>');
        loader.append(progress);
        $("#belle").append(loader);
    }
    
    container = container || $("#belle");
    var cwidth = container.width();
    var cheight = container.height();
    var barWidth = parseInt(cwidth * 0.4);
    var barHeight = parseInt(cheight * 0.05);
    
    loader.css("left", parseInt((cwidth - barWidth) / 2));
    loader.css("top",parseInt((cheight - barHeight) / 2)); 
    loader.width("40%");
    loader.height("5%");
    loader.staticBarWidth = barWidth / 3;
    progress.width(loader.staticBarWidth);
    loader.running = true;
    loader.containerWidth = cwidth;
    loader.containerHeight = cheight;    
    loader.css("display", "block");
   
    updateLoading();
}

var stopLoading = function()
{
    if (loader.running) {
        loader.running = false;
        $("#loader").css("display", "none");
    }
}

//Expose public functions
display.scaleFont = scaleFont;
display.scaleAll = scaleAll;
display.draw = draw;
display.clear = clear;
display.isCanvasSupported = isCanvasSupported;
display.addObject = addObject;
display.addObjects = addObjects;
display.needsRedraw = needsRedraw;
display.drawFPS = drawFPS;
display.stopLoading = stopLoading;
display.loading = loading;
display.init = init;
display.removeObjects = removeObjects;
display.addObjects = addObjects;
display.display = display;
display.windowWidth = windowWidth;
display.windowHeight = windowHeight;

}(belle.display));

log("Display module loaded!");

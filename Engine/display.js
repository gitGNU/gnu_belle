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
display.DOM = false;
display.context = null;
display.bgContext = null;
display.canvas = null;
display.bgCanvas = null;
display.scaledWidthFactor = 1;
display.scaledHeightFactor = 1;

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
        $('#belle').css("font-size", parseInt(size)+"px");
    }
}

var scaleScene = function(widthFactor, heightFactor)
{
    
    var game = belle.game;
    
    //scale all objects if necessary
    var scene = null;
    for(var i=0; i < game.scenes.length; i++) {
      scene = game.scenes[i];
      scene.scale(widthFactor, heightFactor);
      for (var j=0; j < scene.objects.length; j++) {
          object = scene.objects[j];
          object.scale(widthFactor, heightFactor);
      }
    }
}

function resize()
{
    var game = belle.game;
    var width = game.width;
    var height = game.height;
    var $belle = $('#belle');
    var top = 0;
    var left = 0;
    
    var screenWidth = $belle.parent().width();
    screenWidth = screenWidth ? screenWidth : game.width;
    var screenHeight = $belle.parent().height();
    screenHeight = screenHeight ? screenHeight : game.height;
    var widthFactor = 1;
    var heightFactor = 1;
    
    //if game screen size is bigger than screen, scale it
    if (game.width > screenWidth)
        widthFactor = screenWidth / game.width;
    
    if (game.height > screenHeight)
        heightFactor = screenHeight / game.height;

    if (heightFactor < widthFactor)
        widthFactor = heightFactor;
    else
        heightFactor = widthFactor;
    
    display.scaledWidthFactor = widthFactor;
    display.scaledHeightFactor = heightFactor;

    width *= widthFactor;
    height *= heightFactor;

    //if game sreen size is smaller than screen, center it, otherwise no padding is added
    if (width < screenWidth )
      left = (screenWidth - width) / 2;
    
    if (height < screenHeight)
      top = (screenHeight - height) / 2;
       
    $belle.css("top", top + "px");
    $belle.css("left", left + "px");
    $belle.width(width);
    $belle.height(height);
    
    scaleFont(game.font, widthFactor);
    scaleScene(widthFactor, heightFactor);
}


function initCanvas(width, height)
{
  if (! display.DOM) {
    $('#belle canvas').each(function(index, canvas){
        canvas.width = width;
        canvas.height = height;
    });
  }
}

var init = function()
{       
    if (! isCanvasSupported())
      display.DOM = true;
    
    display.canvas = $("#belle #game").find("#canvas")[0];
    display.bgCanvas = $("#belle #game").find("#backgroundCanvas")[0];
    display.bgContext = display.bgCanvas.getContext('2d');
    display.context = display.canvas.getContext('2d');
    
    var game = belle.game; 
    initCanvas(game.width, game.height);
    resize(); 
    
    if ($progress && $progress.attr("running"))
      stopLoading();
        
    if (game.currentScene) {
        game.currentScene.redrawBackground = true;
        addObjects(game.currentScene);
    }
}

var hidePauseScreen = function()
{
    var container = "#belle #game";
    if (! display.DOM) {
        canvas = $container.find("#canvas")[0];
        bgCanvas = $container.find("#backgroundCanvas")[0];
        display.bgContext = bgCanvas.getContext('2d');
        display.context = canvas.getContext('2d');
    }
    
    $("#belle #pauseScreen").css("display", "none");
}

var showPauseScreen = function()
{
    var container = "#belle #pauseScreen";
    if (! display.DOM) {
        canvas = $container.find("#canvas")[0];
        bgCanvas = $container.find("#backgroundCanvas")[0];
        display.bgContext = bgCanvas.getContext('2d');
        display.context = canvas.getContext('2d');
    }
    
    $("#belle #pauseScreen").css("display", "block");
}

var redraw = function()
{
    drawing = true;
    var game = belle.game;
    if (belle.game.paused)
        game = belle.game.pauseScreen;
    var objectsToDraw = game.currentScene.objects;
    var rect = null;
    var obj = null;
    var i, j;
    var length = 0;

    display.context.font = game.font;
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
    if (game.paused)
        game = game.pauseScreen;
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
    if (! display.DOM)
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
    if (! display.DOM)
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
    object.scale(display.scaledWidthFactor, display.scaledHeightFactor);
    
    if (display.DOM) {
        if (! container)
            container = document.getElementById("belle");
        if (object.visible)
            object.element.style.display = "block";
        container.appendChild(object.element);
    }
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

var updateLoading = function($progress)
{
    if (! $progress)
        return;
  
    var pwidth = $progress.parent().width();
    var width = $progress.width();
    
    $progress.animate( {
        "left" : pwidth,
        "easing" : "linear"
      },
      2000,
      function(){
        if ($progress.attr("running")) {
          $progress.css("left", -width);
          setTimeout(function(){updateLoading($progress); }, 10);
        }
        else
          $progress.parent().css("display", "none");
      }
    );
}

var loading = function()
{
    $loader = $("#belle #loader");
    $progress = $("#belle #loader #progress");
    $container = $("#belle");
    $loader.css("top",parseInt($container.height()/2 - $loader.height()/2));  
    $loader.css("display", "block");
    $progress.attr("running", true);
    updateLoading($progress);
}

var stopLoading = function()
{
    if ($progress.attr("running")) {
        $progress.attr("running", false);
        $loader.css("display", "none");
    }
}

//Expose public functions
display.scaleFont = scaleFont;
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
display.windowWidth = windowWidth;
display.windowHeight = windowHeight;
display.showPauseScreen = showPauseScreen;
display.hidePauseScreen = hidePauseScreen;
display.resize = resize;

}(belle.display));

log("Display module loaded!");

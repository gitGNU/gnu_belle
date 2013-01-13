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

var view = 'portrait';
var _drawFPS = false;
var fps = 0;
var elapsed = 0;
var before = 0;

var requestAnimationFrame = (function(){
  return  window.requestAnimationFrame       || 
          window.webkitRequestAnimationFrame || 
          window.mozRequestAnimationFrame    || 
          window.oRequestAnimationFrame      || 
          window.msRequestAnimationFrame     || 
          function( callback ){
            window.setTimeout(callback, Novel.drawTimeout);
          };
})();

function scaleAll(scaleWidth, scaleHeight, reset)
{
    var width = Novel.width;
    var height = Novel.height;
    var container = document.getElementById('container');
    if (! Novel.usingDOM) {
        var gameCanvas = document.getElementById('gameCanvas');
        var bgCanvas = document.getElementById('gameBackgroundCanvas');
        var bgContext = bgCanvas.getContext('2d');
    }

    Novel.scaleWidthFactor = scaleWidth;
    Novel.scaleHeightFactor = scaleHeight;

    //scale font before all objects in case some of the objects use text
    scaleFont(Novel.font, scaleWidth);
    
    //scale all objects if necessary
    var scene = null;
    for(var i=0; i < Novel.scenes.length; i++) {
      scene = Novel.scenes[i];
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
    Novel.width = width;
    Novel.height = height;
    if (! Novel.usingDOM) {
        bgCanvas.width = width; 
        bgCanvas.height = height;
        gameCanvas.width = width;
        gameCanvas.height = height;
    }
    container.style.width = width + "px";
    container.style.height = height + "px";
}

function scaleFont(font, scale)
{
    var size = 0;
    if (font && font.contains("px") && font.split("px").length >= 1) {
        size = font.split("px")[0];
        size = parseFloat(size);
    }
    
    if (size) {
        size *= scale;
        if (font.split("px").length >= 2)
            Novel.font = size + "px" + font.split("px").slice(1);
    }
}

function initDisplay()
{
    var width = Novel.width;
    var height = Novel.height;
    var paddingTop = 0;
    var paddingLeft = 0;
    var container = document.getElementById('container');
    if (! Novel.usingDOM) {
        var gameCanvas = document.getElementById('gameCanvas');
        var myContext = gameCanvas.getContext('2d');
        var bgCanvas = document.getElementById('gameBackgroundCanvas');
        var bgContext = bgCanvas.getContext('2d');
    }
    var _windowWidth = windowWidth();
    var _windowHeight = windowHeight();
    var scaleWidth = Novel.scaleWidthFactor;
    var scaleHeight = Novel.scaleHeightFactor;
    
    //reset any previous scaling
    if (Novel.scaleWidthFactor != 1 && Novel.scaleHeightFactor != 1) {
      scaleWidth = 10 / (scaleWidth * 10);
      scaleHeight = 10 / (scaleHeight * 10);
      scaleAll(scaleWidth, scaleHeight);
      Novel.scaleWidthFactor = 1;
      Novel.scaleHeightFactor = 1;
    }
        
    //if canvas size is bigger than screen, scale it
    if (Novel.width > _windowWidth)
        scaleWidth = _windowWidth / Novel.width;
    
    if (Novel.height > _windowHeight)
        scaleHeight = _windowHeight / Novel.height;
    
    if (scaleHeight < scaleWidth)
        scaleWidth = scaleHeight;
    else
        scaleHeight = scaleWidth;
       
    //scale all objects if necessary
    scaleAll(scaleWidth, scaleHeight);
    
    //if canvas size is smaller than screen, center it, otherwise no padding is added
    if (Novel.width < _windowWidth )
        paddingLeft = (_windowWidth - Novel.width) / 2;
    
    if (Novel.height < _windowHeight)
        paddingTop = (_windowHeight - Novel.height) / 2;
    
    container.style.top = paddingTop + "px";
    container.style.left = paddingLeft + "px";

    width = Novel.width;
    height = Novel.height;
    
    container.style.width = width + "px";
    container.style.height = height + "px";
    
    if (! Novel.usingDOM) {
        gameCanvas.width = width;
        gameCanvas.height = height;
        bgCanvas.width = width;
        bgCanvas.height = height;
        myContext.font = Novel.font;    
        Novel.context = myContext;
        Novel.bgContext = bgContext;
    }
      
    _windowWidth += "px";
    _windowHeight += "px";
    
    if (Novel.currentScene) {
        Novel.currentScene.redrawBackground = true;
        addObjects(Novel.currentScene);
    }
}

function redraw()
{
    Novel.drawing = true;
    var objectsToDraw = Novel.currentScene.objects;
    var rect = null;
    var obj = null;
    var i, j;
    var context = Novel.context;
    var length = 0;
    
    if (Novel.currentScene.redrawBackground)
        Novel.currentScene.paint(Novel.bgContext);

    for(i=objectsToDraw.length-1; i !== -1; --i) {
        if (! objectsToDraw[i].redraw || ! objectsToDraw[i].visible)
            continue;
            
        for(j=objectsToDraw.length-1; j !== -1; --j) {
            if (objectsToDraw[j].redraw || i == j || ! objectsToDraw[j].visible)
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
          objectsToDraw[j].clear(context);
 
    for(j=0; j !== length; j++) {
        
        obj = objectsToDraw[j];
        
        if (! obj.visible || ! obj.redraw)
            continue;
        
        obj.paint(context);
    }
    
    Novel.drawing = false;
}

function draw()
{   
    Novel.drawing = true;
    redraw();
    
    if (_drawFPS) 
        drawFPS();
    
    Novel.drawing = false;
}

function needsRedraw() 
{
    if (Novel.forceRedraw) {
        Novel.forceRedraw = false;
        return true;
    }

    if (Novel.currentScene && Novel.currentScene.redrawBackground) {
        return true;
    }
    
    var objects = Novel.currentScene.objects;
    for(var i=0; i !== objects.length; i++) {
        if (objects[i].needsRedraw()) {
            return true;
        }
    }
    
    return false;
}

function drawFPS()
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
        //elapsed += Novel.drawTimeout;
        fps++;
    }
}

log("Draw module loaded!");

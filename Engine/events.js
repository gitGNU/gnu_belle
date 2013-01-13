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

var container = document.getElementById('container');
var hoveredObject = null;
var pressedObject = null;

function mapToCanvas(event)
{
    var x;
    var y;
    
    var IE = document.all ? true : false; // check to see if you're using IE

    if (IE) { //do if internet explorer 
        x = event.clientX + document.body.scrollLeft;
        y = event.clientY + document.body.scrollTop;
    }
    else {  //do for all other browsers
        x = (window.Event) ? event.pageX : event.clientX + (document.documentElement.scrollLeft ? document.documentElement.scrollLeft : document.body.scrollLeft);
        y = (window.Event) ? event.pageY : event.clientY + (document.documentElement.scrollTop ? document.documentElement.scrollTop : document.body.scrollTop);
    }

    var marginLeft = parseInt(container.style.left, 10);
    var marginTop = parseInt(container.style.top, 10);
    
    event.canvasX = x - marginLeft;
    event.canvasY = y - marginTop;
}

document.onmousemove = function(event)
{
    var ev = event || window.Event || window.event;
  
    if (! Novel || ! Novel.currentScene)
        return;
   
    mapToCanvas(ev);
    
    var prevHoveredObject = hoveredObject;
    hoveredObject = null;
    var objects = Novel.currentScene.objects;
    for (var i=objects.length-1; i !== -1; --i) {
        if (objects[i].processEvent(ev)) {
            hoveredObject = objects[i];
            break;
        }
    }

    if (prevHoveredObject && prevHoveredObject != hoveredObject) {
        ev.mouseleave = true;
        prevHoveredObject.mouseLeaveEvent(ev);
    }
}

document.onmouseup = function(event)
{
    var ev = event || window.Event || window.event;
    var processed = false;

    if (pressedObject && hoveredObject == pressedObject) {
        mapToCanvas(ev);
        if (hoveredObject.processEvent(ev))
            processed = true;
    }
    
    if (Novel.currentAction && ! processed)
        Novel.currentAction.skip();
}

document.onmousedown = function(event) 
{
    var ev = event || window.Event || window.event;
    
    pressedObject = hoveredObject;
    
    if (hoveredObject) {
        mapToCanvas(event);
        hoveredObject.processEvent(ev)
    }
}

document.onkeyup = function(event) 
{
    var ev = event || window.Event || window.event;
        
    switch(ev.keyCode) {
        case 13: //ENTER
        case 32: //SPACE
            //if (Novel.currentAction instanceof Dialogue || Novel.currentAction instanceof Wait)
            if (Novel.currentAction)
                Novel.currentAction.skip();
    }
}

function resize() 
{
    var _view = 'portrait';  
    if (windowWidth() > windowHeight())
        _view = 'landscape';
        
    if (window.view != _view) {
        window.view = _view;    
        initDisplay();
    }
}

if (window.addEventListener) 
  window.addEventListener('resize', resize, false);
else if (window.attachEvent)
  window.attachEvent('resize', resize);
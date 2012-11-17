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
    var x = event.x || event.pageX;
    var y = event.y || event.pageY;
    var marginLeft = parseInt(container.style.marginLeft, 10);
    var marginTop = parseInt(container.style.marginTop, 10);
    
    ev.canvasX = x - marginLeft;
    ev.canvasY = y - marginTop;
}

document.onmousemove = function(event)
{
    ev = event || window.event;
  
    if (! Novel.currentScene)
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
    ev = event || window.event;
    
    var processed = false;
    if (pressedObject && hoveredObject == pressedObject) {
        mapToCanvas(ev);
        if (hoveredObject.processEvent(event))
            processed = true;
    }
    
    if (Novel.currentAction && ! processed)
        Novel.currentAction.skip();
}

document.onmousedown = function(event) 
{
    pressedObject = hoveredObject;
    
    if (hoveredObject) {
        mapToCanvas(event);
        hoveredObject.processEvent(event)
    }
}

document.onkeyup = function(event) 
{
    switch(event.keyCode) {
        case 13: //ENTER
        case 32: //SPACE
            //if (Novel.currentAction instanceof Dialogue || Novel.currentAction instanceof Wait)
                Novel.currentAction.skip();
            break;
        
    }
}

window.addEventListener('resize', function() {
  var _view = 'portrait';  
  if (window.innerWidth > window.innerHeight)
    _view = 'landscape';
    
  if (window.view != _view) {
    window.view = _view;    
    initDisplay();
  }
}, false);

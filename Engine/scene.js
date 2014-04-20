/* Copyright (C) 2012-2014 Carlos Pais 
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

(function(belle) {

var Color = belle.objects.Color;
  
function Scene(data)
{
    this.objects = [];
    this.actions = [];
    this.action = null;
    this.image = null;
    this.ready = false;
    this.finished = false;
    this.redrawBackground = false;
    this.name = "";
    this.backgroundImage = null;
    this.backgroundColor = null;
    this.backgroundImageLoaded = false;
    var backgroundImage = "";
    var backgroundColor = null;
    this.width = this.scaledWidth = belle.game.width;
    this.height = this.scaledHeight = belle.game.height;
    this.x = this.scaledX = 0;
    this.y = this.scaledY = 0;
    data.width = this.width;
    data.height = this.height;
    this.visible = true;
    this.tries = 0;
    this.eventListeners = {};
    
    this.element = document.createElement("div");
    this.backgroundElement = document.createElement("div");
    this.element.appendChild(this.backgroundElement);
    this.element.id = data["name"];
    $(this.element).addClass("scene");
    
    belle.utils.initElement(this.element, data);
    belle.utils.initElement(this.backgroundElement, data);
    this.backgroundElement.style.display = "block";
    
    if (data) {
        if ("backgroundImage" in data)
            backgroundImage = data["backgroundImage"];
        if ("backgroundColor" in data)
            backgroundColor = new Color(data["backgroundColor"]);
        if ("name" in data)
            this.name = data["name"];
    }
    
    if (backgroundImage)
        this.setBackgroundImage(backgroundImage);
    if (backgroundColor)
        this.setBackgroundColor(backgroundColor);
}

Scene.prototype.addEventListener = function(event, listener)
{
  var events = []; 
  if(this.eventListeners.hasOwnProperty(event))
    events =  this.eventListeners[event];
  events.push(listener);
  this.eventListeners[event] = events;
}

Scene.prototype.display = function() {
  belle.display.displayScene(this);
 
  //when transitions for scenes are added, call the animation here and set scene
  //to active after the animation is finished
  this.setActive(true);
}

Scene.prototype.setActive = function(active) {
  var listeners = [];
  if(active && this.eventListeners.hasOwnProperty("onActivated")) {
    listeners = this.eventListeners["onActivated"];
  }
  else if(! active && this.eventListeners.hasOwnProperty("onDeactivated")) {
    listeners = this.eventListeners["onDeactivated"];
  }

  for(var i=0; i < listeners.length; i++)
    listeners[i].call(this);
    
  this.active = active;
}

Scene.prototype.isActive = function() {
  return this.active;
}

Scene.prototype.isFinished = function(active) {
  return this.finished;
}

Scene.prototype.addObject = function(object) {
    this.objects.push(object);
    if (game.getScene() == this) {
        belle.display.addObject(object);
    }
}

Scene.prototype.goto = function(action) {
    this.setCurrentAction(action);
}

Scene.prototype.indexOf = function(object) {
    if (belle.isInstance(object, belle.objects.Object))
        return this.objects.indexOf(object);
    else if (belle.isInstance(object, belle.actions.Action))
        return this.actions.indexOf(object);
    return -1;
}

Scene.prototype.getActions = function() {
    return this.actions;
}

Scene.prototype.setCurrentAction = function(action) {
  var actions = this.getActions();
  if (typeof action == "number") {
    if (action >= 0 && action < actions.length)
      this.action = actions[action];
  }
  else if (action instanceof String || typeof action === 'string') {
      for (var i=0; i !== actions.length; i++) {
	  if (actions[i].name === action) {
	      this.action = actions[i];
	      break;
	  }
      }
  }
  else if (belle.isInstance(action, belle.actions.Action) && this.indexOf(action) !== -1){
      this.action = action;
  }
}

Scene.prototype.getCurrentAction = function() {
  return this.getAction();
}

Scene.prototype.getAction = function(name) {
    if (name) {
        for (var i=0; i < this.actions.length; i++)
            if (this.actions[i].name == name)
                return this.actions[i];
    }
    else if (name === undefined)
        return this.action;
    
    return null;    
}

Scene.prototype.getObject = function(name) {
    if (name) {
      for (var i=0; i < this.objects.length; i++)
	if (this.objects[i].name == name)
	  return this.objects[i];
    }
    
    return null;    
}

Scene.prototype.nextAction = function() {
    if (this.finished)
      return null;
    
    var index = this.actions.indexOf(this.action) + 1;
    
    if (index >= 0 && index < this.actions.length) { 
        this.action = this.actions[index];
	this.action.execute();
	return this.action;
    }
    else {
      this.finished = true;
    }

    return null;
}

Scene.prototype.setBackgroundImage = function(background)
{
    var that = this;
    var AnimationImage = belle.objects.AnimationImage;
    
    if (background instanceof AnimationImage) {
        if (this.backgroundImage && background == this.backgroundImage)
            return;
        this.backgroundImage = background;
        that.backgroundImageLoaded = true; //assume for now
        if (this.backgroundElement) {
            this.backgroundElement.style.backgroundColor = "";
            if (this.backgroundElement.childNodes.length > 0)
                this.backgroundElement.removeChild(this.backgroundElement.childNodes[0]);
            this.backgroundElement.appendChild(this.backgroundImage.img);
            //this.backgroundElement.style.backgroundImage = "url('" + background.src + "')";
        }
    }
    else if (typeof background == "string" || typeof background == "object") {
        if (this.backgroundImage && typeof background == "string" && background == this.backgroundImage.img.src)
            return;
        this.backgroundImage = new AnimationImage(background, this);
        if (this.backgroundElement) {
            this.backgroundElement.style.backgroundColor = "";
            if (this.backgroundElement.childNodes.length > 0)
                this.backgroundElement.removeChild(this.backgroundElement.childNodes[0]);
            this.backgroundElement.appendChild(this.backgroundImage.img);
            //this.backgroundElement.style.backgroundImage = "url('" + background + "')";
        }
    }
   
    if (this.backgroundImage)
        this.redrawBackground = true;
}

Scene.prototype.setBackgroundColor = function(color)
{
    if (this.backgroundColor != color) {
        
        if (color instanceof Array)
            color = new Color(color);

        this.backgroundColor = color;
        
        if (this.backgroundImage) {
            if (this.backgroundElement.childNodes.length > 0)
                this.backgroundElement.removeChild(this.backgroundElement.childNodes[0]);
            this.backgroundImage.src = "";
            this.backgroundImage = null;
        }
        
        if (this.backgroundColor) {
            this.redrawBackground = true;
            this.backgroundElement.style.backgroundColor = this.backgroundColor.toHex();
        }
    }
}

Scene.prototype.paint = function(context)
{    
    if (this.backgroundImage) {
        this.backgroundImage.paint(context, 0, 0, this.width, this.height);
    }
    else if (this.backgroundColor) {
        context.fillStyle  = this.backgroundColor.toString();
        context.fillRect(this.x, this.y, this.width, this.height);
    }
    
    this.redrawBackground = false;
}

Scene.prototype.isReady = function()
{
    if (this.backgroundImage) {
        return this.backgroundImage.isReady();
    }
    return true;
}

Scene.prototype.frameChanged = function()
{
    this.paint(belle.display.bgContext);
}

Scene.prototype.scale = function(widthFactor, heightFactor)
{
    this.setX(this.x);
    this.setY(this.y);
    this.setWidth(this.width);
    this.setHeight(this.height);      
}

Scene.prototype.setX = function(x)
{
    this.x = this.scaledX = x;
    var scale = belle.display.scaledWidthFactor;
    if (scale != 1)
        this.scaledX *= scale;
    this.element.style.left = this.scaledX + "px";
    this.backgroundElement.style.left = this.scaledX + "px";
}

Scene.prototype.setY = function(y)
{
    this.y = this.scaledY = y;
    var scale = belle.display.scaledHeightFactor;
    if (scale != 1)
        this.scaledY *= scale;
    this.element.style.top = this.scaledY + "px";
    this.backgroundElement.style.top = this.scaledY + "px";
}

Scene.prototype.setWidth = function(width)
{
    this.width = this.scaledWidth = width;
    var scale = belle.display.scaledWidthFactor;
    if (scale != 1)
        this.scaledWidth *= scale;
    this.element.style.width = this.scaledWidth + "px";
    this.backgroundElement.style.width = this.scaledWidth + "px";
}


Scene.prototype.setHeight = function(height)
{
    this.height = this.scaledHeight = height;
    var scale = belle.display.scaledHeightFactor;
    if (scale != 1)
        this.scaledHeight *= scale;
    this.element.style.height = this.scaledHeight + "px";
    this.backgroundElement.style.height = this.scaledHeight + "px";
}

belle.Scene = Scene;

}(belle));
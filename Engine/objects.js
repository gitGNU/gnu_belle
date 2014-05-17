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
belle.objects = {};

(function(objects) {

/*********** AnimationImage **********/

function AnimationImage(imageData, parent)
{
    this.imageLoaded = false;
    this.frames = null;
    this.currentFrame = 0;
    this.frameDelay = 100; // 100ms by default
    this.framesLoaded = 0;
    this.image = null;
    this.interval = null;
    this.parent = parent;
    this.animated = false;
    
    this.load(imageData);
}

AnimationImage.prototype.load = function (imageData) 
{ 
    var that = this;
    if (typeof imageData === "object" && "src" in imageData) {
        //activate DOM mode if game contains animated image
        if ("animated" in imageData && imageData["animated"])
            belle.display.DOM = true;
        imageData = imageData["src"];
    }
    
    if (typeof imageData === "string") {
        this.img = new window.Image();
        this.img.onload = function() { 
            that.imageLoaded = true;
        };

        this.img.src = belle.game.directory + imageData;
        this.img.style.width = "100%";
        this.img.style.height = "100%";
        this.img.style.display = "block";
    }
}

AnimationImage.prototype.isReady = function () { 
    if (this.frames) {
        if (this.frames.length == this.framesLoaded)
            return true;
        else
            return false;
    }
    
    if (this.img)
        return this.img.complete && this.imageLoaded;
    
    return true;
}

AnimationImage.prototype.paint = function(context, x, y, width, height) {
   
    if (! context)
        return;
    
    if (this.img.complete) {
        context.drawImage(this.img, x, y, width, height);
    }
    else if (this.frames) {
        context.drawImage(this.frames[this.currentFrame], x, y, width, height);
        this.currentFrame++;
        if (this.currentFrame >= this.frames.length)
            this.currentFrame = 0;
    }
}

AnimationImage.prototype.serialize = function() 
{
    var data = {};
    data = $(this.img).attr("src");
    return data;
}

/**** COLOR ****/
var Color = function(components)
{
    var error = false;
    if (components.length < 4) {
        error = true;
        belle.log("Color(components) : Missing one or more color component(s)");
    }
    
    if (error) {
        this.red = 0;
        this.green = 0;
        this.blue = 0;
        this.alpha = 0;
    }
    else {
        this.red = components[0];
        this.green = components[1];
        this.blue = components[2];
        this.alpha = components[3];
    }
}

Color.prototype.toString = function()
{
    return 'rgba(' + this.red + ',' + this.green + ',' + this.blue + ',' + this.alphaF() + ')';
}

Color.prototype.componentToHex = function (c) 
{
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
}

Color.prototype.toHex = function()
{
    return "#" + this.componentToHex(this.red) + this.componentToHex(this.green) + this.componentToHex(this.blue);
}

Color.prototype.alphaF = function()
{
    return this.alpha / 255;
}

Color.prototype.serialize = function()
{
    return [this.red, this.green, this.blue, this.alpha];
}

/*********** POINT **********/
var Point = function (x, y)
{
    this.x = x;
    this.y = y;
    
    if (this.x === null || this.x === undefined)
        this.x = 0;
    if (this.y === null || this.y === undefined)
        this.y = 0;
}

Point.prototype.distance = function(point) 
{
    return Math.sqrt(Math.pow(point.x-this.x, 2) + Math.pow(point.y-this.y, 2));
}
    
/*********** BASE OBJECT ***********/

function Object(info)
{
    if ("resource" in info && belle.getResource(info["resource"])) {
        var resourceData = belle.getResource(info["resource"]).data;
        belle.utils.extendJsonObject(info, resourceData);
    }
    
    //check for percent width or height and for percent widths or heights
    if ("__parent" in info) 
        var parent = info["__parent"];
    
    if (typeof info.width == "string" && belle.utils.isPercentSize(info.width) && parent && parseInt(parent.width) != NaN)
        info.width =  parseInt(info.width) * parent.width / 100;

    if (typeof info.height == "string" && belle.utils.isPercentSize(info.height) && parent && parseInt(parent.height) != NaN)
        info.height =  parseInt(info.height) * parent.height / 100;
    
    this.element = document.createElement("div");
    this.backgroundElement = document.createElement("div");
    this.element.appendChild(this.backgroundElement);
   
    this.roundedRect = false;
    this.mousePressActions = [];
    this.mouseReleaseActions = [];
    this.mouseMoveActions = [];
    this.mouseLeaveActions = [];
    this.name = "";
    this.loaded = true;
    this.data = info;
    this.parent = parent ? parent : null;
    this.type = info["type"];
    this.resource = info["resource"] || "";
    this.eventListeners = {
        "mousepress" : [],
        "mouserelease" : [],
        "mousemove" : []
    };
    
    var actions;
    var action;
    var actionObject;
    
    if ("onMousePress" in info) {
        this.mousePressActions = this.initActions(info["onMousePress"]);
    }
    
    if ("onMouseRelease" in info) {
        this.mouseReleaseActions = this.initActions(info["onMouseRelease"]);
    }
    
    if ("onMouseMove" in info) {
        this.mouseMoveActions = this.initActions(info["onMouseMove"]);
    }
    
    if ("onMouseLeave" in info) {
        this.mouseLeaveActions = this.initActions(info["onMouseLeave"]);
    }
    
    Object.prototype.load.call(this, info);
}

Object.prototype.load = function(data)
{
    this.setX(data.x);
    this.setY(data.y);
    this.setWidth(data.width);
    this.setHeight(data.height);
    this.backgroundColor = new Color([255, 255, 255, 255]);
    this.backgroundImage = null;
    this.cornerRadius = 0;
    this.opacity = 255;
    this.borderWidth = 0;
    this.borderColor = null;
    this.visible = false;
    this.paintX = false;
    this.paintY = false; 
    this.redraw = false;
    this.redrawing = false;
    
    if ("name" in data)
        this.name = data["name"];
    
    if ("backgroundColor" in data) {
        this.setBackgroundColor(new Color(data["backgroundColor"]));
    }

    if ("cornerRadius" in data && data["cornerRadius"] > 0) {
      this.setCornerRadius(data["cornerRadius"]);
    }
    
    if( "backgroundImage" in data) {
        this.backgroundImage = new AnimationImage(data["backgroundImage"], this);
        if (this.backgroundElement)
            this.backgroundElement.appendChild(this.backgroundImage.img);
    }
    
    if ("borderWidth" in data) {
      this.borderWidth = data["borderWidth"];
    }
    
    if ("borderColor" in data) {
      this.borderColor = new Color(data["borderColor"]);
    }
    
    if ("opacity" in data) {
	this.setOpacity(data["opacity"]);
    }
    
    if ("visible" in data) {
        this.visible = data["visible"];
    }
}

Object.prototype.frameChanged = function()
{
    this.redraw = true;
}

Object.prototype.setCornerRadius = function(radius) {
    this.cornerRadius = radius;
    belle.utils.setBorderRadius(this.element, radius);
    belle.utils.setBorderRadius(this.backgroundElement, radius);
    if (this.cornerRadius > 0)
      this.roundedRect = true;
}

Object.prototype.globalX = function()
{
    if (this.parent)
        return this.x + this.parent.x;
    return this.x;
}

Object.prototype.globalY = function()
{
    if (this.parent)
        return this.y + this.parent.y;
    return this.y
}

Object.prototype.overlaps = function(object)
{
    var width = this.width;
    var height = this.height;
    var x = this.x;
    var y = this.y;
    var otherWidth = object.width;
    var otherHeight = object.height;
    var otherX = object.x;
    var otherY = object.y;
    var style = this.element.style;
    var paintX = this.paintX;
    var paintY = this.paintY;
    
    if ( otherX > x + width || otherY > y + height || x > otherX + otherWidth || y > otherY + otherHeight ) 
        return false;
    
    //if ( otherX > paintX + width || otherY > paintY + height || paintX > otherX + otherWidth || paintY > otherY + otherHeight ) 
    //    return false;
        
    return true;
}

Object.prototype.overlapedRect = function(object) {
    var x=0, y=0, width=0, height=0;
   
    return null;
}

Object.prototype.setX = function(x)
{
    
    this.x = this.scaledX = x;
    var scale = belle.display.scaledWidthFactor;
    if (scale != 1)
        this.scaledX *= scale;
    
    if (this.element)
        this.element.style.left = this.scaledX + "px";
}

Object.prototype.setY = function(y)
{
    this.y = this.scaledY = y;
    var scale = belle.display.scaledHeightFactor;
    if (scale != 1)
        this.scaledY *= scale;
    if (this.element)
        this.element.style.top = this.scaledY + "px";
}

Object.prototype.contains = function(px, py)
{
    if (! this.visible)
      return false;
  
    var x = this.scaledX, y = this.scaledY;
    
    if (this.parent) {
        x += this.parent.scaledX;
        y += this.parent.scaledY;        
    }
    
    if ( px >= x && px <=  x+this.scaledWidth && py >= y && py <= y+this.scaledHeight)
        return true;
    return false;
}

Object.prototype.getBackgroundOpacity = function()
{
  return this.backgroundColor.alpha;
}

Object.prototype.getBackgroundOpacityF = function()
{
  return this.backgroundColor.alphaF();
}

Object.prototype.setBackgroundOpacity = function(alpha)
{
    this.backgroundColor.alpha = alpha; 
    if (this.backgroundElement) {
      $(this.backgroundElement).fadeTo(1, this.getBackgroundOpacityF() * this.getOpacityF());
    }
}

Object.prototype.setBackgroundColor = function(color)
{
    this.backgroundColor = color;
    if (this.backgroundElement)
        this.backgroundElement.style.backgroundColor = color.toHex();
    this.setBackgroundOpacity(color.alpha);
}

Object.prototype.getOpacity = function (alpha)
{
  return this.opacity;
}

Object.prototype.getOpacityF = function (alpha)
{
  return this.opacity / 255;
}

Object.prototype.setOpacity = function (alpha)
{
    if (alpha < 0)
        alpha = 0;
    else if (alpha > 255)
        alpha = 255;
    
    if (this.opacity != alpha) {
      this.opacity = alpha;
      this.redraw = true;
    }
    
    //always update DOM element, since this function is called on constructor
    //and then on initElement with all children added
    if (this.element)
	$(this.element).children().fadeTo(1, this.getOpacityF());   
}

Object.prototype.paint = function(context)
{
    if (! this.redraw ||this.redrawing)
        return false;
    
    this.redrawing = true;
        
    if (! this.visible) {
        this.redrawing = false;
        this.redraw = false;
        return false;
    }
    
    var x = this.globalX();
    var y = this.globalY();
    
    context.globalAlpha = this.getOpacityF();
    
    if (this.backgroundImage) {
        this.backgroundImage.paint(context, x, y, this.width, this.height);
    }
    else if (this.roundedRect) {
        
        var width = this.width;
        var height = this.height;
        var xradius = this.cornerRadius;
        var yradius = this.cornerRadius;
    
        context.fillStyle  = this.backgroundColor.toString();
        context.strokeStyle = this.borderColor.toString();
        context.beginPath();
       
        context.moveTo(x+xradius, y);
        context.lineTo(x+width-xradius, y); //to work with IE
         //draw top and top right corner
        context.arcTo(x + width, y, x + width, y + yradius, yradius);
        
        //to work with IE
        context.lineTo(x+width, y+height-yradius);
        //draw right and bottom right corner
        context.arcTo(x + width, y+height, x+width-xradius, y + height, xradius);

        //to work with IE
        context.lineTo(x+xradius, y+height);
        //draw bottom and bottom left corner
        context.arcTo(x, y+height, x, y+height-yradius, yradius);

        //to work with IE
        context.lineTo(x, y+yradius);
        //draw left and top left corner
        context.arcTo(x, y, x+xradius, y, xradius);
        
        context.closePath();
        
        if (this.borderWidth > 0 ) {
            context.lineWidth = this.borderWidth;
            context.stroke();
        }
        context.fill();
        
    }
    else {
        context.fillStyle  = this.backgroundColor.toString();
        context.fillRect(x, y, this.width, this.height);
    }
        
    this.paintX = x;
    this.paintY = y;
    
    this.redrawing = false;
    this.redraw = false;
    return true;
}

Object.prototype.isReady = function()
{
    if (this.backgroundImage)
        return this.backgroundImage.isReady();

    return true;
}

Object.prototype.mouseLeaveEvent = function(event)
 {
    var actions = this.mouseMoveActions;
    for(var i =0; i !== actions.length; i++) 
        actions[i].reset();
 }

Object.prototype.addEventListener = function(event, listener)
{
    if (event == "mousepress")
        this.eventListeners["mousepress"].push(listener);
    else if (event == "mouserelease")
        this.eventListeners["mouserelease"].push(listener);
    else if (event == "mousemove")
        this.eventListeners["mousemove"].push(listener);        
}
 
Object.prototype.processEvent = function(event)
{
    var x = event.canvasX;
    var y = event.canvasY;

    if (! this.contains(x, y))
        return false;
    
    var actions = [];
    var listeners = [];

    if (event.type == "mousemove") {
        actions = this.mouseMoveActions;
        listeners = this.eventListeners["mousemove"];
    }
    else if (event.type == "mouseup") {
        actions = this.mouseReleaseActions;
        listeners = this.eventListeners["mouserelease"];
    }
    else if (event.type == "mousedown") {
        actions = this.mousePressActions;
        listeners = this.eventListeners["mousepress"];
    }
    
    for(var i =0; i !== actions.length; i++) 
        actions[i].execute();
    for (var i=0; i !== listeners.length; i++)
        listeners[i].call(this);

    if (actions.length || listeners.length)
      return true;
    return false;
}

Object.prototype.moveTo = function(x, y) 
{
    this.setX(x);
    this.setY(y);
}

Object.prototype.needsRedraw = function()
{
    return this.redraw;
}

Object.prototype.clear = function (context)
{
    if (this.paintX !== false && this.paintY !== false) {
        //context.clearRect(this.globalX()-this.borderWidth, this.globalY()-this.borderWidth, this.width+this.borderWidth*2, this.height+this.borderWidth*2);
        context.clearRect(this.paintX-this.borderWidth, this.paintY-this.borderWidth, this.width+this.borderWidth*2, this.height+this.borderWidth*2);
        this.paintX = false;
        this.paintY = false;
    }
}

Object.prototype.contentWidth = function()
{
}

Object.prototype.contentHeight = function()
{
}

Object.prototype.fullWidth = function()
{
  return this.width + this.borderWidth + this.paddingLeft + this.paddingRight;
}

Object.prototype.fullHeight = function()
{
  return this.width + this.borderWidth + this.paddingLeft + this.paddingRight;
}

Object.prototype.setWidth = function(width)
{
    this.width = this.scaledWidth = width;
    var scale = belle.display.scaledWidthFactor;
    if (scale != 1)
        this.scaledWidth *= scale;
    
    if (this.element) {
        if (typeof width == "string" && width.indexOf("%") !== -1)
            this.element.style.width = this.width;
        else
            this.element.style.width = this.scaledWidth + "px";
    }
}

Object.prototype.setHeight = function(height)
{
    this.height = this.scaledHeight = height;
    var scale = belle.display.scaledHeightFactor;
    if (scale != 1)
        this.scaledHeight *= scale;
    if (this.element) {
        if (typeof height == "string" && height.indexOf("%") !== -1)
            this.element.style.height = this.height;
        else
            this.element.style.height = this.scaledHeight + "px";
    }
}

Object.prototype.show = function()
{
    this.visible = true;
    this.redraw = true;
    if (this.element)
        this.element.style.display = "block";
}

Object.prototype.hide = function()
{
    this.visible = false;
    this.redraw = true;
    if (this.element)
        this.element.style.display = "none";
}

Object.prototype.setVisible = function(visible)
{
    if (visible)
        this.show();
    else
        this.hide();
}

Object.prototype.scale = function(widthFactor, heightFactor)
{
  this.setX(this.x);
  this.setY(this.y);
  this.setWidth(this.width);
  this.setHeight(this.height);
  this.redraw = true;
}

Object.prototype.initActions = function(actions) 
{
    var actionInstances = [];
    var actionInstance = [];
    var _Action;
    
    for(var i=0; i !== actions.length; i++) {
        if (! belle.getActionPrototype([actions[i].type]))
            continue;
        
        _Action = belle.getActionPrototype([actions[i].type]);
        actionInstance = new _Action(actions[i]);
        
        //Since the object should be calling this from it's constructor, it hasn't been added to the list of scene objects.
        //Thus we need to explicitly attribute the object.
        actionInstance.object = this;
        actionInstances.push(actionInstance);
    }
    
    return actionInstances;
}

Object.prototype.initElement = function() 
{
  if (! this.element)
    return;

  $(this.element).css("position", "absolute");
  $(this.element).width(this.scaledWidth);
  $(this.element).height(this.scaledHeight);
  if (! this.visible)
    $(this.element).hide();
  var $children = $(this.element).children();
  $children.css("position", "absolute");
  $children.css("width", "100%");
  $children.css("height", "100%");
  $children.css("display", "block");
  $children.css("filter", "inherit");
    
  if (this.opacityF() != 1) {
    this.setOpacity(this.opacity);
  }
  
  if (this.getBackgroundOpacityF() != 1)
    this.setBackgroundOpacity(this.getBackgroundOpacity());

  if (this.borderWidth && this.borderColor)
    this.element.style.border = this.borderWidth + "px" + " solid " + this.borderColor.toHex(); 
}

Object.prototype.serialize = function()
{
  var data = {};
  var serialize = belle.serialize;
  
  data["name"] = this.name;
  data["x"] = this.x;
  data["y"] = this.y;
  data["width"] = this.width;
  data["height"] = this.height;
  if (this.backgroundImage)
    data["backgroundImage"] = serialize(this.backgroundImage);
  
  data["backgroundColor"] = serialize(this.backgroundColor);
  data["cornerRadius"] = this.cornerRadius;
  if (this.borderWidth)
    data["borderWidth"] = this.borderWidth;
  if (this.borderColor)
    data["borderColor"] = serialize(this.borderColor);
  data["opacity"] = this.opacity;
  data["visible"] = this.visible;
  
  return data;
}

/*********** IMAGE OBJECT ***********/
function Image (data)
{
    Object.call(this, data);  
    this.interval = null;
    this.currentFrame = 0;
    this.image = null;
    if ("image" in data) {
        this.image = new AnimationImage(data["image"], this);
        if (this.element) {
	  $(this.element).append(this.image.img);
        }
    }
}

belle.utils.extend(Object, Image);

Image.prototype.paint = function(context)
{
    var draw = Object.prototype.paint.call(this, context);
    if (! draw)
        return false;

    this.image.paint(context, this.globalX(), this.globalY(), this.width, this.height);
    
    this.redraw = false;
    return true;
}

Image.prototype.isReady = function()
{
    var ready = Object.prototype.isReady.call(this);
    if (! ready)
        return ready;
   
    if (this.image)
        return this.image.isReady();

    return true;
}


/*********** CHARACTER ***********/

function Character(data)
{
    var path = "";
    var image;
    var state = null;
    this.states = {};
    this.textColor = new Color([255, 255, 255, 255]);

    if ("images" in data) {
        for(state in data["images"]) {
            image = new AnimationImage(data["images"][state], this);
            image.src = data.images[state];
            this.states[state] = image;
        }
        
        if (state) {
            data["image"] = this.states[state];
            this.image = new AnimationImage(this.states[state], this);
        }
    }
    
    Image.call(this, data);
}

belle.utils.extend(Image, Character);


/*********** TEXT BOX ***********/

function TextBox(info)
{
    Object.call(this, info);
    this.textLeftPadding = [];
    this.textTopPadding = [];
    this.textAlignment = [];
    this.textParts = [];
    this.textElement = document.createElement("div");
    belle.utils.initElement(this.textElement, info);
    this.textElement.style.display = "block";
    this.element.appendChild(this.textElement);
    this.font = belle.game.font;
    this.prevText = "";
    this.displayedText = "";
    this.textHeight = 0;

    this.load(info);
}

belle.utils.extend(Object, TextBox);

TextBox.prototype.load = function(data)
{
    if ("font" in data) 
        this.font = data["font"];
    this.textElement.style.font = this.font;
    
    if ("text" in data)
        this.setText(data["text"]);
	
    if ("textColor" in data)
        this.setTextColor(data["textColor"]);
    
    if ("textAlignment" in data) {
        this.textAlignment = data["textAlignment"].split("|");
    }
}

TextBox.prototype.paint = function(context)
{
    var draw = Object.prototype.paint.call(this, context);
    
    if (! draw)
        return false;
    
    this.redrawing = true;
    
    var width = this.width;
    var height = this.height;
    var x = this.globalX();
    var y = this.globalY();
    context.fillStyle = this.textColor.toString();
    var defaultFont = context.font;
    
    if (this.font)
       context.font = this.font;
    
    var text = game.replaceVariables(this.text);
    if (text != this.displayedText)
        this.alignText();
    this.displayedText = text;

    for (var i=this.textParts.length-1; i !== -1; --i) {
        context.fillText(this.textParts[i], x+this.textLeftPadding[i], y+this.textTopPadding[i], this.width);
    }
    
    context.font = defaultFont;
  
    this.redrawing = false;
    return true;
}

TextBox.prototype.alignText = function(text, size)
{
    if (! text && ! this.text)
        return;
    if (! text)
        text = game.replaceVariables(this.text);
    
    if (belle.display.DOM && this.textElement) {
        text = text.replace("\n", "<br/>");
        if (this.textAlignment.contains("HCenter"))
            this.textElement.style.textAlign = "center";
        else if (this.textAlignment.contains("Right"))
            this.textElement.style.textAlign = "right";  
        else
            this.textElement.style.textAlign = "left"; 
        
        if (this.textAlignment.contains("VCenter"))
            this.textElement.style.lineHeight = this.scaledHeight + "px";
        else if (this.textAlignment.contains("Bottom")) {
            this.textElement.style.lineHeight = 1;
            this.textElement.style.bottom = 0;
            this.textElement.style.height = "auto";
        }
    }
    else {    
        this.textParts = belle.utils.splitText(text, this.font, this.width);
        this.textLeftPadding.length = 0;
        this.textTopPadding.length = 0;
        var sumHeight = 0;
        
        for (var i=0; i < this.textParts.length; i++) {
            var size = belle.utils.textSize(this.textParts[i], this.font);
            var width = size[0];
            var height = size[1] / 1.2;
            var leftPadding = 0;
            sumHeight += height;
            this.textTopPadding.push(sumHeight);

            if (this.textAlignment) {
                if (width < this.width) {
                    if (this.textAlignment.contains("HCenter")) {
                        leftPadding = (this.width - width) / 2;
                    }
                    else if (this.textAlignment.contains("Right")) {
                        leftPadding = this.width - width;
                    }
                }
            }
            
            this.textLeftPadding.push(leftPadding);
        }
        
        if (sumHeight < this.height) {
            var topOffset = 0;
            if (this.textAlignment.contains("VCenter")) 
                topOffset = (this.height - sumHeight) / 2;
            else if (this.textAlignment.contains("Bottom"))
                topOffset = this.height - sumHeight;
            topOffset = topOffset > 0 ? topOffset : 0;
            
            for(var i=0; i < this.textTopPadding.length; i++)
                this.textTopPadding[i] += topOffset;
        }
    }
}

TextBox.prototype.needsRedraw = function()
{
    if (this.redraw)
        return true;
    
    if (! this.visible)
        return false;
    
    var displayText = game.replaceVariables(this.text);
    
    if (displayText != this.displayedText) {
        this.redraw = true;
        return true;
    }
    
    return false;
}

TextBox.prototype.appendText = function(text)
{
    this.setText(this.text + text);
}

TextBox.prototype.setText = function(text)
{
    if (this.text != text) {
        this.text = text;
        this.textElement.innerHTML = game.replaceVariables(text.replace("\n", "<br/>"));
        this.alignText();
        this.redraw = true;
    }
}

TextBox.prototype.setTextColor = function(color)
{
    if (color instanceof Array)
        color = new Color(color);
    this.textColor = color;
    
    if (this.textElement) {
        this.textElement.style.color = color.toHex();
    }
}

TextBox.prototype.scale = function(widthFactor, heightFactor)
{
  Object.prototype.scale.call(this, widthFactor, heightFactor);

  if (this.textElement) {
    this.textElement.style.width = this.scaledWidth + "px";
    this.textElement.style.height = this.scaledHeight + "px";
    var font = parseInt(this.font);
    if (font !== NaN) {
        font *= widthFactor;
        $(this.textElement).css("font-size", font+"px");
    }
  }
  this.alignText();
}

TextBox.prototype.serialize = function()
{
    var data = Object.prototype.serialize.call(this);
    data["text"] = this.text;
    return data;
}


/*********** Object Group ***********/

function ObjectGroup(data)
{
    Object.call(this, data);
    this.objects = [];
    this.hoveredObject = null;
    
    this.load(data);
}

belle.utils.extend(Object, ObjectGroup);

ObjectGroup.prototype.load = function(data) {
    if ("objects" in data) {	
        var obj;
        var objects = data["objects"];
        for (var i=0; i !== objects.length; i++) {
	    obj = this.getObject(objects[i].name);
	    if (obj) {
	      obj.load(objects[i]);
	      continue;
	    }
	    
            objects[i].__parent = this;
            obj = belle.createObject(objects[i]);
            
            if (! obj) {
                belle.log(objects[i].type + ": is not a valid object type!!!!.");
                continue;
            }
            var left = parseInt(this.element.style.left);
            var elemLeft = parseInt(obj.element.style.left);
            obj.setX(elemLeft - left);
            
            var top = parseInt(this.element.style.top);
            var elemTop = parseInt(obj.element.style.top);
            obj.setY(elemTop - top);
            
            //if (belle.display.DOM) {
                obj.element.style.display = "block";
                obj.element.style.left = obj.x + "px";
                //temporary hack in DOM mode to fix top/y value
                obj.element.style.top = obj.y + "px";
                this.element.appendChild(obj.element);
            //}
            
            this.objects.push(obj);
        }
    }
}

ObjectGroup.prototype.serialize = function()
{
    var data = Object.prototype.serialize.call(this);
    data["objects"] = belle.serialize(this.objects);
    return data;
}

ObjectGroup.prototype.objectAt = function(x, y)
{
    for(var i=0; i !== this.objects.length; i++) {
        if (this.objects[i].contains(x, y)) {
            return this.objects[i];
        }
    }
    
    return null;
}

ObjectGroup.prototype.paint = function(context)
{
    var draw = Object.prototype.paint.call(this, context);
    if (! draw)
        return false;
    
    for(var i=0; i !== this.objects.length; i++) {
        this.objects[i].redraw = true;
        this.objects[i].paint(context);
    }
    
    this.redraw = false;
    return true;
}

ObjectGroup.prototype.mouseLeaveEvent = function(event)
{
    Object.prototype.mouseLeaveEvent.call(this, event);
    if (this.hoveredObject) {
        this.hoveredObject.mouseLeaveEvent(event);
    }
}

ObjectGroup.prototype.processEvent = function(event)
{
    var x = event.canvasX;
    var y = event.canvasY;
    if (! this.visible || ! this.contains(x, y))
        return false;
    
    var result = Object.prototype.processEvent.call(this, event);
    var object = this.objectAt(x, y);

    if (this.hoveredObject && this.hoveredObject != object)
        this.hoveredObject.mouseLeaveEvent(event);
    
    if (event.type == "mousemove")
        this.hoveredObject = object;
    
    if (object)
        result = object.processEvent(event);

    return result;
}

ObjectGroup.prototype.clear = function (context)
{
  Object.prototype.clear.call(this, context);
  //context.clearRect(this.globalX()-this.borderWidth, this.globalY()-this.borderWidth, this.width+this.borderWidth*2, this.height+this.borderWidth*2);
  
  for(var i=0; i !== this.objects.length; i++) {
      this.objects[i].clear(context);
  }
}


ObjectGroup.prototype.needsRedraw = function()
{
    for (var i=0; i !== this.objects.length; i++) {
        if (this.objects[i].needsRedraw()) {
            this.redraw = true;
            break;
        }
    }
    
    return this.redraw;
}

ObjectGroup.prototype.scale = function(widthFactor, heightFactor)
{
  Object.prototype.scale.call(this, widthFactor, heightFactor);
  
  for (var i=0; i !== this.objects.length; i++) {
    this.objects[i].scale(widthFactor, heightFactor);
  }
}

ObjectGroup.prototype.isReady = function()
{
  var ready = Object.prototype.isReady.call(this);
  
  if (! ready)
      return false;
  
  for(var i=0; i < this.objects.length; i++){
    if (! this.objects[i].isReady()) {
        ready = false;
        break;
    }
  }
  
  return ready;
}

ObjectGroup.prototype.getObject = function(name)
{
  if (name) {
    for (var i=0; i !== this.objects.length; i++) {
      if (this.objects[i].name == name)
	return this.objects[i];
    }
  }
  
  return null;
}

/*********** DIALOGUE BOX ***********/

function DialogueBox(data)
{
    ObjectGroup.call(this, data);
    
    this.text = "";
    this.speakerName = "";
    this.speakerTextBox = null;
    this.dialgueTextBox = null;
    
    for(var i=0; i !== this.objects.length; i++) {
        if (this.objects[i].name === "speakerTextBox")
            this.speakerTextBox = this.objects[i];
        else if (this.objects[i].name === "dialogueTextBox")
            this.dialogueTextBox = this.objects[i];
    }
}

belle.utils.extend(ObjectGroup, DialogueBox);

DialogueBox.prototype.setSpeakerName = function(text)
{
    if (this.speakerTextBox) {
        this.speakerTextBox.setText(text);
        this.redraw = true;
    }
}

DialogueBox.prototype.appendText = function(text)
{
    if (this.dialogueTextBox) {
        this.dialogueTextBox.appendText(text);
        this.redraw = true;
    }
}

DialogueBox.prototype.setText = function(text)
{
    if (this.dialogueTextBox) {
        this.dialogueTextBox.setText(text);
        this.redraw = true;
    }
}

/************** MENU ************/

function Menu(data)
{
    ObjectGroup.call(this, data);
}

belle.utils.extend(ObjectGroup, Menu);



/************** BUTTON ************/

function Button(data)
{
    TextBox.call(this, data);
    this.visible = true;
}

belle.utils.extend(TextBox, Button);

// Expose the public methods

objects.Point = Point;
objects.AnimationImage = AnimationImage;
objects.Color = Color;
objects.Object = Object;
objects.Image = Image;
objects.TextBox = TextBox;
objects.Character = Character;
objects.ObjectGroup = ObjectGroup;
objects.DialogueBox = DialogueBox;
objects.Button = Button;
objects.Menu = Menu;

}(belle.objects));

belle.log("Objects module loaded!");

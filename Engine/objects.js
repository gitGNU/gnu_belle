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

/**** COLOR ****/
var Color = function(components)
{
    var error = false;
    if (components.length < 4) {
        error = true;
        _console.error("Color(components) : Missing one or more color component(s)");
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
    var that = this;
    
    
    //is it animated
    /*if (typeof imageData === "object") {
        
        if ("animated" in imageData) {
            this.animated = true;
        }
        else if ("frames" in imageData) {
            var frames = imageData["frames"];
            var image = null;
            this.frames = [];
            for (var i=0; i < frames.length; i++) {
                image = new window.Image();
                image.onload = function() { 
                    that.framesLoaded++;
                };
                image.src = frames[i];
                this.frames.push(image);
            }
            
            if ("frameDelay" in imageData && isNumber(imageData["frameDelay"]))
                this.frameDelay = parseInt(imageData["frameDelay"]);
            
            if (this.parent)
                this.interval = setInterval(function() { that.parent.frameChanged(); }, this.frameDelay);
        }
    }*/
    
    if (typeof imageData === "object" && "src" in imageData) {
        //activate DOM mode if game contains animated image
        if ("animated" in imageData && imageData["animated"])
            Novel.usingDOM = true;
        imageData = imageData["src"];
    }
    
    if (typeof imageData === "string") {
        this.img = new window.Image();
        this.img.onload = function() { 
            that.imageLoaded = true;
        };
        
        this.img.src = imageData;
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


var Belle  = (function() {

/*********** BASE OBJECT ***********/

function Object(info)
{
    if ("resource" in info && getResource(info["resource"])) {
        var resourceData = getResource(info["resource"]).data;
        extendJsonObject(info, resourceData);
    }
    
    //check for percent width or height and for percent widths or heights
    if ("__parent" in info) 
        var parent = info["__parent"];
    
    if (typeof info.width == "string" && isPercentSize(info.width) && parent && parseInt(parent.width) != NaN)
        info.width =  parseInt(info.width) * parent.width / 100;

    if (typeof info.height == "string" && isPercentSize(info.height) && parent && parseInt(parent.height) != NaN)
        info.height =  parseInt(info.height) * parent.height / 100;
    
    this.element = document.createElement("div");
    this.backgroundElement = document.createElement("div");
    this.element.appendChild(this.backgroundElement);
    
    initElement(this.element, info);
    initElement(this.backgroundElement, info);
    this.backgroundElement.style.display = "block";
    
    this.setX(info.x);
    this.setY(info.y);
    this.setWidth(info.width);
    this.setHeight(info.height);
    this.xRadius = 10;
    this.yRadius = 10;
    this.paintX = -1;
    this.paintY = -1;
    this.roundedRect = false;
    this.backgroundColor = new Color([255, 255, 255, 255]);
    this.fillStyle = this.backgroundColor.toString();
    this.interval = null;
    this.color = new Color([255,255,255,255]);
    this.mousePressActions = [];
    this.mouseReleaseActions = [];
    this.mouseMoveActions = [];
    this.mouseLeaveActions = [];
    this.backgroundImage = null;
    this.name = "";
    this.visible = false;
    this.context = null;
    this.redraw = false;
    this.redrawing = false;
    this.partsToRedraw = [];
    this.changing = false;
    this.clickable = false;
    this.loaded = true;
    this.moveToPoint = null;
    this.data = info;
    this.receivers = [];
    this.borderWidth = 0;
    this.borderColor = this.color;
    this.parent = parent ? parent : null;
    this.type = info["type"];

    var actions;
    var action;
    var actionObject;
    
    if ("name" in info)
        this.name = info["name"];
    
    if ("backgroundColor" in info) {
        this.setBackgroundColor(new Color(info["backgroundColor"]));
    }
    
    if ("onMousePress" in info) {
        this.mousePressActions = initActions(info["onMousePress"], this);
    }
    
    if ("onMouseRelease" in info) {
        this.mouseReleaseActions = initActions(info["onMouseRelease"], this);
    }
    
    if ("onMouseMove" in info) {
        this.mouseMoveActions = initActions(info["onMouseMove"], this);
    }
    
    if ("onMouseLeave" in info) {
        this.mouseLeaveActions = initActions(info["onMouseLeave"], this);
    }
    
    if ("xRadius" in info) {
        this.xRadius = info["xRadius"];
    }
    
    if ("yRadius" in info) {
        this.yRadius = info["yRadius"];
    }
    
    if ("roundedRect" in info) {
        this.roundedRect = info["roundedRect"];
    }
    
    if( "backgroundImage" in info) {
        this.backgroundImage = new AnimationImage(info["backgroundImage"], this);
        if (this.backgroundElement)
            this.backgroundElement.appendChild(this.backgroundImage.img);
    }
    
    if ("borderWidth" in info) {
      this.borderWidth = info["borderWidth"];
    }
    
    if ("borderColor" in info) {
      this.borderColor = new Color(info["borderColor"]);
    }
    
    if ("visible" in info) {
        this.visible = info["visible"];
        if (this.visible) {
            this.redraw = true;
        }
    }
    
    if (this.element) {
        this.element.style.border = this.borderWidth + "px" + " solid " + this.borderColor.toHex(); 
    }
}

Object.prototype.frameChanged = function()
{
    this.redraw = true;
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
    
    if ( otherX > paintX + width || otherY > paintY + height || paintX > otherX + otherWidth || paintY > otherY + otherHeight ) 
        return false;
        
    return true;
}

Object.prototype.overlapedRect = function(object) {
    var x=0, y=0, width=0, height=0;
   
    return null;
}

Object.prototype.setX = function(x)
{
    this.x = x;
    if (this.element)
        this.element.style.left = x + "px";
}

Object.prototype.setY = function(y)
{
    this.y = y;
    if (this.element)
        this.element.style.top = y + "px";
}

Object.prototype.contains = function(px, py)
{
    var x = this.x, y = this.y;
    if (this.parent) {
        x += this.parent.x;
        y += this.parent.y;        
    }

    if ( px >= x && px <=  x+this.width && py >= y && py <= y+this.height)
        return true;
    return false;
}

Object.prototype.setBackgroundOpacity = function(alpha)
{
    if (this.backgroundElement) {
        this.backgroundElement.style.opacity = alpha / 255; 
        //ie support
        this.backgroundElement.style.filter = "alpha(opacity=" + Math.round(100 * alpha / 255) + ");";
    }
    this.backgroundColor.alpha = alpha; 
}

Object.prototype.setBackgroundColor = function(color)
{
    this.backgroundColor = color;
    if (this.backgroundElement)
        this.backgroundElement.style.backgroundColor = color.toHex();
    this.setBackgroundOpacity(color.alpha);
    
}

Object.prototype.setColor = function (color)
{
    this.color = color;
    setOpacity(this.color.alpha);
}

Object.prototype.setOpacity = function (alpha)
{
    this.color.alpha = alpha;
    if (this.element) {
        this.element.style.opacity = alpha / 255;
        //ie support
        this.element.style.filter = "alpha(opacity=" + Math.round(100 * alpha / 255) + ");";
    }
}

Object.prototype.backgroundOpacity = function()
{
    return this.backgroundColor.alpha;
}

Object.prototype.opacity = function ()
{
    return this.color.alpha;
}

Object.prototype.paint = function(context)
{
    if (! this.redraw ||this.redrawing)
        return;
    
    this.redrawing = true;
    
    var x = this.globalX();
    var y = this.globalY();
    
    if (this.paintX != -1 && this.paintX != this.x || this.paintY != -1 && this.paintY != this.y) {
        context.clearRect(this.paintX, this.paintY, this.width, this.height);
    }
    
    if (this.backgroundImage) {
        this.backgroundImage.paint(context, x, y, this.width, this.height);
    }
    else if (this.roundedRect) {
        
        var width = this.width;
        var height = this.height;
        var xradius = this.xRadius;
        var yradius = this.yRadius;
    
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
        
        context.lineWidth = this.borderWidth;
       
        context.stroke();
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

Object.prototype.processEvent = function(event)
{
    var x = event.canvasX;
    var y = event.canvasY;
    
    if (! this.visible || ! this.contains(x, y))
        return false;
    
    var actions = [];

    if (event.type == "mousemove")
        actions = this.mouseMoveActions;
    else if (event.type == "mouseup") {
        actions = this.mouseReleaseActions;
    }
    else if (event.type == "mousedown")
        actions = this.mousePressActions;
    
    for(var i =0; i !== actions.length; i++) 
        actions[i].execute();
    
    this.notify(event);
    
    if (actions.length)
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

Object.prototype.addReceiver = function(receiver) 
{
    this.receivers.push(receiver);
}

Object.prototype.notify = function(event) {
  for(var i=0; i !== this.receivers.length; i++)
    this.receivers[i].receive(event);
}

Object.prototype.clear = function (context)
{
  if (! this.visible)
      return;
      
  context.clearRect(this.globalX()-this.borderWidth, this.globalY()-this.borderWidth, this.width+this.borderWidth*2, this.height+this.borderWidth*2);
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
    this.width = width;
    if (this.element) {
        if (typeof width == "string" && width.indexOf("%") !== -1)
            this.element.style.width = this.width;
        else
            this.element.style.width = width + "px";
    }
}

Object.prototype.setHeight = function(height)
{
    this.height = height;
    if (this.element) {
        if (typeof height == "string" && height.indexOf("%") !== -1)
            this.element.style.height = this.height;
        else
            this.element.style.height = height + "px";
    }
}

Object.prototype.show = function()
{
    this.visible = true;
    if (this.element)
        this.element.style.display = "block";
}

Object.prototype.hide = function()
{
    this.visible = false;
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
  this.x *= widthFactor;
  this.width *= widthFactor;
  this.y *= heightFactor;
  this.height *= heightFactor;
  if (this.element) {
    this.element.style.left = this.x + "px";
    this.element.style.width = this.width + "px";
    this.element.style.top = this.y + "px";
    this.element.style.height = this.height + "px";
    this.backgroundElement.style.width = this.width + "px";
    this.backgroundElement.style.height = this.height + "px";
  }
  this.redraw = true;
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
            this.image.img.style.display = "block";
            var firstNode = this.element.childNodes[0];
            this.element.insertBefore(this.image.img, firstNode);
        }
    }
}

extend(Object, Image);

Image.prototype.paint = function(context)
{
    Object.prototype.paint.call(this, context);
    
    context.save();
    if (context.globalAlpha != this.color.alphaF())
        context.globalAlpha = this.color.alphaF();
 
    this.image.paint(context, this.globalX(), this.globalY(), this.width, this.height);
    context.restore();
    
    this.redraw = false;
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
    
    Image.call(this, data);
    this.states = {};
    this.textColor = new Color([255, 255, 255, 255]);
    
    var image;
    if ("images" in data) {
        for(var property in data["images"]) {
            image = new window.Image();
            image.src = data.images[property];
            this.states[property] = image;
        }
        
        this.image.src = this.states[property].src;
    }
}


extend(Image, Character);


/*********** TEXT BOX ***********/

function TextBox(info)
{
    Object.call(this, info);
    this.textLeftPadding = 0;
    this.textTopPadding = 0;
    this.textAlignment = [];
    this.textElement = document.createElement("div");
    initElement(this.textElement, info);
    this.textElement.style.display = "block";
    this.element.appendChild(this.textElement);
    
    if ("text" in info)
        this.text = info["text"];
    
    if ("textColor" in info)
        this.setTextColor(info["textColor"]);
    
    if ("textAlignment" in info) {
        this.textAlignment = info["textAlignment"].split("|");
        /*var properties = info["textAlignment"].split("|");
        for (var i=0; i !== properties.length; i++) {
        }*/
    }
     
    var textNode = document.createTextNode(this.text);
    this.prevText = "";
    this.prevSize = [0, 60];
    this.textParts = [];
    this.displayedText = "";
    var size = textSize(Novel.font, this.text);
    var height = size[1];
    this.text = replaceVariables(this.text);
    this.alignText();
    this.textElement.appendChild(textNode);
}

extend(Object, TextBox);

TextBox.prototype.paint = function(context)
{
    if (! this.redraw || this.redrawing)
        return;
    
    Object.prototype.paint.call(this, context);
    this.redrawing = true;
    
    var width = this.width;
    var height = this.height;
    var x = this.globalX();
    var y = this.globalY();
    context.fillStyle = this.textColor.toString();
    
    /*if (this.prevText != this.text) {
        this.textParts = splitText(context.font, this.text, this.rect.width-this.leftPadding);
        //this.prevSize = textSize(context.font, this.text);
        this.prevText = this.text;
    }*/
    
    var text = replaceVariables(this.text);
    if (text != this.displayedText)
      this.textParts = splitText(context.font, text, width-this.textLeftPadding);
    this.displayedText = text;

    for (var i=this.textParts.length-1; i !== -1; --i) {
        context.fillText(this.textParts[i], x+this.textLeftPadding, y+this.textTopPadding+this.heightOffset*(i+1), this.width);
    }
  
    this.redrawing = false;
}

TextBox.prototype.alignText = function(size)
{
    if (! size)
        size = textSize(Novel.font, this.text);
    
    var width = size[0];
    var height = size[1];
    
    this.heightOffset = height / 1.2;
    
    if (this.text && this.textAlignment) {
        if (width < this.width) {
            if (this.textAlignment.contains("HCenter")) {
                this.textLeftPadding = (this.width - width) / 2;
                if (this.textElement)
                    this.textElement.style.textAlign = 'center'; 
            }
            else if (this.textAlignment.contains("Right")) {
                this.textLeftPadding = (this.width - width);
                if (this.textElement)
                    this.textElement.style.textAlign = 'right';                
            }
            else
                this.textLeftPadding = 0;
        }
        
        if (height < this.height) {
            if (this.textAlignment.contains("VCenter")) {
                this.textTopPadding = (this.height - height) / 2;
                if (this.textElement)
                    this.textElement.style.marginTop = this.textTopPadding + "px";
                
            }
            else if (this.textAlignment.contains("Bottom")) {
                this.textTopPadding = (this.height - height);
                if (this.textElement)
                    this.textElement.style.marginTop = this.textTopPadding + "px";
            }
            else
                this.textTopPadding = 0;
        }
    }
}


TextBox.prototype.needsRedraw = function()
{
    if (this.redraw)
        return true;
    
    var text = this.text;
    var newText = replaceVariables(text);
    
    if (newText != this.displayedText) {
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
    this.text = text;
    this.element.childNodes[1].childNodes[0].nodeValue = replaceVariables(text);
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
    this.textElement.style.width = this.width + "px";
    this.textElement.style.height = this.height + "px";
  }
  this.alignText();
}


/*********** Object Group ***********/

function ObjectGroup(data)
{
    Object.call(this, data);
    this.objects = [];
    this.hoveredObject = null;
    
    if ("objects" in data) {
        var obj;
        var objects = data["objects"];
        for (var i=0; i !== objects.length; i++) {
            objects[i].__parent = this;
            obj = createResource(objects[i]);
            
            if (! obj) {
                _console.error(objects[i].type + ": is not a valid object type. Ignoring...");
                continue;
            }
            var left = parseInt(this.element.style.left);
            var elemLeft = parseInt(obj.element.style.left);
            obj.x = elemLeft - left;
            
            var top = parseInt(this.element.style.top);
            var elemTop = parseInt(obj.element.style.top);
            obj.y = elemTop - top;
            
            if (Novel.usingDOM) {
                obj.element.style.display = "block";
                obj.element.style.left = obj.x + "px";
                //temporary hack in DOM mode to fix top/y value
                obj.element.style.top = obj.y + "px";
                this.element.appendChild(obj.element);
            }
            
            this.objects.push(obj);
        }
    }
}

extend(Object, ObjectGroup);

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
    if (! this.redraw)
        return;
    
    Object.prototype.paint.call(this, context);
    
    for(var i=0; i !== this.objects.length; i++) {
        this.objects[i].redraw = true;
        this.objects[i].paint(context);
    }
    
    this.redraw = false;
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
  if (! this.visible)
      return;
    
  context.clearRect(this.globalX()-this.borderWidth, this.globalY()-this.borderWidth, this.width+this.borderWidth*2, this.height+this.borderWidth*2);
  
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

/*********** DIALOGUE BOX ***********/

function DialogueBox(data)
{
    ObjectGroup.call(this, data);
    
    this.text = "";
    this.speakerName = "";
}

extend(ObjectGroup, DialogueBox);

DialogueBox.prototype.paint = function(context)
{
    if (! this.redraw)
        return;
    
    Object.prototype.paint.call(this, context);

    for(var i=0; i !== this.objects.length; i++) {
       
        if (this.objects[i].name === "speakerTextBox")
            this.objects[i].text = this.speakerName;
        else if (this.objects[i].name === "dialogueTextBox")
            this.objects[i].text = this.text;
        
        this.objects[i].redraw = true;
        this.objects[i].paint(context);
    }
    
    this.redraw = false;
}

/************** MENU ************/

function Menu(data)
{
    ObjectGroup.call(this, data);
}

extend(ObjectGroup, Menu);

Menu.prototype.paint = function(context)
{
    if (! this.redraw)
        return;
    
    Object.prototype.paint.call(this, context);

    for(var i=0; i !== this.objects.length; i++) { 
        this.objects[i].redraw = true;
        this.objects[i].paint(context);
    }
    
    this.redraw = false;
}


/************** BUTTON ************/

function Button(data)
{
    TextBox.call(this, data);
    this.visible = true;
}

extend(TextBox, Button);

/*********** SCENE ***********/

//function Scene(background, width, height)
function Scene(data)
{
    this.objects = [];
    this.actions = [];
    this.image = null;
    this.ready = false;
    this.redrawBackground = false;
    this.name = "";
    this.backgroundImage = null;
    this.backgroundColor = null;
    this.backgroundImageLoaded = false;
    var backgroundImage = "";
    var backgroundColor = null;
    this.width = Novel.width;
    this.height = Novel.height;
    this.x = 0;
    this.y = 0;
    data.width = this.width;
    data.height = this.height;
    this.visible = true;
    this.tries = 0;
    
    this.element = document.createElement("div");
    this.backgroundElement = document.createElement("div");
    this.element.appendChild(this.backgroundElement);
    this.element.id = data["name"];
    
    initElement(this.element, data);
    initElement(this.backgroundElement, data);
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

Scene.prototype.addObject = function(object) {
    this.objects[this.objects.length] = object;
    if (Novel.currentScene == this)
        addObject(object);
}

Scene.prototype.setBackgroundImage = function(background)
{
    var that = this;
    
    if (background instanceof AnimationImage) {
        if (this.backgroundImage && background == this.backgroundImage)
            return;
        this.backgroundImage = background;
        that.backgroundImageLoaded = true; //assume for now
        if (this.backgroundElement) {
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
            this.backgroundImage.src = null;
            this.backgroundImage = null;
        }
        
        if (this.backgroundColor)
            this.redrawBackground = true;
    }
}

Scene.prototype.paint = function(context)
{    
    if (this.backgroundImage) {
        this.backgroundImage.paint(context, 0, 0, Novel.width, Novel.height);
    }
    else if (this.backgroundColor) {
        context.fillStyle  = this.backgroundColor.toString();
        context.fillRect(0, 0, Novel.width, Novel.height);
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
    this.paint(Novel.bgContext);
}

Scene.prototype.scale = function(widthFactor, heightFactor)
{
    this.width *= widthFactor;
    this.height *= heightFactor;
    if (this.element) {
        this.element.style.width = this.width + "px";
        this.element.style.height = this.height + "px";
        this.backgroundElement.style.width = this.width + "px";
        this.backgroundElement.style.height = this.height + "px";
    }
}

// Return an object that exposes the public methods
return { 
    "Color": Color,
    "Object": Object,
    "Image": Image,
    "Character": Character,
    "TextBox": TextBox,
    "ObjectGroup": ObjectGroup,
    "DialogueBox": DialogueBox,
    "Scene": Scene,
    "Button": Button,
    "Menu": Menu
};

})();

_console.log("Objects loaded!");

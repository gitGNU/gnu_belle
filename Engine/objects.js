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
    var that = this;
      
    if (typeof imageData === "object" && "src" in imageData) {
        //activate DOM mode if game contains animated image
        if ("animated" in imageData && imageData["animated"])
            belle.display.usingDOM = true;
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

/**** COLOR ****/
var Color = function(components)
{
    var error = false;
    if (components.length < 4) {
        error = true;
        log("Color(components) : Missing one or more color component(s)");
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
    
    belle.utils.initElement(this.element, info);
    belle.utils.initElement(this.backgroundElement, info);
    this.backgroundElement.style.display = "block";
    
    this.setX(info.x);
    this.setY(info.y);
    this.setWidth(info.width);
    this.setHeight(info.height);
    this.xRadius = 10;
    this.yRadius = 10;
    this.paintX = false;
    this.paintY = false;
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
    this.setOpacity(this.color.alpha);
}

Object.prototype.setOpacity = function (alpha)
{
    if (alpha < 0)
        alpha = 0;
    else if (alpha > 255)
        alpha = 255;
    
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
        return false;
    
    this.redrawing = true;
        
    if (! this.visible) {
        this.redrawing = false;
        this.redraw = false;
        return false;
    }
    
    var x = this.globalX();
    var y = this.globalY();
    
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

belle.utils.extend(Object, Image);

Image.prototype.paint = function(context)
{
    var draw = Object.prototype.paint.call(this, context);
    if (! draw)
        return false;

    context.save();
    if (context.globalAlpha != this.color.alphaF())
        context.globalAlpha = this.color.alphaF();

    this.image.paint(context, this.globalX(), this.globalY(), this.width, this.height);
    context.restore();
    
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
    
    if ("font" in info) 
        this.font = info["font"];
    this.textElement.style.font = this.font;
        
    if ("text" in info)
        this.setText(info["text"]);
    
    if ("textColor" in info)
        this.setTextColor(info["textColor"]);
    
    if ("textAlignment" in info) {
        this.textAlignment = info["textAlignment"].split("|");
        /*var properties = info["textAlignment"].split("|");
        for (var i=0; i !== properties.length; i++) {
        }*/
    }
    
    //this.textWidth = belle.utils.textWidth(this.displayedText, this.font);
}

belle.utils.extend(Object, TextBox);

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

    context.save();
    if (context.globalAlpha != this.color.alphaF())
        context.globalAlpha = this.color.alphaF();
    
    var text = belle.replaceVariables(this.text);
    if (text != this.displayedText)
        this.alignText();
    this.displayedText = text;

    for (var i=this.textParts.length-1; i !== -1; --i) {
        context.fillText(this.textParts[i], x+this.textLeftPadding[i], y+this.textTopPadding[i], this.width);
    }
    
    context.restore();
    context.font = defaultFont;
  
    this.redrawing = false;
    return true;
}

TextBox.prototype.alignText = function(text, size)
{
    if (! text && ! this.text)
        return;
    if (! text)
        text = belle.replaceVariables(this.text);
    
    if (belle.display.usingDOM && this.textElement) {
        text = text.replace("\n", "<br/>");
        if (this.textAlignment.contains("HCenter"))
            this.textElement.style.textAlign = "center";
        else if (this.textAlignment.contains("Right"))
            this.textElement.style.textAlign = "right";  
        else
            this.textElement.style.textAlign = "left"; 
        
        var size = belle.utils.textSize(text, this.font);
        var height = size[1];
        
        if (height < this.height) {
            var topOffset = 0;
            if (this.textAlignment.contains("VCenter"))
                topOffset = (this.height - height) / 2;
            else if (this.textAlignment.contains("Bottom"))
                topOffset = this.height - height;
            topOffset = topOffset > 0 ? Math.round(topOffset) : 0;
            
            this.textElement.style.top = topOffset + "px";
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
    
    var displayText = belle.replaceVariables(this.text);
    
    if (displayText != this.displayedText) {
        this.redraw = true;
        return true;
    }
    
    var width = belle.utils.textWidth(displayText, this.font);
    
    if (this.textWidth != width) {
        this.textWidth = width;
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
        this.textElement.innerHTML = belle.replaceVariables(text.replace("\n", "<br/>"));
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
            obj = belle.createObject(objects[i]);
            
            if (! obj) {
                log(objects[i].type + ": is not a valid object type!!!!.");
                continue;
            }
            var left = parseInt(this.element.style.left);
            var elemLeft = parseInt(obj.element.style.left);
            obj.x = elemLeft - left;
            
            var top = parseInt(this.element.style.top);
            var elemTop = parseInt(obj.element.style.top);
            obj.y = elemTop - top;
            
            //if (belle.display.usingDOM) {
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

belle.utils.extend(Object, ObjectGroup);

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
    this.width = belle.game.width;
    this.height = belle.game.height;
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

Scene.prototype.addObject = function(object) {
    this.objects[this.objects.length] = object;
    if (belle.game.currentScene == this) {
        belle.display.addObject(object);
    }
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
        context.fillRect(0, 0, this.width, this.height);
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
    this.width *= widthFactor;
    this.height *= heightFactor;
    if (this.element) {
        this.element.style.width = this.width + "px";
        this.element.style.height = this.height + "px";
        this.backgroundElement.style.width = this.width + "px";
        this.backgroundElement.style.height = this.height + "px";
    }
}

// Expose the public methods

objects.Point = Point;
objects.AnimationImage = AnimationImage;
objects.Color = Color;
objects.AnimationImage = AnimationImage;
objects.Object = Object;
objects.Image = Image;
objects.TextBox = TextBox;
objects.Character = Character;
objects.ObjectGroup = ObjectGroup;
objects.DialogueBox = DialogueBox;
objects.Scene = Scene;
objects.Button = Button;
objects.Menu = Menu;

}(belle.objects));

log("Objects module loaded!");

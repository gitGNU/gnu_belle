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
        console.error("Color(components) : Missing one or more color component(s)");
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

Color.prototype.alphaF = function()
{
    return this.alpha / 255;
}


var Belle  = (function() {

/*********** RECT ***********/

var Rect = function (x, y, width, height) 
{
    this.x = x;
    this.y = y;
    this.paintX = -1;
    this.paintY = -1;
    this.width = width ;
    this.height = height;
    this.xRadius = 10;
    this.yRadius = 10;
    this.roundedRect = false;
    this.defaultColor = new Color([255, 255, 255, 255]);
    this.highlightColor = new Color([70, 70, 255, 255]);
    this.color = this.defaultColor;
    this.fillStyle = this.color.toString();
}

Rect.prototype.contains = function(x, y) 
{
    if ( x >= this.x && x <=  this.x+this.width && y >= this.y && y <= this.y+this.height)
        return true;
    return false;
}

Rect.prototype.getWidth = function() 
{
    return this.width;
}

Rect.prototype.getHeight = function() 
{
    return this.height;
}

Rect.prototype.setWidth = function(width) 
{
    this.width = width;
}

Rect.prototype.setHeight = function(height) 
{
    this.height = height;
}

Rect.prototype.setOpacity = function(opacity) 
{
    if (opacity > 1)
        opacity /= 255;
    
    this.color.alpha = opacity;
    this.fillStyle = this.color.toString();    
}

Rect.prototype.paint = function(context)
{
}

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
    
    if (typeof info.width == "string" || typeof info.height == "string") {
        if (isPercentSize(info.width) && parent && parent.rect && parent.rect.width && parseInt(parent.rect.width) != NaN)
            info.width =  parseInt(info.width) * parent.rect.width / 100;

        if (isPercentSize(info.height) && parent && parent.rect && parent.rect.height && parseInt(parent.rect.height) != NaN)
            info.height =  parseInt(info.height) * parent.rect.height / 100;
    }
    
    this.rect = new Rect(info.x, info.y, info.width, info.height);
    this.interval = null;
    this.color = new Color([255,255,255,255]);
    this.mousePressActions = [];
    this.mouseReleaseActions = [];
    this.mouseMoveActions = [];
    this.mouseLeaveActions = [];
    this.backgroundImage = null;
    this.backgroundImageLoaded = false;
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
    this.borderColor = this.rect.color;

    var actions;
    var action;
    var actionObject;
    
    if ("name" in info)
        this.name = info["name"];
    
    if ("backgroundColor" in info) {
        this.rect.color = new Color(info["backgroundColor"]);
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
        this.rect.xRadius = info["xRadius"];
    }
    
    if ("yRadius" in info) {
        this.rect.yRadius = info["yRadius"];
    }
    
    if ("roundedRect" in info) {
        this.rect.roundedRect = info["roundedRect"];
    }
    
    if( "backgroundImage" in info) {
        this.backgroundImage = new window.Image();
        var that = this;
        this.backgroundImage.onload = function() {
            that.backgroundImageLoaded = true;
        };
        this.backgroundImage.src = info["backgroundImage"]; 
    }
    
    if ("borderWidth" in info) {
      this.borderWidth = info["borderWidth"];
    }
    
    if ("borderColor" in info) {
      this.borderColor = info["borderColor"];
    }
    
    if ("visible" in info) {
        this.visible = info["visible"];
        if (this.visible)
            this.redraw = true;
    }
    
}

Object.prototype.overlaps = function(object)
{
    var rect = this.rect;
    var otherRect = object.rect;
    
    if ( otherRect.x > rect.x + rect.width || otherRect.y > rect.y + rect.height || rect.x > otherRect.x + otherRect.width || rect.y > otherRect.y + otherRect.height ) 
        return false;
    
    if ( otherRect.x > rect.paintX + rect.width || otherRect.y > rect.paintY + rect.height || rect.paintX > otherRect.x + otherRect.width || rect.paintY > otherRect.y + otherRect.height ) 
        return false;
        
    return true;
}

Object.prototype.overlapedRect = function(object) {
    var x=0, y=0, width=0, height=0;
   
    return null;
}

Object.prototype.setX = function(x)
{
   this.rect.x = x;
}

Object.prototype.setY = function(y)
{
    this.rect.y = y;
}

Object.prototype.contains = function(x, y)
{
    return this.rect.contains(x, y);
}

Object.prototype.setOpacity = function (opacity)
{
    this.opacity = opacity;
    
    if (opacity == 0)
        this.visible = false;
    else
        this.visible = true;
}

Object.prototype.paint = function(context)
{
    if (! this.redraw ||this.redrawing)
        return;
    
    this.redrawing = true;
    /*if ( this.moveToPoint ) {
        this.rect.x = this.moveToPoint.x;
        this.rect.y = this.moveToPoint.y;
        this.moveToPoint = null;
    }*/
    
    if (this.rect.paintX != -1 && this.rect.paintX != this.rect.x || this.rect.paintY != -1 && this.rect.paintY != this.rect.y) {
        context.clearRect(this.rect.paintX, this.rect.paintY, this.rect.width, this.rect.height);
    }
    
    if (this.backgroundImageLoaded) {
        context.drawImage(this.backgroundImage, this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    }
    else if (this.rect.roundedRect) {
        
        var width = this.rect.width;
        var height = this.rect.height;
        var x = this.rect.x;
        var y = this.rect.y;
        var xradius = this.rect.xRadius;
        var yradius = this.rect.yRadius;
    
        context.fillStyle  = this.rect.color.toString();
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
        context.fillStyle  = this.rect.color.toString();
        context.fillRect(this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    }
    
    this.rect.paintX = this.rect.x;
    this.rect.paintY = this.rect.y;
    
    this.redrawing = false;
    this.redraw = false;
}

Object.prototype.isReady = function()
{
    if (this.backgroundImage)
        return this.backgroundImage.complete;

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
    this.rect.x = x;
    this.rect.y = y;
    //this.moveToPoint = new Point(x, y);
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
      
  context.clearRect(this.rect.x-this.borderWidth, this.rect.y-this.borderWidth, this.rect.width+this.borderWidth*2, this.rect.height+this.borderWidth*2);
}

Object.prototype.contentWidth = function()
{
}

Object.prototype.contentHeight = function()
{
}

Object.prototype.fullWidth = function()
{
  return this.rect.width + this.borderWidth + this.paddingLeft + this.paddingRight;
}

Object.prototype.fullHeight = function()
{
  return this.rect.width + this.borderWidth + this.paddingLeft + this.paddingRight;
}

Object.prototype.scale = function(widthFactor, heightFactor)
{
  this.rect.x *= widthFactor;
  this.rect.width *= widthFactor;
  this.rect.y *= heightFactor;
  this.rect.height *= heightFactor;
  this.redraw = true;
}

/*********** IMAGE OBJECT ***********/
function Image (data)
{
    Object.call(this, data);  
    this.imageLoaded = false;
    this.frames = [];
    this.currentFrame = 0;
    this.frameDelay = 100; // 100ms by default
    this.framesLoaded = 0;
    this.image = null;
    this.interval = null;
    var that = this;
    
    //is it animated
    if ("frames" in data) {
        var frames = data["frames"];
        var image = null;
        for (var i=0; i < frames.length; i++) {
            image = new window.Image();
            image.onload = function() { 
                that.framesLoaded++;
            };
            image.src = frames[i];
            console.log(frames[i]);
            this.frames.push(image);
        }
        
        if ("frameDelay" in data && isNumber(data["frameDelay"]))
            this.frameDelay = Integer.parseInt(data["frameDelay"]);
        
        this.interval = setInterval(function() { that.frameChanged(); }, this.frameDelay);
    }
    else {
        this.image = new window.Image();
        this.image.onload = function() { 
            that.imageLoaded = true;
        };
        
        if ("image" in data && data["image"].length > 0)
            this.image.src = data["image"];
    }
}

extend(Object, Image);

Image.prototype.paint = function(context)
{
    Object.prototype.paint.call(this, context);
    
    context.save();
    if (context.globalAlpha != this.color.alphaF())
        context.globalAlpha = this.color.alphaF();
    
    if (this.image)
        context.drawImage(this.image, this.rect.x, this.rect.y, this.rect.width, this.rect.height);
    else if (this.frames) {
        context.drawImage(this.frames[this.currentFrame], this.rect.x, this.rect.y, this.rect.width, this.rect.height);
        this.currentFrame++;
        if (this.currentFrame >= this.frames.length)
            this.currentFrame = 0;
    }
    
    context.restore();
    
    //this.redraw = false;
}

Image.prototype.isLoaded = function()
{
    return this.loaded;
}

Image.prototype.isReady = function()
{
    var ready = Object.prototype.isReady.call(this);
    if (! ready)
        return ready;
    
    if (this.frames) {
        if (this.frames.length == this.framesLoaded)
            return true;
        else
            return false;
    }
    
    if (this.image)
        return this.image.complete;

    return true;
}

Image.prototype.frameChanged = function()
{
    this.redraw = true;
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
    
    if ("text" in info)
        this.text = info["text"];
    
    if ("textColor" in info)
        this.textColor = new Color(info["textColor"]);
    
    if ("textAlignment" in info) {
        this.textAlignment = info["textAlignment"].split("|");
        /*var properties = info["textAlignment"].split("|");
        for (var i=0; i !== properties.length; i++) {
        }*/
    }
        
    this.prevText = "";
    this.prevSize = [0, 60];
    this.textParts = [];
    this.displayedText = "";
    var size = textSize(Novel.font, this.text);
    var height = size[1];
    this.heightOffset = height / 3 + height / 2;
    this.text = replaceVariables(this.text);
    this.alignText();
}

extend(Object, TextBox);

TextBox.prototype.paint = function(context)
{
    if (! this.redraw || this.redrawing)
        return;
    
    Object.prototype.paint.call(this, context);
    this.redrawing = true;
    
    context.fillStyle = this.textColor.toString();
    
    /*if (this.prevText != this.text) {
        this.textParts = splitText(context.font, this.text, this.rect.width-this.leftPadding);
        //this.prevSize = textSize(context.font, this.text);
        this.prevText = this.text;
    }*/
    
    var text = replaceVariables(this.text);
    if (text != this.displayedText)
      this.textParts = splitText(context.font, text, this.rect.width-this.textLeftPadding);
    this.displayedText = text;

    for (var i=this.textParts.length-1; i !== -1; --i) {
        context.fillText(this.textParts[i], this.rect.x+this.textLeftPadding, this.rect.y+this.textTopPadding+this.heightOffset*(i+1), this.rect.width);
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
        if (width < this.rect.width) {
            if (this.textAlignment.contains("HCenter"))
                this.textLeftPadding = (this.rect.width - width) / 2;
            else if (this.textAlignment.contains("Right"))
                this.textLeftPadding = (this.rect.width - width);
            else
                this.textLeftPadding = 0;
        }
        
        if (height < this.rect.height) {
            if (this.textAlignment.contains("VCenter"))
                this.textTopPadding = (this.rect.height - height) / 2;
            else if (this.textAlignment.contains("Bottom"))
                this.textTopPadding = (this.rect.height - height);
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

TextBox.prototype.scale = function(widthFactor, heightFactor)
{
  Object.prototype.scale.call(this, widthFactor, heightFactor);
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
            console.log(objects[i]);
            obj = createResource(objects[i]);
            
            if (! obj) {
                console.error(objects[i].type + ": is not a valid object type. Ignoring...");
                continue;
            }
            this.objects.push(obj);
        }
    }
}

extend(Object, ObjectGroup);

ObjectGroup.prototype.objectAt = function(x, y)
{
    for(var i=0; i !== this.objects.length; i++) {
        if (this.objects[i].contains(x, y))
            return this.objects[i];
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
    if (this.hoveredObject)
        this.hoveredObject.mouseLeaveEvent(event);
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
    
  context.clearRect(this.rect.x-this.borderWidth, this.rect.y-this.borderWidth, this.rect.width+this.borderWidth*2, this.rect.height+this.borderWidth*2);
  
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
}

Scene.prototype.setBackgroundImage = function(background)
{
    if (this.backgroundImage && background == this.backgroundImage.src)
        return;
    var that = this;
    
    if (background instanceof window.Image) {
        this.backgroundImage = background;
        that.backgroundImageLoaded = true; //assume for now
    }
    else if (typeof background == "string") {
        this.backgroundImage = new window.Image();
        this.backgroundImage.onload = function() {
            that.backgroundImageLoaded = true;
        };
        this.backgroundImage.src = background;
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
    if (this.backgroundImageLoaded)
        context.drawImage(this.backgroundImage, 0, 0, Novel.width, Novel.height);
    else if (this.backgroundColor) {
        context.fillStyle  = this.backgroundColor.toString();
        context.fillRect(0, 0, Novel.width, Novel.height);
    }
}

Scene.prototype.isReady = function()
{
    if (this.backgroundImage)
        return this.backgroundImage.complete;
    return true;
}

// Return an object that exposes the public methods
return { 
    Color: Color,
    Rect : Rect,
    Object: Object,
    Image: Image,
    Character: Character,
    TextBox: TextBox,
    ObjectGroup: ObjectGroup,
    DialogueBox: DialogueBox,
    Scene: Scene,
    Button: Button,
    Menu: Menu
};

})();

console.log("Objects loaded!");

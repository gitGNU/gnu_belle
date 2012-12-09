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

/*********** ACTION ***********/

function Action(data)
{
    this.finished = false;
    this.interval = null;
    this.wait = null;
    this.needsRedraw = true;
    this.skippable = true;
    this.type = "Action";
    this.valid = true;
    this.elapsedTime = 0;
    var scene = data["__scene"];
    
    if (data) {

        if ("object" in data) {
            if (typeof data["object"] == "string")
                this.object = getResource(data["object"], scene);
            else if (typeof data["object"] == "object") {
                if (data["object"].name)
                    this.object = getResource(data["object"].name, scene);

                if (! this.object && Belle[data["object"].type])  {
                    var _Object = Belle[data["object"].type];
                    this.object = new _Object(data["object"]);
                }
            }
        }
            
        if ("skippable" in data)
            this.skippable = data["skippable"];
        
        if ("wait" in data)
            this.wait = new Wait(data["wait"]);
        
        if ("name" in data)
            this.name = data["name"];
        
        if ("type" in data)
            this.type = data["type"];
    }
}

Action.prototype.getInterval = function() {
    return this.interval;
}

Action.prototype.setInterval = function(interval) {
    this.interval = interval;
}

Action.prototype.isFinished = function() {
    return this.finished;
}

Action.prototype.setFinished = function(finished) {
    this.finished = finished;
}

Action.prototype.skip = function() 
{
    this.skipped = true;
}

Action.prototype.reset = function ()
{
    this.finished = false;
    this.skipped = false;
    if (this.interval) {
        clearInterval(this.interval)
        this.interval = null;
    }
    this.elapsedTime = 0;
    if (this.wait)
        this.wait.reset();
}

Action.prototype.execute = function() 
{
    this.reset();
    this.setFinished(true);
}

Action.prototype.receive = function(event) 
{
}

Action.prototype.isReady = function()
{
    return true;
}

Action.prototype.scale = function(widthFactor, heightFactor)
{
}

/*********** FADE ACTION ***********/

function Fade(data) 
{
    Action.call(this, data);
    
    if ("fadeType" in data)
        this.fadeType = data["fadeType"];
    if ("duration" in data)
        this.duration = data["duration"];
    
    this.duration *= 1000;
    this.time = this.duration / 255;
    this.timePassed = 0;
    //this.object.color.alpha = 0;
    this.target = 1;
    this.increment = 1;

    if (this.fadeType == "in") {
        if (this.object) {
            this.target = this.object.color.alpha;
            this.bgTarget = this.object.rect.color.alpha;
        }
        else {
            this.target = 255;
            this.bgTarget = 255;
        }
        this.increment = 1;
    }
    else if (this.fadeType == "out") {
        this.target = 0;
        this.bgTarget = 0;
        this.increment = -1;
    }
    else {
        error("PropertyError: Fade type '" + this.type + "' is not a valid type");
    }
}

extend(Action, Fade);

Fade.prototype.execute = function () {
    var t = this;
    
    //special case for fade in; fade out usually goes to zero
    if (this.fadeType == "in") {
        this.target = this.object.color.alpha;
        this.bgTarget = this.object.rect.color.alpha;
    }
    
    //this.reset();
    
    if (this.fadeType == "in") {
        this.object.rect.color.alpha  = 0;
        this.object.color.alpha = 0;
    }
     
    this.interval = setInterval(function() {t.fade();}, this.time);        
    this.object.redraw = true;
}

Fade.prototype.fade = function () {
    this.timePassed += this.time;
  
    if (this.timePassed >= this.duration ||
       (this.fadeType == "in" && this.object.color.alpha >= this.target) ||
       (this.fadeType == "out" && this.object.color.alpha <= this.target)) {
        clearInterval(this.interval);
        this.interval = null;
        this.finished = true;
        this.timePassed = 0;
        return;
    }
    
    if (this.fadeType == "in") {
      if (this.object.color.alpha < this.target)
          this.object.color.alpha += this.increment;
      if (this.object.rect.color.alpha < this.bgTarget)
          this.object.rect.color.alpha += this.increment;
    }
    else {
      if (this.object.color.alpha > this.target)
          this.object.color.alpha += this.increment;
      if (this.object.rect.color.alpha > this.bgTarget)
          this.object.rect.color.alpha += this.increment;
    }
      
    this.object.redraw = true;
}

Fade.prototype.skip = function () {
    if (! this.skippable)
        return;
    this.object.color.alpha = this.target;
    this.object.rect.color.alpha = this.bgTarget;
    Action.prototype.skip.call(this);
}

Fade.prototype.reset = function () {
    Action.prototype.reset.call(this);
    this.object.color.alpha = this.target;
    this.object.rect.color.alpha = this.bgTarget;
}

/*********** SLIDE ACTION ***********/

function Slide(data)
{
    Action.call(this, data);
    this.startPoint = new Point(data["startX"], data["startY"]);
    this.endPoint = new Point(data["endX"], data["endY"]);
    this.duration = 0;
    this.objectOriginalPosition = null;
    
    if ("duration" in data)
        this.duration = data["duration"];
    
    this.duration *= 1000;
    this.incX = 2;
    this.incY = 2;

    if (this.startPoint.x > this.endPoint.x)
        this.incX *= -1;

    if (this.startPoint.y > this.endPoint.y)
        this.incY *= -1;
    
}

extend(Action, Slide);

Slide.prototype.execute = function () 
{
    var t = this;
    this.reset();
    var duration = this.duration / this.startPoint.distance(this.endPoint);
    //duration *= 2;
    this.object.rect.x = this.startPoint.x;
    this.object.rect.y = this.startPoint.y;
    
    this.object.redraw = true;
    
    
    this.interval = setInterval(function() { t.slide(); }, duration);
}

Slide.prototype.slide = function () 
{   
    var x = this.object.rect.x, y = this.object.rect.y;
    if ((this.incX > 0 && this.object.rect.x < this.endPoint.x) ||
        (this.incX < 0 && this.object.rect.x > this.endPoint.x))
        x += this.incX;
    
    if ((this.incY > 0 && this.object.rect.y < this.endPoint.y) ||
        (this.incY < 0 && this.object.rect.y > this.endPoint.y))
        y += this.incY;
    
    //if x and y have NOT been modified, set action finished
    if (x === this.object.rect.x && y === this.object.rect.y) {
        clearInterval(this.interval);
        this.setFinished(true);
        return;
    }
  
    this.object.moveTo(x, y);
    this.object.redraw = true;
}

Slide.prototype.skip = function () {
    clearInterval(this.interval);
    this.object.rect.x = this.endPoint.x;
    this.object.rect.y = this.endPoint.y;
    this.setFinished(true);
}

Slide.prototype.reset = function () {
    
    Action.prototype.reset.call(this);
    
    if (this.objectOriginalPosition) {
        this.object.rect.x = this.objectOriginalPosition.x;
        this.object.rect.y = this.objectOriginalPosition.y;
    }
    
    this.setFinished(false);
}

Slide.prototype.scale = function(widthFactor, heightFactor)
{
    Action.prototype.scale.call(this, widthFactor, heightFactor);
    
    this.startPoint.x *= widthFactor;
    this.startPoint.y *= heightFactor;
    this.endPoint.x *= widthFactor;
    this.endPoint.y *= heightFactor;
}

/*********** DIALOGUE ACTION ***********/

function Dialogue(data)
{
    Action.call(this, data);
    
    this.character = null;
    this.speakerName = "";
    this.text = "";
    this.index = 0;
    this.lines = [];
    this.rawText = "";
    
    if (data) {
        if ("character" in data) {
            this.speakerName = data["character"];
            this.character = getResource(data["character"]);
            if (this.character) 
                this.speakerName = this.character.name;
        }
        
        if ("text" in data) {
            this.text = data["text"];
        }
    }
    
    this.rawText = this.text;
}

extend(Action, Dialogue);

Dialogue.prototype.execute = function () {
    var t = this;
    this.reset();
    
    if (! this.object) {
        this.setFinished(true);
        return;
    }
    
    if (this.object.speakerName !== undefined) {
        if (this.character) //in case character's name changed since the beginning
            this.object.speakerName = this.character.name;
        else
            this.object.speakerName = this.speakerName;
    }
   
    this.text = replaceVariables(this.text);
    
    //this.lines = splitText(context.font, this.text, this.object.rect.width-this.object.leftPadding);
    if (this.character)
        this.object.color = this.character.textColor;
    
    this.object.visible = true;
    this.object.redraw = true;
    this.interval = setInterval(function() { t.updateText(); }, Novel.textDelay);
}

Dialogue.prototype.updateText = function() {
 
    if (this.index == this.text.length) {
        clearInterval(this.interval);
        this.interval = null;
        this.setFinished(true);
        this.object.text = this.rawText;
        return;
    }

    this.object.text += this.text[this.index];
    this.index += 1;
    this.object.redraw = true;
}

Dialogue.prototype.setVisible = function (visible) 
{
    this.object.visible = visible;
}

Dialogue.prototype.skip = function () {
    for(var i=this.index; i !== this.text.length; i++)
        this.object.text += this.text[i];
    
    this.index = this.text.length;
    this.object.redraw = true;
}

Dialogue.prototype.reset = function () 
{
    Action.prototype.reset.call(this);
    this.index = 0;
    if (this.object)
        this.object.text = "";
    if (this.object && this.object.speakerName !== undefined)
        this.object.speakerName = "";
}

/*********** WAIT ACTION ***********/

function Wait(data)
{
    Action.call(this, data);
    
    if ( "time" in data)
        this.time = data["time"] * 1000;
    if ("waitType" in data) {
        this.waitType = data["waitType"];
        if (this.waitType == "MouseClick")
            this.skippable = true;
    }
    this.needsRedraw = false;
}

extend(Action, Wait);

Wait.prototype.execute = function ()
{
    var t = this;
    this.reset();
    if (this.waitType == "Timed" && this.time > 0) {
        setTimeout(function() {t.end(); }, this.time);
    }
    //else if ( this.waitType == "MouseClick" || this.waitType == "Forever" )
        
}

Wait.prototype.end = function ()
{
    this.setFinished(true);
}

Wait.prototype.skip = function (){
    if (this.skippable)
        this.setFinished(true);
    console.log("skipped");
}


/*********** CHANGE VISIBILITY Action ***********/

function ChangeVisibility(data)
{
    Action.call(this, data);
    this.transitions = [];
    this.duration = 0;
    
    if ("transitions" in data) {
        var transitions = data["transitions"];
        for(var i=0; i !== transitions.length; i++) {
            if (! window[transitions[i].type])
                continue;
            
            transitions[i].__scene = data.__scene;
            var transitionAction = window[transitions[i].type];
            var action = new transitionAction(transitions[i]);
            this.transitions.push(action);

            if (transitions[i].duration && transitions[i].duration > this.duration )
                this.duration = transitions[i].duration;
        }
    }
    
    this.duration *= 1000;
}

extend(Action, ChangeVisibility);

ChangeVisibility.prototype.execute = function () 
{
    //if (this.object.visible)
    //    return;
    
    this.reset();
    
    var that = this;
    for (var i=0; i < this.transitions.length; i++) {
        this.transitions[i].execute();
    }
    
    this.object.visible = true;
    
    if (this.transitions.length === 0)
        this.setFinished(true);
    else
        this.interval = setInterval(function(){ that.check(); }, this.duration);
}

ChangeVisibility.prototype.check = function () 
{
    for (var i=0; i < this.transitions.length; i++) 
        if (! this.transitions[i].isFinished())
            return;
    
    clearInterval(this.interval);
    this.setFinished(true);
}

ChangeVisibility.prototype.skip = function ()
{
    for (var i=0; i !== this.transitions.length; i++)
        this.transitions[i].skip();
}

ChangeVisibility.prototype.reset = function () 
{
    Action.prototype.reset.call(this);
    this.object.visible = false;
    this.object.redraw = true;
    
    for (var i=0; i !== this.transitions.length; i++)
        this.transitions[i].reset();
}

/*********** Show Action ***********/

function Show(data)
{
    ChangeVisibility.call(this, data);
    this.characterState = "";
    
}

extend(ChangeVisibility, Show);

Show.prototype.execute = function () 
{
    //if (this.object.visible)
    //    return;
    
    this.reset();
    
    var that = this;
    for (var i=0; i < this.transitions.length; i++) {
        this.transitions[i].execute();
    }
    
    this.object.visible = true;
    
    if (this.transitions.length === 0)
        this.setFinished(true);
    else
        this.interval = setInterval(function(){ that.check(); }, this.duration);
}

Show.prototype.check = function () 
{
   
    for (var i=0; i < this.transitions.length; i++) 
        if (! this.transitions[i].isFinished())
            return;
    
    clearInterval(this.interval);
    this.setFinished(true);
}

Show.prototype.skip = function ()
{
    ChangeVisibility.prototype.skip.call(this);
}

Show.prototype.reset = function () 
{
    Action.prototype.reset.call(this);
    this.object.visible = false;
    this.object.redraw = true;
    
    for (var i=0; i !== this.transitions.length; i++)
        this.transitions[i].reset();
}

Show.prototype.scale = function (widthFactor, heightFactor) 
{
    //Action.prototype.scale.call(this, widthFactor, heightFactor);
        
    for (var i=0; i < this.transitions.length; i++) 
        this.transitions[i].scale(widthFactor, heightFactor);
}

/*********** HIDE CHARACTER ACTION ***********/

function Hide(data)
{
    ChangeVisibility.call(this, data);
}

extend(ChangeVisibility, Hide);

Hide.prototype.execute = function () 
{
    //this.reset();
    
    var that = this;
    for (var i=0; i < this.transitions.length; i++) {
        this.transitions[i].execute();
    }
    
    this.object.visible = true;
    
    if (this.transitions.length === 0) {
        this.setFinished(true);
        this.object.visible = false;
    }
    else
        this.interval = setInterval(function(){ that.check(); }, this.duration);
}

Hide.prototype.check = function () 
{
    for (var i=0; i < this.transitions.length; i++) 
        if (! this.transitions[i].isFinished())
            return;
    
    clearInterval(this.interval);
    this.object.visible = false;
    this.setFinished(true);
}

Hide.prototype.reset = function () 
{
    ChangeVisibility.prototype.reset.call(this);
    this.object.visible = true;
    this.object.redraw = true;
}

/************* Change Background *****************/

function ChangeBackground(data)
{
    Action.call(this, data);
    
    this.backgroundImage = null;
    this.backgroundColor = null;
    
    if ("backgroundImage" in data){
         this.backgroundImage = new window.Image();
         this.backgroundImage.src = data["backgroundImage"];
    }
    
    if ("backgroundColor" in data)
        this.backgroundColor = data["backgroundColor"];
}

extend(Action, ChangeBackground);

ChangeBackground.prototype.execute = function () 
{
    this.reset();
    
    if (Novel.currentScene) {
        if (this.backgroundImage)
            Novel.currentScene.setBackgroundImage(this.backgroundImage);
        if (this.backgroundColor)
            Novel.currentScene.setBackgroundColor(this.backgroundColor);
    }
    
    this.setFinished(true);
}

/************* LABEL *****************/

function Label (data)
{
    Action.call(this, data);
    this.needsRedraw = false;
}

extend (Action, Label);

/************* Go TO LABEL *****************/

function GoToLabel(data)
{
    Action.call(this, data);
    if ("label" in data)
        this.label = data["label"];
    this.needsRedraw = false;
}

extend(Action, GoToLabel);

GoToLabel.prototype.execute = function()
{
   this.reset();
   
   var currentScene = Novel.currentScene;
   
   if (! currentScene || ! currentScene.actions) {
        this.setFinished(true);
        return;
   }
   
    
   if (this.label instanceof String || typeof this.label === 'string') {
        for (var i=0; i !== currentScene.actions.length; i++) {
            
            if (currentScene.actions[i].name === this.label) {
                this.resetActions(i, Novel.nextAction-1); 
                Novel.currentAction.setFinished(true);
                Novel.nextAction = i;
                break;
            }
        }
   }
   else if (this.label instanceof Label || typeof this.object === 'label'){
       if (currentScene.actions.indexOf(this.label) != -1)
            window.nextAction = Novel.currentScene.actions.indexOf(this.label);
   }
 
   this.setFinished(true);
}

GoToLabel.prototype.resetActions = function(from, to)
{
    var actions =  Novel.currentScene.actions;
    for(var i=from; i < to && i < actions.length; i++) {
        actions[i].reset();
    }
}

/************* Go To Scene *****************/

function GoToScene(data)
{
    Action.call(this, data);
    this.scene = "";
    if ("scene" in data && typeof data["scene"] === "string")
        this.scene = data["scene"];
    
    this.needsRedraw = false;
}

extend(Action, GoToScene);

GoToScene.prototype.execute = function()
{
   this.reset();
   
   var scenes = Novel.scenes;
   
   if (this.scene) {
        for (var i=0; i !== scenes.length; i++) {
            
            if (this.scene == scenes[i].name) {
              
                Novel.nextAction = Novel.currentScene.actions.length;
                Novel.nextScene = i;
                Novel.currentAction.setFinished(true);
                break;
            }
        }
   }

   this.setFinished(true);
}


/************* Branch *****************/

function Branch(data)
{
    Action.call(this, data);
    this.condition = "";
    this.trueActions = [];
    this.falseActions = [];
    
    var action;
    var actions;
    var _Action;
   
    if ( "trueActions" in data) {
        actions = data["trueActions"];
        for(var i=0; i < actions.length; i++) {
          _Action = window[actions[i].type];
          if (Action)
            this.trueActions.push(new _Action(actions[i]));
        }
    }
    
    if ( "falseActions" in data) {
        actions = data["falseActions"];
        for(var i=0; i < actions.length; i++) {
          _Action = window[actions[i].type];
          if (Action)
            this.falseActions.push(new _Action(actions[i]));
        }
    }
    
    if ("condition" in data)
      this.loadCondition(data["condition"]);
    
    /*if ("condition" in data) {
        var condition = data["condition"];
        var parts = condition.split(" ");
        var keywords = ["in", "true", "false", "defined", "undefined", "and", "or"];
        var operators = ["==", "!=" , ">" , ">=", "<", "<="];
        
        for(var i=0; i < parts.length; i++) {
            if (keywords.indexOf(parts[i]) !== -1 || parseInt(parts[i]) !== NaN || parts[i].indexOf('"') != -1)
                continue;
            parts[i] = "Novel.variables['" + parts[i] + "']";
        }
        
        if (condition.indexOf(" contains ") !== -1) {
            var index = 0;
            while(parts.indexOf("contains") != -1) {
                index = parts.indexOf("contains");
                if (index+1 < parts.length && index-1 >= 0) {
                    parts[index-1] = parts[index-1] + ".indexOf(" + parts[index+1] + ") !== -1";
                    parts.splice(index, 2);
                }
            }
        }
        
        condition = parts.join(" ");
    }*/
}

extend(Action, Branch);

Branch.prototype.execute = function()
{
  var result = eval(this.condition);
  if (result) {
    for(var i=0; i < this.trueActions.length; i++) {
      this.trueActions[i].execute();
    }
    
  }
  else {
    for(var i=0; i < this.falseActions.length; i++)
      this.falseActions[i].execute();
  }
  
  this.setFinished(true);
}

Branch.prototype.loadCondition = function(condition)
{
    var symbol = "";
    var name = "";
    var _in = "in";
    var _and = "and";
    var _or = "or";
    var letter = /^[a-zA-Z]$/g;
    var number = /^[0-9]|([0-9]*\.[0-9]+)$/g;
    var variable = /^[a-zA-Z]+[0-9]*$/g;
    var c = "";
    var i = 0;
    var conditionParts = [];
    var symbols = ["==", "!=", ">" , ">=", "<", "<=", "&&", "||"];
    var string = false;
  
    for(i=0; i < condition.length; i++) {
        c = condition[i];
        
        if (c == '"' || c == '\'')
            string = !string;

        //if string add everything to the same value
        if (string) {
            name += c;
            continue;
        }

        if (c.search(letter) != -1 || c.search(number) != -1 || c == '"' || c == '\'') {
            if (! name) {
                if (condition.slice(i, _in.length) == _in) {
                    i += _in.length;
                    conditionParts.push(_in);
                }
                else if (condition.slice(i, _and.length) == _and) {
                    i += _and.length;
                    conditionParts.push(_and);
                }
                else if (condition.slice(i, _or.length) == _or) {
                    i += _or.length;
                    conditionParts.push(_or);
                }
                else
                    name += c;
            }
            else
                name += c;
        }
        else if (c == '.') {
          if (! string) {
            if (name){
                if (parseInt(name) === NaN)
                    break;
            }
            else
                name += "0";
          }
          name += c;
        }
        else {

            if (name) {
                if (name.search(variable) != -1)
                  conditionParts.push("Novel.variables['" + name + "']");
                else
                  conditionParts.push(name);
                name = "";
            }
             
            if (c == ' ') {
                //do nothing :)
            }
            else if (c == '&' || c == '|') {
                symbol += c;
                if (i+1 < condition.length && condition[i+1] == c) {
                    symbol += condition[i+1];
                    ++i;
                }
            }
            else if (c == '>' || c == '=' || c == '<' || c == '!') {
                symbol += c;
                //in case of ">=" or "<="
                if (i+1 < condition.length && condition[i+1] == '=') {
                    symbol += condition[i+1];
                    ++i;
                }
            }
            else
                continue;

            if (symbols.indexOf(symbol) != -1) {
                conditionParts.push(symbol);
                symbol = "";
            }
        }
    }
    
    if (name) {
        if (name.search(variable) != -1)
          conditionParts.push("Novel.variables['" + name + "']");
        else
          conditionParts.push(name);
    }
    
    this.condition = conditionParts.join(" ");
}

/************* Change Color *****************/

function ChangeColor(data)
{
    Action.call(this, data);
    this.color = new Color([255, 255, 255, 255]);
    this.previousObjectColor = null;
    this.previousObjectBackgroundColor = null;
    this.changeObjectColor = true;
    this.changeObjectBackgroundColor = false;
    
    if ( "color" in data) {
        this.color = new Color(data["color"]);
    }
    
    if ( "changeObjectColor" in data) {
        this.changeObjectColor = data["changeObjectColor"];
    }
    
    if ( "changeObjectBackgroundColor" in data) {
        this.changeObjectBackgroundColor = data["changeObjectBackgroundColor"];
    }
}

extend(Action, ChangeColor);

ChangeColor.prototype.execute = function()
{
    this.reset();
   
    if (! this.object) {
        this.setFinished(true);
        return;
    }
    
    if (this.changeObjectColor) {
        this.previousObjectColor = this.object.color;
        this.object.color = this.color;
    }
    
    if (this.changeObjectBackgroundColor) {
        this.previousObjectBackgroundColor = this.object.rect.color;
        this.object.rect.color = this.color;
    }
    
    if (this.changeObjectColor || this.changeObjectBackgroundColor) {
        this.object.redraw = true;
    }
    
    this.setFinished(true);
}

ChangeColor.prototype.reset = function()
{
    Action.prototype.reset.call(this);
        
    if (this.changeObjectColor && this.previousObjectColor) {
        this.object.color = this.previousObjectColor;
    }
    
    if (this.changeObjectBackgroundColor && this.previousObjectBackgroundColor) {
        this.object.rect.color = this.previousObjectBackgroundColor;
    }
    
    if (this.changeObjectColor || this.changeObjectBackgroundColor)
        this.object.redraw = true;
    
    this.setFinished(false);
}

/************* Play Sound *****************/

function PlaySound(data)
{
    Action.call(this, data);
    
    this.soundPath = "";
    this.soundName = "";
    this.sound = null;
    this.loop = false;
    this.formats = [];
    this.volume = 100;
    var ext = "";
    
    if ( "sound" in data) {
        this.soundPath = data["sound"];
        var fileParts = this.soundPath.split(".");
        this.soundName = fileParts[fileParts.length-2];
        if (! this.soundName)
          this.soundName = this.soundPath;
    }
    
    if ("loop" in data) {
        this.loop = data["loop"];
    }
    
    if ("volume" in data)
        this.volume = data["volume"]; 
    
    if ("formats" in data) {
        this.formats = data["formats"];
    }  
        
}

extend(Action, PlaySound);

PlaySound.prototype.execute = function()
{
    this.reset();
   
    if (! this.soundPath) {
        this.setFinished(true);
        return;
    }
    
    this.sound = new buzz.sound(this.soundPath);
    //sound.setVolume(this.volume);
    
    if (this.loop)
        this.sound.play().loop();
    else
        this.sound.play();
    
    this.setFinished(true);
}

PlaySound.prototype.reset = function () {
    
    Action.prototype.reset.call();
    
    if (this.sound) {
        this.sound.stop();
    }
}


/************* Stop Sound *****************/

function StopSound(data)
{
    Action.call(this, data);
    
    this.fade = 0;
    this.sound = null;
    
    //can be the name of the file or the name of the action playing it
    if ( "sound" in data) 
        this.soundPath = data["sound"];
    
    if ("fade" in data && typeof data["fade"] === "number") 
        this.fade = data["fade"];
        
}

extend(Action, StopSound);

StopSound.prototype.execute = function()
{
    this.reset();

    this.sound = this.getSound(this.soundPath);
    
    if (this.sound) {
        if (this.fade > 0)
            this.sound.fadeOut(this.fade);
        else
            this.sound.stop();
    }
            
    this.setFinished(true);
}

StopSound.prototype.getSound = function (name) { 
    /*
    *  If name is null, this function returns the last PlaySound action from the current scene. 
    *  If name is a valid string, it's matched against previous PlaySound actions' names.
    *  If the above fails, it attempts to match PlaySound's sound name.
    */
    
    var actions = Novel.actions;
 
    for (var i=Novel.nextAction-2; i >= 0; i--) {
        //The sound can be matched by either 
        if (actions[i].type == "PlaySound" && (! name || actions[i].name == name || actions[i].soundName == name) ) {
            return actions[i].sound;
        }
    }
    
    return null;
}

StopSound.prototype.reset = function () {
    
    Action.prototype.reset.call();
    
    if (this.sound) {
        this.sound.play();
    }
}


/************* Show Menu *****************/

function ShowMenu(data)
{
    Action.call(this, data);
    
    this.options = 0;
   
    if ( "options" in data && typeof data["options"] == "number") 
        this.options = options; 
    
    if (this.object)
        this.object.addReceiver(this);
}

extend(Action, ShowMenu);

ShowMenu.prototype.execute = function()
{
    this.reset();

    if (this.object) {
        Novel.currentScene.objects.push(this.object);
    }
}

ShowMenu.prototype.receive = function(event) 
{
  if (event.type == "mouseup") {
    this.object.visible = false;
    this.object.redraw = true;
  
    this.setFinished(true);
  }
  
}

ShowMenu.prototype.scale = function(widthFactor, heightFactor)
{
    Action.prototype.scale.call(this, widthFactor, heightFactor);
    
    if (this.object)
        this.object.scale(widthFactor, heightFactor);
}

/************* End Novel *****************/

function EndNovel(data)
{
    Action.call(this, data);
}

extend(Action, EndNovel);

EndNovel.prototype.execute = function()
{
    Novel.end = true;  
    this.setFinished(true);
}

/************* Get user input *****************/

function GetUserInput(data)
{
    Action.call(this, data);
    this.variable = null;
    this.defaultValue = "";
    this.message = "Insert value here:";
    
    if ( "variable" in data) {
        this.variable = data["variable"];
    }
    
    if ( "message" in data ) {
        this.message = data["message"];
    }
    
    if ( "defaultValue" in data ) {
        this.defaultValue = data["defaultValue"];
    }
    
}

extend(Action, GetUserInput);

GetUserInput.prototype.execute = function()
{
    this.reset();
   
    if (! this.variable) {
        this.setFinished(true);
        return;
    }
       
    var value = prompt(this.message, this.defaultValue);
    if (! value)
        value = this.defaultValue;
    Novel.variables[this.variable] = value;
    
    this.setFinished(true);
}

GetUserInput.prototype.reset = function()
{
    Action.prototype.reset.call(this);
    this.value = null;
    //if (Novel.containsVariable())
}

/************* Game Variable *****************/

function GameVariable(data)
{
    Action.call(this, data);
    
    this.variable = "";
    this.operation = "";
    this.value = "";
    this.validOperators = ["assign", "add", "subtract", "multiply", "divide", "append"];
    
    if ("variable" in data)
        this.variable = data["variable"];
    
    if ("operator" in data)
        this.operator = data["operator"];
    
    if ("value" in data)
        this.value = data["value"];
    
    if (! this.validOperators.contains(this.operator))
        error("PropertyError: Invalid operator '" + this.operator  + "'");
    

}

extend(Action, GameVariable);

GameVariable.prototype.execute = function()
{   
    var currValue = "";
    var newValue = this.value;
    if (Novel.containsVariable(this.variable))
        currValue = Novel.value(this.variable);
    
    //console.log("SET GAME VARIABLE");
    //console.log(this.variable + " " + this.operator + " " + this.value + " " + currValue);
    
    //if arithmetic operation
    if (this.validOperators.slice(1,5).contains(this.operator)) {
        if (typeof currValue == "string") {
          if (currValue == "")
            currValue = "0";
          currValue = parseFloat(currValue);
        }
        
        if (typeof newValue == "string")
            newValue = parseFloat(newValue);
    }
 
    switch(this.operator) {
        case "assign": 
            currValue = newValue;
            break;
        case "add": 
            if (currValue != NaN && newValue != NaN)
                currValue += newValue;
            break;    
        case "subtract": 
            if (currValue != NaN && newValue != NaN)
                currValue -= newValue;
            break;
        case "multiply": 
            if (currValue != NaN && newValue != NaN)
                currValue *= newValue;
            break;
        case "divide":
            if (currValue != NaN && newValue != NaN && newValue != 0)
                currValue /= newValue;
            break;
        case "append":
            currValue += newValue + "";
            break;
    }
    
    Novel.variables[this.variable] = currValue;

    this.setFinished(true);
}

console.log("Actions loaded!");


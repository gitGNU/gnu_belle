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

/*********** ACTION ***********/

var belle = belle ||  {};
belle.actions = {};

(function(actions) {

    
actions.init = function()
{
    Color = belle.objects.Color;
    Point = belle.objects.Point;
    utils = belle.utils;
}
    
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
                this.object = belle.getObject(data["object"], scene);
            else if (typeof data["object"] == "object") {
                if (data["object"].name)
                    this.object = belle.getObject(data["object"].name, scene);

                if (! this.object && belle.objects[data["object"].type])  {
                    var _Object = belle.getObjectPrototype(data["object"].type);
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
    this.intervalDuration = this.duration / 255;
    this.timePassed = 0;
    this.prevTime = 0;
    //this.object.color.alpha = 0;
    this.target = 1;
    this.increment = 1;

    if (this.fadeType == "in") {
        if (this.object) {
            this.target = this.object.color.alpha;
            this.bgTarget = this.object.backgroundColor.alpha;
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

belle.utils.extend(Action, Fade);

Fade.prototype.execute = function () {
    var t = this;
    
    this.prevTime = new Date().getTime();
    this.timePassed = 0;
    
    //special case for fade in; fade out usually goes to zero
    if (this.fadeType == "in") {
        this.target = this.object.opacity();
        this.bgTarget = this.object.backgroundOpacity();
    }
    
    //this.reset();
    
    if (this.fadeType == "in") {
        this.object.setBackgroundOpacity(0);
        this.object.setOpacity(0);
    }
     
    this.interval = setInterval(function() {t.fade();}, this.intervalDuration);        
    this.object.redraw = true;
}

Fade.prototype.fade = function () {
    
    var now = new Date().getTime();
    var passed = now - this.prevTime;
    this.timePassed += passed;
    this.prevTime = now;
    
    if (this.timePassed >= this.duration) {
        this.object.setOpacity(this.target);
        this.object.setBackgroundOpacity(this.bgTarget);
    }
    
    var backgroundOpacity = this.object.backgroundOpacity();
    var opacity = this.object.color.alpha;
    
    if ((this.fadeType == "in" && opacity >= this.target) ||
       (this.fadeType == "out" && opacity <= this.target)) {
        clearInterval(this.interval);
        this.interval = null;
        this.finished = true;
        this.timePassed = 0;
        return;
    }
    
    passed = passed > this.intervalDuration ? passed : this.intervalDuration;
    var increment = passed * this.increment / this.intervalDuration; 
    if (this.fadeType == "in") {
      if (opacity < this.target)
          this.object.setOpacity(opacity + increment);
      if (backgroundOpacity < this.bgTarget)
          this.object.setBackgroundOpacity(backgroundOpacity + increment);
    }
    else {
      if (opacity > this.target)
          this.object.setOpacity(opacity+increment);
      if (backgroundOpacity > this.bgTarget)
          this.object.setBackgroundOpacity(backgroundOpacity + increment);
    }
      
    this.object.redraw = true;
}

Fade.prototype.skip = function () {
    if (! this.skippable)
        return;
    this.object.setOpacity(this.target);
    this.object.setBackgroundOpacity(this.bgTarget);
    Action.prototype.skip.call(this);
}

Fade.prototype.reset = function () {
    Action.prototype.reset.call(this);
    this.object.setOpacity(this.target);
    this.object.setBackgroundOpacity(this.bgTarget);
}

/*********** SLIDE ACTION ***********/

function Slide(data)
{
    Action.call(this, data);
    this.startPoint = new Point(data["startX"], data["startY"]);
    this.endPoint = new Point(data["endX"], data["endY"]);
    this.duration = 0;
    this.timePassed = 0;
    this.intervalDuration = 0;
    this.prevTime = 0;
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

belle.utils.extend(Action, Slide);

Slide.prototype.execute = function () 
{
    var t = this;
    this.reset();
    this.timePassed = 0;
    this.prevTime = new Date().getTime();
    this.intervalDuration = Math.round(this.duration / this.startPoint.distance(this.endPoint));
    
    this.object.setX(this.startPoint.x);
    this.object.setY(this.startPoint.y);
    
    this.object.redraw = true;
    this.interval = setInterval(function() { t.slide(); }, this.intervalDuration);
}

Slide.prototype.slide = function () 
{   
    var now = new Date().getTime();
    var passed = now - this.prevTime;
    this.timePassed += passed;
    this.prevTime = now;

    if (this.timePassed >= this.duration) {
        this.object.setX(this.endPoint.x);
        this.object.setY(this.endPoint.y);
    }
    
    passed = passed > this.intervalDuration ? passed : this.intervalDuration;
    var incX = passed * this.incX / this.intervalDuration;
    var incY = passed * this.incY / this.intervalDuration;
    var x = this.object.x, y = this.object.y;
    
    if ((incX > 0 && this.object.x < this.endPoint.x) ||
        (incX < 0 && this.object.x > this.endPoint.x))
        x += incX;
    
    if ((incY > 0 && this.object.y < this.endPoint.y) ||
        (incY < 0 && this.object.y > this.endPoint.y))
        y += incY;
    
    //if x and y have NOT been modified, set action finished
    if (x === this.object.x && y === this.object.y) {
        clearInterval(this.interval);
        this.setFinished(true);
        return;
    }
  
    this.object.moveTo(x, y);
    this.object.redraw = true;
}

Slide.prototype.skip = function () {
    clearInterval(this.interval);
    this.object.setX(this.endPoint.x);
    this.object.setY(this.endPoint.y);
    this.setFinished(true);
}

Slide.prototype.reset = function () {
    this.timePassed = 0;
    Action.prototype.reset.call(this);
    
    if (this.objectOriginalPosition) {
        this.object.setX(this.objectOriginalPosition.x);
        this.object.setY(this.objectOriginalPosition.y);
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
            this.character = belle.getObject(data["character"]);
        }
        
        if ("text" in data) {
            this.text = data["text"];
        }
    }

    this.rawText = this.text;
}

belle.utils.extend(Action, Dialogue);

Dialogue.prototype.execute = function () {
    var t = this;
    this.reset();
    
    if (! this.object) {
        this.setFinished(true);
        return;
    }
    
    if (typeof this.object.setSpeakerName == "function") {
        if (this.character) //in case character's name changed since the beginning
            this.object.setSpeakerName(this.character.name);
        else
            this.object.setSpeakerName(this.speakerName);
    }
    this.object.setText("");
    this.text = belle.replaceVariables(this.text);
    
    //this.lines = splitText(context.font, this.text, this.object.rect.width-this.object.leftPadding);
    if (this.character)
        this.object.color = this.character.textColor;
    
    this.object.redraw = true;
    this.interval = setInterval(function() { t.updateText(); }, belle.game.textDelay);
}

Dialogue.prototype.updateText = function() {
 
    if (this.index == this.text.length) {
        clearInterval(this.interval);
        this.interval = null;
        this.setFinished(true);
        this.object.setText(this.rawText);
        return;
    }

    this.object.appendText(this.text.charAt(this.index));
    this.index += 1;
}

Dialogue.prototype.skip = function () {
    this.object.appendText(this.text.slice(this.index));
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
        else if (this.waitType == "Forever")
            this.skippable = false;
    }
    this.needsRedraw = false;
}

belle.utils.extend(Action, Wait);

Wait.prototype.execute = function ()
{
    var t = this;
    this.reset();
    if (this.waitType == "Timed" && this.time > 0) {
        setTimeout(function() {t.end(); }, this.time);
    }
}

Wait.prototype.end = function ()
{
    this.setFinished(true);
}

Wait.prototype.skip = function (){
    if (this.skippable)
        this.setFinished(true);
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
            if (! belle.getActionPrototype(transitions[i].type)) {
                log("TypeError: Action '" + transitions[i].type + "' is not a valid Action"); 
                continue;
            }
            
            transitions[i].__scene = data.__scene;
            var transitionAction = belle.getActionPrototype([transitions[i].type]);
            var action = new transitionAction(transitions[i]);
            this.transitions.push(action);

            if (transitions[i].duration && transitions[i].duration > this.duration )
                this.duration = transitions[i].duration;
        }
    }
    
    this.duration *= 1000;
}

belle.utils.extend(Action, ChangeVisibility);

ChangeVisibility.prototype.execute = function () 
{
    //if (this.object.visible)
    //    return;
    
    this.reset();
    
    var that = this;
    for (var i=0; i < this.transitions.length; i++) {
        this.transitions[i].execute();
    }
    
    this.object.setVisible(true);
    
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
    this.object.setVisible(false);
    this.object.redraw = true;
    
    for (var i=0; i !== this.transitions.length; i++)
        this.transitions[i].reset();
}

ChangeVisibility.prototype.scale = function (widthFactor, heightFactor) 
{
    //Action.prototype.scale.call(this, widthFactor, heightFactor);
        
    for (var i=0; i < this.transitions.length; i++) 
        this.transitions[i].scale(widthFactor, heightFactor);
}

/*********** Show Action ***********/

function Show(data)
{
    ChangeVisibility.call(this, data);
    this.characterState = "";
    
}

belle.utils.extend(ChangeVisibility, Show);

Show.prototype.execute = function () 
{
    //if (this.object.visible)
    //    return;
    
    this.reset();
    
    var that = this;
    for (var i=0; i < this.transitions.length; i++) {
        this.transitions[i].execute();
    }
    
    this.object.setVisible(true);
    
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
    this.object.setVisible(false);
    this.object.redraw = true;
    
    for (var i=0; i !== this.transitions.length; i++)
        this.transitions[i].reset();
}

/*********** HIDE CHARACTER ACTION ***********/

function Hide(data)
{
    ChangeVisibility.call(this, data);
}

belle.utils.extend(ChangeVisibility, Hide);

Hide.prototype.execute = function () 
{
    //this.reset();
    
    var that = this;
    for (var i=0; i < this.transitions.length; i++) {
        this.transitions[i].execute();
    }
    
    if (this.transitions.length === 0) {
        this.setFinished(true);
        this.object.setVisible(false);
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
    this.object.setVisible(false);
    this.setFinished(true);
}

Hide.prototype.reset = function () 
{
    ChangeVisibility.prototype.reset.call(this);
    this.object.setVisible(true);
    this.object.redraw = true;
}

/************* Change Background *****************/

function ChangeBackground(data)
{
    Action.call(this, data);
    
    this.backgroundImage = null;
    this.backgroundColor = null;

    if ("backgroundImage" in data){
         this.backgroundImage = new belle.objects.AnimationImage(data["backgroundImage"]);
    }
    
    if ("backgroundColor" in data)
        this.backgroundColor = data["backgroundColor"];
}

belle.utils.extend(Action, ChangeBackground);

ChangeBackground.prototype.execute = function () 
{
    this.reset();
    var game = belle.game;
    
    if (game.currentScene) {
        if (this.backgroundImage)
            game.currentScene.setBackgroundImage(this.backgroundImage);
        if (this.backgroundColor)
            game.currentScene.setBackgroundColor(this.backgroundColor);
    }
    
    this.setFinished(true);
}

/************* LABEL *****************/

function Label (data)
{
    Action.call(this, data);
    this.needsRedraw = false;
}

belle.utils.extend (Action, Label);

/************* Go TO LABEL *****************/

function GoToLabel(data)
{
    Action.call(this, data);
    if ("label" in data)
        this.label = data["label"];
    this.needsRedraw = false;
}

belle.utils.extend(Action, GoToLabel);

GoToLabel.prototype.execute = function()
{
   this.reset();
   var game = belle.game;
   
   var currentScene = game.currentScene;
   
   if (! currentScene || ! currentScene.actions) {
        this.setFinished(true);
        return;
   }
   
    
   if (this.label instanceof String || typeof this.label === 'string') {
        for (var i=0; i !== currentScene.actions.length; i++) {
            
            if (currentScene.actions[i].name === this.label) {
                this.resetActions(i, game.nextAction-1); 
                game.currentAction.setFinished(true);
                game.nextAction = i;
                break;
            }
        }
   }
   else if (this.label instanceof Label || typeof this.object === 'label'){
       if (currentScene.actions.indexOf(this.label) != -1)
            window.nextAction = game.currentScene.actions.indexOf(this.label);
   }
 
   this.setFinished(true);
}

GoToLabel.prototype.resetActions = function(from, to)
{
    var actions =  belle.game.currentScene.actions;
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

belle.utils.extend(Action, GoToScene);

GoToScene.prototype.execute = function()
{
   this.reset();
   
   var game = belle.game;
   var scenes = game.scenes;
   
   if (this.scene) {
        for (var i=0; i !== scenes.length; i++) {
            
            if (this.scene == scenes[i].name) {
              
                game.nextAction = game.currentScene.actions.length;
                game.nextScene = i;
                game.currentAction.setFinished(true);
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
          _Action = belle.getActionPrototype([actions[i].type]);
          if (Action)
            this.trueActions.push(new _Action(actions[i]));
        }
    }
    
    if ( "falseActions" in data) {
        actions = data["falseActions"];
        for(var i=0; i < actions.length; i++) {
          _Action = belle.getActionPrototype([actions[i].type]);
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

belle.utils.extend(Action, Branch);

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
    condition = condition.split("");
  
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
                  conditionParts.push("belle.game.variables['" + name + "']");
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
          conditionParts.push("belle.game.variables['" + name + "']");
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

belle.utils.extend(Action, ChangeColor);

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
        this.previousObjectBackgroundColor = this.object.backgroundColor;
        this.object.setBackgroundColor(this.color);
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
        this.object.setColor(this.previousObjectColor);
    }
    
    if (this.changeObjectBackgroundColor && this.previousObjectBackgroundColor) {
        this.object.setBackgroundColor(this.previousObjectBackgroundColor);
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
        this.soundPath = belle.game.directory + data["sound"];
        this.soundName = this.soundPath;
        if (this.soundPath.indexOf("/") !== -1) {
            var parts = this.soundPath.split("/");
            this.soundName = parts[parts.length-1];
        }
        if (this.soundName.indexOf(".") !== -1)
            this.soundName = this.soundName.split(".")[0];
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

belle.utils.extend(Action, PlaySound);

PlaySound.prototype.execute = function()
{
    this.reset();
   
    if (! this.soundPath) {
        this.setFinished(true);
        return;
    }
    
    this.sound = new buzz.sound(this.soundPath);
    this.sound.setVolume(this.volume);
    
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
        this.fade = data["fade"] * 1000;
        
}

belle.utils.extend(Action, StopSound);

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
    
    var game = belle.game;
    var actions = game.actions;
 
    for (var i=game.nextAction-2; i >= 0; i--) {
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

belle.utils.extend(Action, ShowMenu);

ShowMenu.prototype.execute = function()
{
    this.reset();

    if (this.object) {
        belle.addObject(this.object);
    }
}

ShowMenu.prototype.receive = function(event) 
{
  if (event.type == "mouseup") {
    this.object.setVisible(false);
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

belle.utils.extend(Action, EndNovel);

EndNovel.prototype.execute = function()
{
    belle.game.end = true;  
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

belle.utils.extend(Action, GetUserInput);

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
    belle.insertVariable(this.variable, value);
    
    this.setFinished(true);
}

GetUserInput.prototype.reset = function()
{
    Action.prototype.reset.call(this);
    this.value = null;
}

/************* Change Game Variable *****************/

function ChangeGameVariable(data)
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

belle.utils.extend(Action, ChangeGameVariable);

ChangeGameVariable.prototype.execute = function()
{   
    var currValue = "";
    var newValue = this.value;
    if (belle.containsVariable(this.variable))
        currValue = belle.value(this.variable);
    
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
    
    belle.insertVariable(this.variable, currValue);

    this.setFinished(true);
}

//Expose objects
actions.Fade = Fade;
actions.Slide = Slide;
actions.Dialogue = Dialogue;
actions.Wait = Wait;
actions.Show = Show;
actions.Hide = Hide;
actions.ChangeBackground = ChangeBackground;
actions.Label = Label;
actions.GoToLabel = GoToLabel;
actions.GoToScene = GoToScene;
actions.Branch = Branch;
actions.ChangeColor = ChangeColor;
actions.PlaySound = PlaySound;
actions.StopSound = StopSound;
actions.ShowMenu = ShowMenu;
actions.GetUserInput = GetUserInput;
actions.ChangeGameVariable = ChangeGameVariable;

}(belle.actions));

log("Actions module loaded!");


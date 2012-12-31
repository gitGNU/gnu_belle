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

//New methods for Javascript's Array and String objects
if (typeof String.prototype.startsWith != 'function') 
{
  String.prototype.startsWith = function (str) {
    return this.slice(0, str.length) == str;
  };
}

if (typeof Array.prototype.indexOf != 'function')
{
  Array.prototype.indexOf = function(value, start)
  {
    if (! start)
        start = 0;
    
    for (var i=start; i < this.length; i++)
        if (this[i] === value)
            return i;
   
    return -1;
  };
}

if (typeof String.prototype.contains != 'function') 
{
    String.prototype.contains = function(text) { 
        return this.indexOf(text) != -1; 
    };
}

if(! String.prototype.trim) {  
  String.prototype.trim = function () {  
    return this.replace(/^\s+|\s+$/g,'');  
  };  
}

if (typeof String.prototype.isDigit != 'function') 
{
    String.prototype.isDigit = function(text) { 
        text = text.trim();
        return ! isNaN(text); 
    };
}

if (typeof Array.prototype.contains != 'function') 
{
    Array.prototype.contains = function(text) { 
        return this.indexOf(text) != -1; 
    };
}

function surrogateCtor() {}
    
function extend(base, sub) 
{
// Copy the prototype from the base to setup inheritance
surrogateCtor.prototype = base.prototype;
// Tricky huh?
sub.prototype = new surrogateCtor();
// Remember the constructor property was set wrong, let's fix it
sub.prototype.constructor = sub;
}
 
function updateSize(object)
{
    if (object.image) {
        object.rect.width = object.image.width;
        object.rect.height = object.image.height;
    }
    else {
        object.width = object.background.width;
        object.height = object.background.height;
    }
        
}

function getBody()
{
    var body = document.getElementById("body");
    if (! body)
        body = document.body;
    if (! body)
        body = document.getElementById("body")[0];
    
    return body;
}

function textSize(font, text) 
{
    var body = getBody();
    if (! body)
        return [-1, -1];
    
    size = [];
    
    var dummy = document.getElementById("dummy");
    dummy.style.font = font;
    var textNode = dummy.childNodes[0];
    textNode.nodeValue = text;
    
    size.push(dummy.offsetWidth);
    textNode.nodeValue = "ABCDEFGHIJKLMNOPQRSTUVXWYZ";
    size.push(dummy.offsetHeight);

    return size;
}

function splitText(font, text, maxWidth) 
{
    var body = getBody();
    var fullText = text;
    if (! body || ! fullText)
        return [];
    
    var dummy = document.getElementById("dummy");
    dummy.style.font = font;
    var textNode = dummy.childNodes[0];
    textNode.nodeValue = fullText;

    var width = 0;
    maxWidth -= 4;
   
    /*dummy.setAttribute("style", "font:"+font+";"+"position: absolute; visibility: hidden; height: auto; width: auto; border: none; padding: 0 0; margin: 0 0; white-space: nowrap;");
    body.appendChild(dummy);*/
    width = dummy.offsetWidth;
    
    if ( width > maxWidth ) {
        var breaks = Math.ceil(width / maxWidth);
        var textSplitted = [];
        var textsize = Math.floor(fullText.length / breaks);
        var txt = "";
        var i, j = 0;
        var start = 0;
        
        for(i=0; i < breaks || j < fullText.length; i++) { 
            
            //skip whitespaces
            for (; fullText[j] == " " && j < textsize*(i+1) && j < fullText.length; j++)
                continue;
            
            //probable text for a line
            for (; j < textsize*(i+1) && j < fullText.length; j++)
                txt += fullText[j];
        
            textNode.nodeValue = txt;
            
            //test if that text really fits in a line and correct it if it doesn't
            if (dummy.offsetWidth > maxWidth) {
                while(dummy.offsetWidth > maxWidth) {
                    txt = txt.substr(0, txt.length-1);
                    textNode.nodeValue = txt;
                    j--;
                }
            }
            else if (dummy.offsetWidth < maxWidth){
                            
                while(dummy.offsetWidth < maxWidth) {
                    if (j >= fullText.length) 
                        break;

                    txt += fullText[j];
                    textNode.nodeValue = txt;
                    j++;
                }
            }
            
            //fix split words
            if (j > 0 && j < fullText.length) {
                if (fullText[j-1] != " " && fullText[j] != " ") {
                    var k = 0;
                    while( fullText[j] != " " ) {
                        --j;
                        k++;
                    }
                    txt = txt.substr(0, txt.length-k);
                }
            }

            textSplitted.push(txt);
            txt = "";
        }

        return textSplitted;
        
    }
    
    return [fullText];
}

function replaceVariables(text)
{
    if (! text)
        return text;
    
    if (! text.contains("$"))
        return text;
    
    var validChar = /^[a-zA-Z]+[0-9]*$/g;
    var variable = "";
    var variables = [];
    var values = [];
    var appendToVariable = false;
    
    //Parse text to determine variables, which start from "$" 
    //until the end of string or until any other character that is not a letter nor a digit.
    for(var i=0; i !== text.length; i++) {
      
      if (text[i].search(validChar) == -1) {
        appendToVariable = false;          
        if (variable)
          variables.push(variable);
        variable = "";
        if(text[i] == "$")
          appendToVariable = true;
      }
        
      if (appendToVariable)
        variable += text[i];
    }
    
    //replace variables with the respective values and append them to the values list
    for(var i=0; i != variables.length; i++) {
        if (Novel.containsVariable(variables[i]))
          values.push(Novel.value(variables[i]));
        else //if the variable is not found, still add an empty value to the values so we have an equal number of elements in both lists
          values.push("");
    }
    
    //replace variables with the values previously extracted
    for(var i=0; i != values.length; i++) {
      text = text.replace(variables[i], values[i]);
    }
    
    return text;
}

function initActions(actions, object) 
{
    var actionInstances = [];
    var actionInstance = [];
    var _Action;
    
    for(var i=0; i !== actions.length; i++) {
        if (! window[actions[i].type])
            continue;
        
        _Action = window[actions[i].type];
        actionInstance = new _Action(actions[i]);
        
        //if 'object' is passed, we overwrite the previous object assigned, which should be null anyways.
        //The 'object' is usually passed when we're initializing actions from events of a certain object.
        //Since the object should be calling this from it's constructor, it hasn't been added to the list of scene objects.
        //Thus we need to explicitly attribute the object.
        if (object)
            actionInstance.object = object;
        
        actionInstances.push(actionInstance);
    }
    
    return actionInstances;
    
}

function createResource(data)
{
    var type = data["type"];
    if (! type) {
        var resource = data["resource"];
        if ("resource" in data && resource in Novel.resources) {
            type = Novel.resources[resource].data.type;
        }
        else
            return null;
    }

    var obj = Belle[type];
    
    if (! obj) {
        _console.error("'" + type + "' is not a valid object type.");
        return null;
    }

    return new obj(data);
}

function getResource(name, scene)
{
    if (! name)
        return null;
    
    if (scene) {
       var objects = scene.objects;
        
       for (var i = 0; i !== objects.length; i++) {
           if (objects[i].name === name)
               return objects[i];
       }
    }
    
    if (name in Novel.resources)
        return Novel.resources[name];
        
    return null;
}

function extendJsonObject(a, b)
{
    for(var key in b)
        if(! a.hasOwnProperty(key))
            a[key] = b[key];
}

function isPercentSize(value)
{
    if (typeof value != "string")
        return false;

    return value.search(/[0-9]+\%/g) != -1;
}

function parseSize(value)
{
    if (parseInt(value) != NaN)
        return parseInt(value);
    
    if (isPercentSize(value)) {
        value.split("%")
        var parts = value.split("%");
        if (parts && parseInt(parts[0]) != -1)
            return parseInt(value);
    }

    return 0;
}

function isNumber(n) {
  return !isNaN(parseFloat(n)) && isFinite(parseFloat(n));
}

function initElement(element, info)
{
    element.style.position = "absolute";
    if (typeof info.width == "string" && info.width.indexOf("%") != -1)
        console.log(info.name, info.__parent, info.width);
    element.style.width = info.width + "px";
    element.style.height = info.height + "px";
    element.style.display = "none";
}

function isCanvasSupported() {
  var elem = document.createElement("canvas");
  return !!(elem.getContext && elem.getContext('2d'));
}

function windowWidth() 
{
    return window.innerWidth || document.documentElement.clientWidth || document.documentElement.offsetWidth;
}

function windowHeight()
{
    return window.innerHeight || document.documentElement.clientHeight || document.documentElement.offsetHeight;
}

/*********** POINT **********/
function Point (x, y)
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

_console = (function() {
    
    function log (text) {        
        if (window.console)
            console.log(text);
    }
    
   function error(text) {
        if (window.console)
            console.error(text);
    }
    
    return { 
        "log": log,
        "error": error
    };
})();

function addJavascript(jsname,pos) 
{
    var th = document.getElementsByTagName(pos)[0];
    var s = document.createElement('script');
    s.setAttribute('type','text/javascript');
    s.setAttribute('src',jsname);
    th.appendChild(s);
} 

_console.log("Utils loaded!");

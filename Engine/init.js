belle = {};
belle.game = {
    "resources" : {},
    "actions": [],
    "customFonts" : [],
    "currentScene" : null,
    "nextScene" : 0,
    "scenes" : [],
    "ready" : false,
    "nextAction" : 0,
    "width": 640,
    "height" : 480,
    "currentWidth": 640,
    "currentHeight": 480,
    "variables" : {},
    "end" : false,
    "textSpeed" : 10,
    "textDelay" : 100,
    "filename" : "game.json",
    "directory" : "",
    "states" : {},
    "data" : null,
    "paused": false
};

belle.game.pauseScreen = {
    "currentScene" : null,
    "nextScene" : 1,
    "scenes" : [],
    "actions": [],
    "nextAction" : 0,
    "currentAction": null,
    "states" : {},
    "end": false
};

belle.actions = {};
belle.objects = {};
belle.display = {};
belle.utils = {};
belle.stateMachine = {};

//Extend Array and String objects
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

if (typeof String.prototype.trim != 'function') 
{
    String.prototype.trim =  function(){
        return this.replace(/^\s+|\s+$/g, '');
        
    };
}

if (typeof String.prototype.isDigit != 'function') 
{
    String.prototype.isDigit = function(text) { 
        text = text.trim();
        return ! isNaN(text); 
    };
}

if (typeof String.prototype.hasBreakPointAt != 'function') 
{
    String.prototype.hasBreakPointAt = function(i) { 
        return this[i] == " " || this[i] == "\n"; 
    };
}

if (typeof Array.prototype.contains != 'function') 
{
    Array.prototype.contains = function(text) { 
        return this.indexOf(text) != -1; 
    };
}


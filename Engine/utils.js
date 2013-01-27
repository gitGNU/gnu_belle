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

var belle = belle || {};
belle.utils = {};

(function(utils) {

var dummy = null;
function surrogateCtor() {}
 
function getBody()
{
    var body = document.getElementById("body");
    if (! body)
        body = document.body;
    if (! body)
        body = document.getElementById("body")[0];
    
    return body;
}

var parseSize = function(value)
{
    if (parseInt(value) != NaN)
        return parseInt(value);
    
    if (utils.isPercentSize(value)) {
        value.split("%")
        var parts = value.split("%");
        if (parts && parseInt(parts[0]) != -1)
            return parseInt(value);
    }

    return 0;
}
 
utils.extend = function (base, sub) 
{
    // Copy the prototype from the base to setup inheritance
    surrogateCtor.prototype = base.prototype;
    // Tricky huh?
    sub.prototype = new surrogateCtor();
    // Remember the constructor property was set wrong, let's fix it
    sub.prototype.constructor = sub;
}

utils.textSize = function(text, font) 
{
    var size = [];
    if (! dummy)
        dummy = document.getElementById("dummy");

    dummy.style.font = font;
    var textNode = dummy.childNodes[0];
    textNode.nodeValue = text;
    
    size.push(dummy.offsetWidth);
    textNode.nodeValue = "ABCDEFGHIJKLMNOPQRSTUVXWYZ0123456789";
    size.push(dummy.offsetHeight);

    return size;
}

utils.textWidth = function(text, font) 
{
    if (! font)
        return 0;
    
    if (! dummy)
      dummy = document.getElementById("dummy");
    dummy.style.font = font;
    var textNode = dummy.childNodes[0];
    textNode.nodeValue = text;
    return dummy.offsetWidth;
}

utils.splitText = function(font, text, maxWidth) 
{
    var fullText = text;
    if (! fullText)
        return [];
    
    if (! dummy)
        dummy = document.getElementById("dummy");
    dummy.style.font = font;
    var textNode = dummy.childNodes[0];
    textNode.nodeValue = fullText;

    var width = 0;
    maxWidth -= 4;
   
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

utils.extendJsonObject = function (a, b)
{
    for(var key in b)
        if(! a.hasOwnProperty(key))
            a[key] = b[key];
}

utils.isPercentSize = function(value)
{
    if (typeof value != "string")
        return false;

    return value.search(/[0-9]+\%/g) != -1;
}

utils.isNumber = function(n) {
  return !isNaN(parseFloat(n)) && isFinite(parseFloat(n));
}

utils.initElement = function (element, info)
{
    element.style.position = "absolute";
    if (typeof info.width == "string")
        element.style.width = info.width;
    else
        element.style.width = info.width + "px";
    
    if (typeof info.height == "string")
        element.style.height = info.height;
    else
        element.style.height = info.height + "px";
    element.style.display = "none";
}

utils.importFile = function(url, callback, async, mimeType)
{
    if (async == "undefined")
        async = false;
    
    var xobj = new XMLHttpRequest();
    if (xobj.overrideMimeType && mimeType)
        xobj.overrideMimeType(mimeType);
    
    xobj.onreadystatechange = function() {
        if(xobj.readyState == 4){
            if (callback)
                callback(xobj);
        }
    };
    
    xobj.open("GET", url, async);
    xobj.send(null);
}

utils.isFontLoaded = function(name)
{
    name = utils.getFontName(name);

    var defaultFont = "12px Arial, Helvetica, sans-serif";
    var font = "12px " + name + ",  Arial, Helvetica, sans-serif";
    var defaultWidth = utils.textWidth("ABCDEFGHIJKLMNOPQRSTUVXWYZ0123456789", defaultFont);
    var width = utils.textWidth("ABCDEFGHIJKLMNOPQRSTUVXWYZ0123456789", font);
    if ( width != defaultWidth)
        return true;
    return false;
}

utils.isFontAvailable = function(fontFile)
{
    var status = 404;
    utils.importFile(fontFile, function(obj) {status = obj.status;}, false, "application/octet-stream");
    if (status == 200)
        return true;
    return false;
}

utils.getFontName = function(font)
{
    if (font.indexOf(".") !== -1)
        return font.split(".")[0];
    if (font.indexOf("px") !== -1)
        return font.split("px")[1];
    if (font.indexOf("%") !== -1)
        return font.split("%")[1];
    
    return font;
}

}(belle.utils));

log("Utils module loaded!");

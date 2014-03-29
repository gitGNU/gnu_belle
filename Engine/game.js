/* Copyright (C) 2014 Carlos Pais
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
 
(function(game) {
    
  	game.isFinished = function() {
	  return this.finished;
	}
	
	game.indexOf = function(scene) {
	  if (scene && belle.isInstance(scene, belle.objects.Scene)) {
	    var scenes = this.getScenes() || [];
	    return scenes.indexOf(scene);
	  }
	  return -1;
	}

	game._setCurrentScene = function(scene) {
	  if (! scene)
	    return;
	  if (this.paused)
	    this.pauseScreen.scene = scene;
	  else
	    this.scene = scene;
	  scene.setActive(true);
	}
	
	game.goto = function(scene) {
	  this.setCurrentScene(scene);
	}
	
	game.setCurrentScene = function(scene) {
	    var scenes = this.getScenes();
	    if (typeof scene == "number") {
	      if (scene >= 0 && scene < scenes.length)
		this._setCurrentScene(scenes[scene]);
	    }
	    else if (scene instanceof String || typeof scene === 'string') {
		for (var i=0; i !== scenes.length; i++) {
		    if (scenes[i].name === scene) {
			this._setCurrentScene(scenes[i]);
			break;
		    }
		}
	    }
	    else if (belle.isInstance(scene, belle.objects.Scene) && this.indexOf(scene) !== -1){
		this._setCurrentScene(scene);
	    }
	}
	
	game.getScenes = function() {
		if (this.paused)
			return this.pauseScreen.scenes;
		return this.scenes;
	}
	
	game.getCurrentScene = function() {
	  return this.getScene();
	}
	
	game.getScene = function(scene) {
		if (this.paused)
			return this.pauseScreen.scene;
		return this.scene;  
	}
	
	game.nextScene = function() {
	  if (this.isFinished())
	    return null;
	  
	  var index = this.indexOf(this.getScene()) + 1;
	  var scenes = this.getScenes();
	  if (index >= 0 && index < scenes.length) {
	    var scene = scenes[index];
	    scene.setActive(true);
	    this.scene = scene;
	    return this.scene;
	  }
	  else {
	    this.finished = true;
	  }
	  
	  return null;
	}
	
	game.next = function() {
		var action = null;
		while (! action && this.scene) {
			action = this.scene.nextAction();
			if (this.scene.isFinished())
				this.scene = this.nextScene();
		}
			
		if (! action)
			this.finished = true;
		return action;
	}
	
	game.getResource = function(name) {
		if (name && name in game.resources)
			return game.resources[name];
		return null;
	}
	
	game.hasVariable = function(variable) {
		if (variable.startsWith("$"))
			variable = variable.slice(1);
		
		return (variable in game.variables);
	}

	game.getValue = function(variable) {
		if (! this.hasVariable(variable))
			return "";
			
		if (variable.startsWith("$"))
			variable = variable.slice(1);
		
		return this.variables[variable];
	}

	game.addVariable = function (variable, value) {
		game.variables[variable] = value;
	}
	
	game.pause = function() {
	  if (! this.pauseScreen.scenes.length) 
	    return;
	  var scene = game.pauseScreen.scenes[0];
	  this.pauseScreen.scene = scene;
	  scene.action = null;
	  scene.setActive(true);
	  this.paused = true;
	}
	
	game.resume = function() {
	  this.paused = false;
	}

	game.replaceVariables = function(text) {
		if (! text)
			return text;
		
		if (! text.contains("$"))
			return text;
		
		var validChar = /^[a-zA-Z]+[0-9]*$/g;
		var variable = "";
		var variables = [];
		var values = [];
		var appendToVariable = false;
		
		//Parse text to determine variables
		//Variables start from "$" until the end of string or
		//until any other character that is not a letter nor a digit.
		for(var i=0; i !== text.length; i++) {
		  
		  if (text.charAt(i).search(validChar) == -1) {
			appendToVariable = false;          
			if (variable)
			  variables.push(variable);
			variable = "";
			if(text.charAt(i) == "$")
			  appendToVariable = true;
		  }
			
		  if (appendToVariable)
			variable += text.charAt(i);
		}
		
		//replace variables with the respective values and append them to the values list
		for(var i=0; i != variables.length; i++) {
			if (this.containsVariable(variables[i]))
			  values.push(this.getValue(variables[i]));
			else //if the variable is not found, still add an empty value to the values so we have an equal number of elements in both lists
			  values.push("");
		}
		
		//replace variables with the values previously extracted
		for(var i=0; i != values.length; i++) {
		  text = text.replace(variables[i], values[i]);
		}
		
		return text;
	}
	
})(game);

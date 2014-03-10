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
    
  	game.isFinished = function(scene) {
	  return this.finished;
	}
	
	game.indexOf = function(scene) {
	  if (scene && belle.utils.isInstance(scene, belle.objects.Scene)) {
	    var scenes = this.getScenes() || [];
	    return scenes.indexOf(scene);
	  }
	  return -1;
	}

	game.getScenes = function() {
		if (this.paused)
			return this.game.pause.scenes;
		return this.game.scenes;
	}

	game.getScene = function(scene) {
		if (this.paused)
			return this.game.pause.scene;
		return this.game.scene;  
	}
	
	game.nextScene = function() {
	  if (this isFinished())
	    return null;
	  
	  var index = this.indexOf(this.getScene()) + 1;
	  var scenes = this.getScenes();
	  if (index >= 0 && index < scenes.length) {
	    this.scene = scenes[index+1];
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
		if (! containsVariable(variable))
			return "";
			
		if (variable.startsWith("$"))
			variable = variable.slice(1);
		
		return game.variables[variable];
	}

	game.addVariable = function (variable, value) {
		game.variables[variable] = value;
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
	
}(game);

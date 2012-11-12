/*
 * Copyright 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
var routingHelperJNext, 
    _event = require("../../lib/event"),	
	APNConnectMessage = require("./APNConnectMessage"),
	APNType = require("./APNType"),
	APNEventType = require("./APNEventType");
	_globalId = 0, //control message ID
	_connectId = 0, //id of the connection. 
	_apnType = "",
	_pps = qnx.webplatform.pps,
	_ppsObj = null,
	_ppsStatusObj = null,
	PPS_PATH_PREFIX = '/pps/services/cds/cnice/'
	PPS_CONTROL_PATH_SUFFIX = '_control'
	PPS_STATUS_SUFFIX = 'status'
	DEFAULT_CONNECT_ID = 0,
	CONNECT_STR =  'connect',
	DISCONNECT_STR = 'disconnect',
	ERROR ='error',
	_connectComplete = false;

/** Notification when PPS open failed. Could be permissions issues
*/
function openFailed(data) {			
	_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.PPS_ERROR);
};

/** Called when PPS write failed. 
*/
function writeFailed(data) {
	_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.PPS_ERROR);
};

/** Callback onNewData is mapped to this Function.
  * Response is received to the PPS control message to connect to a particular interface
  * A connect ID is sent with the reponse. This connect id should be used when a disconnect is called
  * The interface on which the device is connected to the APN will also be set as the default route for 
  * the device. 
  * @param data
  */
function onConnect(data){
	//in case of error send event and return
	if(data.hasOwnProperty(ERROR) && data.error){
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.CONNECT_FAILURE);
		_apnType = ""; //reset
		return;
	}
	//get the pps object name i.e carrier_apps_control and store the connect ID
	var apnObj = _apnType + PPS_CONTROL_PATH_SUFFIX; 
	_connectId = data[apnObj].dat.connect_id;	
	_ppsObj.close();	
	//get the current connected interface 
	var netIf = getInterface();			
	setRoutingInterface(netIf);	
}  

/** Callback when response is received to a PPS control message to disconnect from a particular interface
  * This will also reset the default route that was set in the connect. 
  * @param data
  */
function onDisconnect(data){	
	//get the pps object name i.e carrier_apps_control
	if(data.hasOwnProperty(ERROR) && data.error){
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.DISCONNECT_FAILURE);
		return;
	}
	var apnObj = _apnType + PPS_CONTROL_PATH_SUFFIX;
	var refRemaining = data[apnObj].dat.ref_remaining;
	//if for some other application created a connection that is active, the lower layer will
	//not bring down the connection
	_connectId = DEFAULT_CONNECT_ID;
	_ppsObj.close();
	_ppsObj = null;
	_apnType = "";
	if( refRemaining){
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.DISCONNECT_FAILURE);			
	}else{
		_event.trigger(APNEventType.DISCONNECT_EVENT, APNConnectMessage.DISCONNECTED);
	}
}

/** Callback when the status object /pps/services/cds/cnice/status is updated with a network interface
  */
function onStatusObjectChanged(data){
	var netIf = data[PPS_STATUS_SUFFIX][_apnType].net_if;
	if (!_connectComplete){
		setRoutingInterface(netIf);
	}
}

function setRoutingInterface(netIf){
	if(netIf && netIf != ""){ 
		var args = " "+ netIf; //add space required for JNEXT parsing
		//All sockets will be bound to this interface	
		var retVal = routingHelperJNext.setRoutingInterface(args);		
		_connectComplete = true;	
		//close the pps object
		if(retVal == 0){ //success
			_event.trigger(APNEventType.CONNECT_EVENT, APNConnectMessage.CONNECTED);							
		}else{
			//disconnect in case of error
			disconnect(_apnType);
			_apnType = "";
			_connectId = DEFAULT_CONNECT_ID; //should happen in onDisconnect but for safet_apnType =
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.ROUTING_IF_UPDATE_FAILURE);			
		}
	}
}

/** Unsets the current default routing if
  */
function resetRoutingInterface(){
	var retVal = routingHelperJNext.resetRoutingInterface();		
	if(retVal != 0){ //success
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.ROUTING_IF_UPDATE_FAILURE);	
		return false;		
	}
	return true;
}	

/** Get the current network interface associated with the connected APN 
  * This interface is set as the default interface for all sockets created
  */	
function getInterface(){
	var path = PPS_PATH_PREFIX + PPS_STATUS_SUFFIX;
	var _ppsStatusObj = _pps.createObject(path, _pps.PPSMode.FULL);
	var netIf = null;	
	if(_ppsStatusObj ){
		_ppsStatusObj.onOpenFailed = openFailed;
		if(_ppsStatusObj.open(_pps.FileMode.RDONLY)){
			var netIf = _ppsStatusObj.data[PPS_STATUS_SUFFIX][_apnType].net_if;
			if(netIf && netIf != ""){
				//if netIf is available then close the PPS object. 
				_ppsStatusObj.close(); 
				return netIf;
			}else{
				_ppsStatusObj.onNewData = onStatusObjectChanged;	
			}
		}
	}else{
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.PPS_ERROR);
	}
		
	return netIf;
}

/** Generates a unique ID used for sending the control message, in order
  * to get a response
  */
function generateId() {
    var id = _globalId++;
    if (!window.isFinite(id)) {
        globalId = 0;
        id = 0;
    }
    return id.toString();
}

/** Check if the APN type passed in is a valid expected type
  */
function isValidApn(apn){
	return (apn == APNType.CARRIER_ADMIN) ||
	(apn == APNType.CARRIER_APPS) ||
	(apn == APNType.CARRIER_800) ||
	(apn == APNType.PLAN_ADMIN);		
}

/** Connects to the specified APN
  * Calls connect on the control object and specifies callback functions 
  * @param apnType (APNType)
  */
function connect(apnType){
	//eg path for carrier_apps apnName /pps/services/cds/cnice/carrier_apps_control
	var ppsPath = PPS_PATH_PREFIX + _apnType + PPS_CONTROL_PATH_SUFFIX;
	_ppsObj = _pps.createObject(ppsPath, _pps.PPSMode.FULL);
	if(_ppsObj){
		//set up callbacks for PPS
		_ppsObj.onOpenFailed = openFailed;
		_ppsObj.onWriteFailed = writeFailed;
		_ppsObj.onNewData = onConnect;	
		
		if(_ppsObj.open(_pps.FileMode.RDWR)){
			//generate an ID for the control message 
			var reqId = generateId();
			var writeData = { msg: CONNECT_STR, id: reqId};
			_ppsObj.write(writeData);					
		}
	} else{
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.PPS_ERROR);
	}	
}

function disconnect(apnType){
	var ppsPath = PPS_PATH_PREFIX + apnType + PPS_CONTROL_PATH_SUFFIX;
	_ppsObj = _pps.createObject(ppsPath, _pps.PPSMode.FULL);
	if(_ppsObj){
		//set up callbacks for PPS
		_ppsObj.onOpenFailed = openFailed;
		_ppsObj.onWriteFailed = writeFailed;
		_ppsObj.onNewData = onDisconnect;
		if(_ppsObj.open(_pps.FileMode.RDWR)){			

			var reqId = generateId();
			var writeData = { msg: DISCONNECT_STR, id: reqId, dat:{connect_id: _connectId} };
			_ppsObj.write(writeData);		
		}
	}else{
		_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.PPS_ERROR);
	}		
}

module.exports = {
    bindToInterface: function (success, fail, args, env) {
		//if a connection already exists do not allow creation of a second one. 
		if(_connectId != DEFAULT_CONNECT_ID){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.CONNECTION_ALREADY_EXISTS);
			fail(-1, "Connection Exists");
			return;
		}
		//get the passed in APN type and validate
		var apn = JSON.parse(decodeURIComponent(args.apnName));
		
		if(!apn || apn === ""){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.UNKNOWN_CONNECTION_TYPE);
			fail(-1, "Invalid Type");
			return;
		}
		
		if(! isValidApn(apn)){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.UNKNOWN_CONNECTION_TYPE);
			fail(-1, "Invalid Type");
			return;
		}
		_connectComplete = false;
		_apnType = apn;
		connect(_apnType);
		success();
    },

    disconnectFromInterface: function (success, fail, args, env) {
		//If there is no connection nothing to disconnect
		if(_connectId == DEFAULT_CONNECT_ID){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.NOT_CONNECTED);
			fail(-1, "Connection Exists");
			return;
		}
        var apn = JSON.parse(decodeURIComponent(args.apnName));
		//if the APN type passed in is invalid return
		if (!apn || apn === ""){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.UNKNOWN_CONNECTION_TYPE);
			fail(-1, "Invalid Type");
			return;	
		}
		
		if(!isValidApn(apn)){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.UNKNOWN_CONNECTION_TYPE);
			fail(-1, "Invalid Type");
			return;
		}
		//check if the apn type being disconnected is the same as the connection
		if(apn != _apnType){
			_event.trigger(APNEventType.CONNECT_ERROR, APNConnectMessage.NOT_CONNECTED);
			fail(-1, "Not Connected");
			return;
		}	
		//if default routing if could not be updated notify but don't diconnect.
		if(resetRoutingInterface()){
			disconnect(apn);
		}
		success();
    }
};

///////////////////////////////////////////////////////////////////
// JavaScript wrapper for JNEXT plugin
///////////////////////////////////////////////////////////////////

JNEXT.RoutingHelper = function ()
{   
    var _self = this;

    _self.setRoutingInterface = function (args) {
        return JNEXT.invoke(_self._id, "setRoutingInterface" + args);
    };
	
	_self.resetRoutingInterface = function () {
        return JNEXT.invoke(_self._id, "resetRoutingInterface");
    };
	
	_self.getRoutingInterface = function () {
        return JNEXT.invoke(_self._id, "getRoutingInterface");
    };

    _self.getId = function () {
        return _self._id;
    };

    _self.init = function () {
        if (!JNEXT.require("myVzWext")) {
            return false;
        }

        _self._id = JNEXT.createObject("myVzWext.RoutingHelper");

        if (!_self._id || _self._id === "") {
            return false;
        }

        JNEXT.registerEvents(_self);
    };

    _self.onEvent = function (strData) {
    };
    
    _self._id = "";
    
    _self.init();
};

APNType = APNType;
APNConnectMessage = APNConnectMessage;
APNEventType = APNEventType;
routingHelperJNext = new JNEXT.RoutingHelper();

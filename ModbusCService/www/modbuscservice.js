var clientMQTT1;
var polaczMQTT1;
var clientMQTT2;
var polaczMQTT2;
var lastConnState = 0;

const messageSubscribeIn  = 'fvz_in/#';
const messageSubscribeOut = 'fvz_out/#';
const messageSubscribeTemp = 'fvz/temp';


// ********** Time calculation. 
function getDateTime() {
	let dt = new Date();
	let y  = dt.getFullYear();
	let m  = "0" + parseInt(dt.getMonth()+1);
	let d  = "0" + dt.getDate();
	let hr = "0"+ dt.getHours();
	let mm = "0" + dt.getMinutes();
	let s  = "0" + dt.getSeconds();
	let ms = "00" + dt.getMilliseconds();
	return y + "-" + m.substr(-2) +"-" + d.substr(-2) + " " + hr.substr(-2) + ':' + mm.substr(-2) + ':' + s.substr(-2) +"."+ ms.substr(-3);  
}


// ********** Cyclic checking MQTT dconnection for server calling by 5 sek.
function ChkMQTTConneton1(){
    if (!clientMQTT1.isConnected())
        polaczMQTT1();
}


// ********** Dom conntent loaded event (on start).
document.addEventListener("DOMContentLoaded", function(event) {
		
    // ********** MQTT for modbusCService **********
	let host="your.mqtt.server.com";
    if (window.location.protocol == "https:")
		clientMQTT1 = new Paho.MQTT.Client(host, Number(9002), Math.random().toString(36).substring(2));
    else
		clientMQTT1 = new Paho.MQTT.Client(host, Number(9001), Math.random().toString(36).substring(2));

    clientMQTT1.onConnectionLost = onConnectionLost1;
    clientMQTT1.onMessageArrived = onMessageArrived1;
 
	polaczMQTT1 = function() {

        clientMQTT1.connect({
            onSuccess: (function() {
                console.log(getDateTime()+" MQTT connected");
				document.getElementById("overlay").style.display = "none";
				let teraz = new Date().getTime();
				if (lastConnState>0 && teraz-lastConnState>10000){
					lastConnState = 0;
					location.reload();
					return;
				}
				clientMQTT1.subscribe(messageSubscribeIn);
				clientMQTT1.subscribe(messageSubscribeOut);
	            }),
                useSSL: window.location.protocol == "https:",
                userName : 'YourLoginToMQTTserver',
                password : 'YourPasswdToMQTTserver',
        });
    }


	// ********** Lost connestion event (internal function).
    function onConnectionLost1(responseObject) {
      if (responseObject.errorCode !== 0){
		  lastConnState = new Date().getTime();
          console.log(getDateTime()+" MQTT connection lost: "+responseObject.errorMessage);
		  document.getElementById("overlay").style.display = "block";
          document.getElementById("textOverlay").innerHTML = "Connection to MQTT server is lost!";
		  setTimeout(() => {  //Reload site
					location.reload();
					}, 20000);
      }
    } 

    ChkMQTTConneton1();
    window.setInterval(ChkMQTTConneton1, 5000);

});






// ********** One message arrived from MQTT server.
function onMessageArrived1(message) {
	//console.log(getDateTime()+" onMessageArrived1:"+" destinationName="+message.destinationName+" payload="+message.payloadString);
	
	if (message.destinationName=="fvz_in/V2to3"){
		let jsonVal = JSON.parse(message.payloadString);
		if (jsonVal)
			refreschLeds("ledA", jsonVal);
	}

	if (message.destinationName=="fvz_out/V0to1"){
		let jsonVal = JSON.parse(message.payloadString);
		if (jsonVal)
			refreschSwitchess("switchA", jsonVal);
	}
}


// ********* Refresh leds light state.
function refreschLeds(prefix, jsonVal){
	for (i in jsonVal){
		let ledObj = document.getElementById(prefix+i);
		if (ledObj)
			setLedColor(ledObj, jsonVal[i]);
	}
}


// ********* Refresh Switches state.
function refreschSwitchess(prefix, jsonVal){
	for (i in jsonVal){
		let swObj = document.getElementById(prefix+i);
		if (swObj)
			swObj.checked = jsonVal[i];
	}	
}

// ********* Set state of single led light.
function setLedColor(ledObj, state){
	let c = ledObj.getAttribute("color");
	ledObj.className="";
	ledObj.className = (state==1) ? ("led-"+c) : "led";
	//console.log(ledObj.className);
}

// ********* Event on click switch.
function chkBoxClicked(obj){
	//console.log(obj.id + "  " + obj.checked);

	let arr = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
	for (let i = 0; i < arr.length; i++) {
		let swObj = document.getElementById("switchA"+i);
		if (swObj)
			if (swObj.checked==true)
				arr[i] = 1;
	}
	arr = "["+arr.toString()+"]";
	//console.log(arr);

	// Publish a Message
	var message = new Paho.MQTT.Message(arr);
	message.destinationName = "fvz_out/V0to1";
	message.qos = 0;
	message.retained = true;
	clientMQTT1.send(message);
}

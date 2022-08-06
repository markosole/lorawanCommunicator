require('dotenv').config()
const app = require("express")();
var express = require('express');
const http = require("http").createServer(app);
var bodyParser = require('body-parser');
var SerialPort = require("serialport").SerialPort;
const ReadlineParser = require('@serialport/parser-readline').ReadlineParser;
const socketio = require("socket.io")(http);


var com_port = process.env.SERIAL_COM_PORT;
var server_port = process.env.SERVER_PORT;
var use_web_interface = true;                   // Use internal web interface 
var lora_clients = [];                          // All LoraWan Nodes
var acceptableVoltage = 4.0;                    // Define normal acceptable battery voltage
var alarmingVoltage = 3.7;                      // Define Min acceptable (warning) battery voltage
const printConsole = false;                     // Print object details in console
const printBatteryStatus = true;                // Print battery status in console

//    ___   ___           ___           _     _               _             
//   / __\ /___\/\/\     / _ \___  _ __| |_  | |__  _ __ ___ | | _____ _ __ 
//  / /   //  //    \   / /_)/ _ \| '__| __| | '_ \| '__/ _ \| |/ / _ \ '__|
// / /___/ \_// /\/\ \ / ___/ (_) | |  | |_  | |_) | | | (_) |   <  __/ |   
// \____/\___/\/    \/ \/    \___/|_|   \__| |_.__/|_|  \___/|_|\_\___|_| 


// Express Middleware
app.use(express.static('public'));
app.use(bodyParser.urlencoded({
    extended: true
}));

// Render Main Web view
app.get('/', function (req, res) {
    if(use_web_interface){
        res.sendFile('views/index.html', {
            root: __dirname
        });
    }else {
        res.send("Node Broker running..."); 
    }
});

// Sockets actions
socketio.on("connection", (client) => {  
    console.log("Web Client connected, with socket ID:" + client.id);
    socketio.emit("lora-clients", lora_clients);        // Emit clients list browser when ever Web app is connected
});

openSerial();

async function openSerial() {    
    var serialPort = new SerialPort({ path: com_port, baudRate: 115200, bufferSize: 10000, autoOpen: false });
    const parser = serialPort.pipe(new ReadlineParser({ delimiter: '\r\n' }));
    try {
        serialPort.open(function (error) {
            if ( error ) {
                console.log('failed to open serial: ' + error);
            } else {
                try{
                    // Join multiline buffered string into one line (max 255 chars long)
                    parser.on('data', function(e){                        
                        addLoraClients(e);                        
                    });
                } catch (err) {
                   console.log("Parsing failed! String contains unexpected characters.")
                }                
            }
        })
    } catch (e) {
        console.log("Serial Error ", e.toString());
    }
}

function addLoraClients(client){
    // Use try as passed in client may not be valid JSON string of data
    try {    
        var parsed = JSON.parse(client);
        Array.prototype.inArray = function(comparer) {
            for(var i=0; i < this.length; i++) {
                if(comparer(this[i])) return true;
            }
            return false;
        };


        // adds an element to the array if it does not already exist using a comparer
        Array.prototype.pushIfNotExist = function(element, comparer) {
            if (!this.inArray(comparer)) {
                // Find element index for matching element
                lora_client_index = lora_clients.findIndex((obj => obj.system.Name == element.system.Name));
                if(lora_client_index >= 0){
                    // Update with latest location - map item in array by identifier and update price value
                    const newState = lora_clients.map(obj =>
                        obj.system.Name === element.system.Name ? { ...obj, location: parsed.location, gpsDateTime: parsed.gpsDateTime, system: parsed.system, satellites: parsed.satellites, altitude: parsed.altitude} : obj
                    );
                    // apply new updated array
                    lora_clients = newState;

                } else {
                    this.push(element);
                }                
            }
        };

        // Prepare new element data to push in array
        var element = { 
            appid: parsed.appid,
            system: parsed.system,
            location: parsed.location,
            satellites: parsed.satellites,
            altitude: parsed.altitude,
            gpsDateTime: parsed.gpsDateTime    
        };

        lora_clients.pushIfNotExist(element, function(e) {
            return e.appid === element.appid && e.system === element.system && e.location === element.location && e.satellites === element.satellites && e.altitude === element.altitude && e.gpsDateTime === element.gpsDateTime;
        });


        // Send battery stats for each LoraWan node 
        if(printBatteryStatus){
            var batt_obj = {
                name: parsed.system.Name,
                battery: parsed.system.Battery
            };
            batteryStatusCheck(batt_obj);
        }
        
        if(printConsole){
            console.log('\x1b[33m%s\x1b[0m',"LoraWan clients list lenght: " + lora_clients.length);
            console.log('\x1b[46m%s\x1b[0m',"## Latest updated data: ##");
            console.log(lora_clients);
        }        

        // Send all client's data to Socket web client(s)
        socketio.emit("lora-clients", lora_clients);

    } catch (error) {
      console.log("Error in parsing object, err: " + error);
      console.log(client);
    }
}

function batteryStatusCheck(loraNode){    
    var voltage = parseFloat(loraNode.battery.Voltage);

    // Warning voltage
    if(voltage < acceptableVoltage && voltage > alarmingVoltage){
        console.log('\x1b[33m%s\x1b[0m',"LoraWan node: [" + loraNode.name + "] voltage dropped below " + acceptableVoltage + "V and it\'s now " + voltage + "V");
    }
    // Alarming voltage
    if(voltage <= alarmingVoltage){
        console.log("\x1b[31m%s\x1b[0m","LoraWan node: [" + loraNode.name + "] voltage dropped on or below alarming " + acceptableVoltage + "V and it\'s now " + voltage + "V"); 
    }
}


// Run Server
app.set("port", server_port);
http.listen(app.get('port'), () => console.log('Broker is served on port: ' + app.get('port')));

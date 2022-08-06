# lorawanCommunicator - LoraWan GPS positioning

Well known TTGO Lora32 development boards are used as Receiver and Sender (clients). NodeJS is used to open COM port (configured in .env file) and accept data sent by Clients using LoraWan network.
There is no LoraWan gateway involved. 

This project is based on idea (with it's parts of the code) from Youtuber "ThatProject" and for more info about similar project check out https://www.youtube.com/watch?v=zJvDw4UVDLc&ab_channel=ThatProject

Websocket Socket.io is used for communication with Web interface and send (plot) data on Google Map. Map plotting ins't completed yet and it's on ToDo list.

# Configure

 - Change COM port in ` .env ` file to match device attached
 - Flash Client and Received .ino using Arduino IDE to your devices (1 x Receiver and multiple clients)
 - For more console logging set ` printConsole = true `   in server.js on line 17.

# Install & Run

`npm install`

`nodemon server.js`

# ToDo

- Show client's cooordinates on Google (or others) map. 
- Optimize battery usage (help is welcome) 
- Enable sleep mode and wakeup to send data



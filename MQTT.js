var mqtt = require('mqtt');//我們需要用到mqtt模組
var opt = {//設置埠號和庫戶端的ID
    port:1883,
    clientId: 'nodejs'

};
//從此行到第19行是為了連到MQTT Broker
var express = require("express");//我們需要用到express
var app =express();
app.use(express.static('www'));
var server = app.listen(5438);
var sio = require("socket.io")(server);
var client =mqtt.connect('tcp://120.107.168.127',opt);//建立用戶端並連到broker
client.on('connect',function(){
    console.log('以連接');//方便看到底有沒有連到
    client.subscribe('Temperature');//訂閱topic
    client.subscribe('Humidity');//訂閱topic
    client.subscribe('AQI');//訂閱topic
});
//利用socket.io去收資料和傳資料
sio.on('connection',function(socket){
    client.on('message',function(topic,msg){  
        if(topic=='Temperature') {
            socket.emit('mqtt','\{' + '\"temp\":'+msg.toString() +'\}');
            //           事件名稱          字串(我們將在html檔撰寫一串程式將字串轉成javascript object)
            //將資料以字串的形式傳出去

        }
        if(topic=='Humidity') {
            socket.emit('mqtt','\{' + '\"humid\":'+msg.toString() +'\}'); 
        }
        if(topic=='AQI') {
            socket.emit('mqtt','\{' + '\"AQI\":'+msg.toString() +'\}'); 
        }
    });
});

var express = require('express');
var Slack = require('./slack.seed');
//var app = express();
var path = require('path');
var mqtt = require('mqtt');
var socketIO = require('socket.io');
var mongo = require('mongodb');
var url = "mongodb://abc:abc@ds115436.mlab.com:15436/finger_id";
var newUser;
var n_data = 0;

mongo.connect(url, function(err, db) {
  if (err) throw err;
  db.collection("FingerID").count({}, function(err, result) {
    if (err) throw err;
    console.log(result);
    n_data = parseInt(result);
    db.close();
  });
});


//app.set('port', (process.env.PORT || 5000));

//app.use(express.static(path.join(__dirname, 'public')));


// views is directory for all template files
//app.set('views', path.join(__dirname, 'views'));
//app.set('view engine', 'ejs');
/*
app.get('/', function(request, response) {
  response.render('pages/index');
});

app.listen(app.get('port'), function() {
  console.log('Node app is running on port', app.get('port'));
});
*/
//const INDEX = path.join(__dirname, 'pages/index');


const PORT = process.env.PORT || 3000;
const INDEX = path.join(__dirname, 'index.html');

const server = express()
.use(express.static(path.join(__dirname, 'public')))
.set('views', path.join(__dirname, 'views'))
.set('view engine', 'ejs')
.use((req, res) => res.render('pages/index'))
.listen(PORT, () => console.log(`Listening on ${ PORT }`));

const io = socketIO(server);

io.on('connection', (socket) => {
  console.log('Client connected');
  socket.on('disconnect', () => console.log('Client disconnected'));
  socket.on('register', function(message){
    newUser = message;
    console.log(newUser);
    n_data++;
    client.publish('/ESP8266/Register','GetID#'+n_data.toString());
  });
});

//setInterval(() => io.emit('complete', new Date().toTimeString()), 1000);



webhookUri = "https://hooks.slack.com/services/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";

slack = new Slack();
slack.setWebhook(webhookUri);

var client  = mqtt.connect({
  host: 'iot.eclipse.org',
  port: 1883,
  keepalive: 10,
  connectTimeout: 30*1000
});

client.on('connect', function () {
  client.subscribe('/Server/Register');
  client.subscribe('/Server/toSlack');
  client.publish('chrome_test', 'Hello mqtt');
  console.log("Someone connected!!!");
  slack.webhook({
    channel: "#sensor",
    username: "Server",
    text: "Server is ready !!!"
  }, function(err, response) {
    console.log(response);
  });
});

client.on('message', function (topic, message) {
  // message is Buffer
  //console.log(message.toString());
  switch (topic){
    case '/Server/Register': 
      console.log("Register with ID = "+message.toString());
      mongo.connect(url, function(err,db) {
        if (err) throw err;
        var obj = {
          name: newUser.name,
          slack_user: newUser.slack_user,
          position: newUser.position,
          fingerid: n_data.toString()
        };
        console.log(obj.toString());
        db.collection("FingerID").insertOne(obj, function(err, res){
          if (err) throw err;
          console.log('1 object inserted !!!');
          db.close();
        });
      });
      io.emit('complete', message.toString());

      break;
    case '/Server/toSlack':
      console.log("Send to Slack: "+message.toString());
      mongo.connect(url, function(err, db) {
        if (err) throw err;
        db.collection("FingerID").findOne({"fingerid": message.toString()}, function(err, result) {
          if (err) throw err;
          console.log(result.name);
          slack.webhook({
            channel: "#sensor",
            username: "ESP8266",
            text: new Date().toString() + " : "+ result.name+" is coming!!!",
          }, function(err, response) {
            //console.log(response);
          });
          db.close();
        });
      });
      break;
  }
  
  //client.end();
});


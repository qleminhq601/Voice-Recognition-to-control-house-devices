const mqtt = require('mqtt');

const brokerUrl = 'mqtt://broker.hivemq.com';
const client = mqtt.connect(brokerUrl);

client.on('connect', () => {
  console.log('Connected to MQTT broker');

  const topic = 'myhome/temperature';
  client.subscribe(topic, (err) => {
    if (!err) {
      console.log(`Subscribed to topic: ${topic}`);
    } else {
      console.error(`Failed to subscribe: ${err.message}`);
    }
  });
});

client.on('message', (topic, message) => {
  console.log(`Message received on topic "${topic}": ${message.toString()}`);
});

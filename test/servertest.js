const dgram = require('dgram');
const socket = dgram.createSocket('udp4');

const ADDRESS = '192.168.0.213';
const PORT = 555;

socket.on('message', (data, info) => {
    console.log(`Got ${JSON.stringify(data)} from ${info.address} at port ${info.port}`);
});

/* Ping */
// socket.send(Buffer.from([0x00]), 0, 1, PORT, ADDRESS);

/* Get color */
// socket.send(Buffer.from([0x10]), 0, 1, PORT, ADDRESS);

/* Set color */
// socket.send(Buffer.from([0x20, 0xff, 0x00, 0x00]), 0, 4, PORT, ADDRESS);

/* Set transition */
// socket.send(Buffer.from([0x30, 0x00, 0xff, 0x00, 5]), 0, 5, PORT, ADDRESS);

/* Burst */
// var i = 0;
// var m = 3;
// const x = setInterval(() => {
//     socket.send(Buffer.from([0x40, 0x00, 0x00, i]), 0, 4, PORT, ADDRESS);
//     i += m;
//     if (i >= 0xff || i <= 0x00) {
//         m *= -1;
//         (i >= 0xff) ? (i = 0xff) : (i = 0x00);
//     }
//     console.log(i);
// }, 20);
// setTimeout(() => {
//     clearInterval(x);
//     socket.close();
// }, 15000);
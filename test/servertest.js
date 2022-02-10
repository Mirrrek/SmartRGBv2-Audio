const dgram = require('dgram');
const socket = dgram.createSocket('udp4');

socket.bind(666);

socket.on('message', (data, info) => {
    console.log(`Got ${JSON.stringify(data)} from ${info.address} at port ${info.port}`);
});

/* Send color change */

// socket.send(Buffer.from([0x20, 200, 0, 255, 1]), 0, 5, 555, '192.168.0.213');


/* Burst colors */

// var i = 0;
// var m = 3;
// const x = setInterval(() => {
//     socket.send(Buffer.from([0x30, i, 0, 0]), 0, 4, 555, '192.168.0.213');
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
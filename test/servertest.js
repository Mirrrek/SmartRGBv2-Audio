const dgram = require('dgram');
const socket = dgram.createSocket('udp4');

socket.bind(666);

socket.on('message', (data, info) => {
    console.log(`Got ${JSON.stringify(data)} from ${info.address} at port ${info.port}`);
});

/* Get color */

// socket.send(Buffer.from([0x10]), 0, 1, 555, 'IP_ADDRESS');

/* Send transition */

// socket.send(Buffer.from([0x30, 0, 0, 0, 5]), 0, 5, 555, 'IP_ADDRESS');


/* Burst colors */

// var i = 0;
// var m = 3;
// const x = setInterval(() => {
//     socket.send(Buffer.from([0x40, i, 0, 0]), 0, 4, 555, 'IP_ADDRESS');
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

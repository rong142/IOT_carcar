// import library
const express = require('express')
const ServerSocket = require('ws').Server   // 引用 Server

const WebSocket = require('ws');

// 指定一個 port
const PORT = 9090

// 建立 express 物件並用來監聽 9090 port
const server = express()
    .listen(PORT, () => console.log(`[Server] Listening on https://localhost:${PORT}`))

// 建立實體，透過 ServerSocket 開啟 WebSocket 的服務
const wss = new ServerSocket({ server })

// Connection opened
wss.on('connection', ws => {
    console.log('[Client connected]')

    // Connection closed
    ws.on('close', () => {
        console.log('Close connected')
    })

    // Listen for messages from client
    ws.on('message', data => {
        let today = new Date();

        if (data instanceof Buffer) {
            // console.log(today, '[Message from client Buffer]: ', data)
            try {
                const Data = data.toString('base64');
                console.log(today, '[Message from client event]: ', 'image')
                wss.clients.forEach(client => {
                    if (ws !== client && client.readyState === WebSocket.OPEN) {
                        client.send(Data);
                    }
                });
            } catch (error) {
                console.log('[Error]: ', error);
            }
        }


        // console.log(today, '[Message from client]: ', data.toString())
        /*
        // Send message to client
        // ws.send('[Get message from server]')
        let clients = wss.clients
        clients.forEach(client => {
            client.send(data.toString());
        });
        */
    })

    ws.on('atime', function (data) {
        let today = new Date();
        console.log(today, '[Message from client a]: ', data)
    })
})
const Websocket = require('ws')
const screenSync = require('../node-addon-screen-sync')

const wsserver = new Websocket.Server({
  host: '0.0.0.0',
  port: 4000
})

const { width, height } = screenSync.init()
let timer, clients = new Set(), capturing = false
wsserver.on('connection', (ws) => {
  ws.on('close', () => {
    clients.delete(ws)
    if (clients.size <= 0) {
      stopCapture()
    }
  })
  clients.add(ws)
  startCapture()
})

function startCapture() {
  if (capturing) return
  capturing = true

  timer = setInterval(function loop() {
    const encodedbgr = screenSync.record()
    for (let client of clients) {
      client.send(encodedbgr, (err) => {
        console.log(err)
      })    
    }
  }, 1000 / 25)
}
function stopCapture() {
  clearInterval(timer)
  capturing = false
}
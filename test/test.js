const screenSync = require('../build/Release/screen_sync.node')
const { performance } = require('perf_hooks')

screenSync.init()
const st = performance.now()

const encodedBuf = screenSync.record()
// const res = screenSync.extract(new Uint8Array(encodedBuf))
// console.log(res)
console.log(performance.now() - st)
// console.log(screenSync.extract())
const screenSync = require('./build/Release/screen_sync.node')

module.exports = {
  record() {
    return screenSync.record()
  },
  init(options) {
    return screenSync.init(options)
  },
  release() {
    return screenSync.release()
  },
  extract(uint8Array) {
    return screenSync.extract(uint8Array)
  }
}
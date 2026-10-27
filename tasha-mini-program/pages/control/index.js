const app = getApp()

Page({
  data: {
    isPad: false,
    displayMode: 'phone',
    host: '', connected: false, connecting: false, statusLevel: 'offline', lastUpdate: '--:--:--',
    telemetry: {}, logList: [], speed: 60, activeDir: ''
  },

  onLoad() {
    this.unsubscribe = app.subscribe((state) => {
      this.setData({
        isPad: state.isPad,
        displayMode: state.displayMode,
        host: state.host,
        connected: state.connected,
        connecting: state.connecting,
        statusLevel: state.statusLevel,
        lastUpdate: state.lastUpdate,
        telemetry: state.telemetry,
        logList: state.logList,
        speed: state.speed,
        activeDir: state.activeDir
      })
    })
  },
  onUnload() { if (this.unsubscribe) this.unsubscribe() },
  onHostInput(e) { app.setHost(e.detail.value) },
  connect() { app.connect() },
  disconnect() { app.disconnect() },
  onSpeedChange(e) { app.setSpeed(Number(e.detail.value)) },
  pressMove(e) {
    const dir = e.currentTarget.dataset.dir
    app.setActiveDir(dir)
    app.sendCmd('move', { dir, speed: this.data.speed })
  },
  releaseMove() {
    if (!this.data.activeDir) return
    app.setActiveDir('')
    app.sendCmd('stop')
  },
  tapStop() {
    app.setActiveDir('')
    app.sendCmd('stop')
    app.addLog('已发送停车指令')
  },
  tapStartAuto() {
    wx.showModal({
      title: '启动自动种植',
      content: '请确认车体周围安全、急停可用、模式允许自动运行。',
      confirmText: '启动',
      success: (res) => {
        if (res.confirm) {
          app.sendCmd('start_auto')
          app.addLog('已发送自动种植启动指令')
        }
      }
    })
  },
  tapPause() { app.sendCmd('pause'); app.addLog('已发送暂停指令') },
  tapResume() { app.sendCmd('resume'); app.addLog('已发送继续指令') },
  tapResetCount() { app.sendCmd('reset_count'); app.addLog('已发送苗数清零指令') },
  tapEstop() {
    wx.showModal({
      title: '确认急停',
      content: '急停会立即关闭小车、钻孔、送苗、覆土、水泵等执行机构。',
      confirmText: '急停',
      confirmColor: '#e94444',
      success: (res) => {
        if (res.confirm) {
          app.sendCmd('estop')
          app.addLog('已发送急停指令')
        }
      }
    })
  }
})
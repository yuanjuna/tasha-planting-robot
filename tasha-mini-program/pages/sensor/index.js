const app = getApp()

Page({
  data: {
    isPad: false,
    displayMode: 'phone',
    host: '', connected: false, connecting: false, statusLevel: 'offline', lastUpdate: '--:--:--',
    telemetry: {}
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
        telemetry: state.telemetry
      })
    })
  },
  onUnload() { if (this.unsubscribe) this.unsubscribe() },
  onHostInput(e) { app.setHost(e.detail.value) },
  connect() { app.connect() },
  disconnect() { app.disconnect() }
})
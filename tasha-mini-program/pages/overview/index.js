const app = getApp()

Page({
  data: {
    isPad: false,
    displayMode: 'phone',
    host: '',
    connected: false,
    connecting: false,
    statusLevel: 'offline',
    lastUpdate: '--:--:--',
    telemetry: {},
    stageSteps: [],
    logList: [],
    capabilityList: [
      { title: '自动种植闭环', desc: '钻孔、送苗、落苗、覆土、浇水五状态实时联动', tag: '核心流程' },
      { title: '远程实时接管', desc: '通过平板直连树莓派，支持手动接管与急停', tag: '人工干预' },
      { title: '多源感知决策', desc: '超声波、IMU、GPS、雷达等数据同屏可视化', tag: '感知融合' }
    ]
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
        stageSteps: state.stageSteps,
        logList: state.logList
      })
    })
  },

  onUnload() {
    if (this.unsubscribe) this.unsubscribe()
  },

  onHostInput(e) {
    app.setHost(e.detail.value)
  },

  connect() { app.connect() },
  disconnect() { app.disconnect() },

  gotoControl() { wx.switchTab({ url: '/pages/control/index' }) },
  gotoSensor() { wx.switchTab({ url: '/pages/sensor/index' }) }
})
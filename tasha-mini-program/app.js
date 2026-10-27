const STAGE_MAP = {
  idle: { text: '待机', index: -1, desc: '等待连接或等待启动' },
  drill: { text: '钻孔', index: 0, desc: '移动电钻下降 / 电钻工作' },
  seed: { text: '送苗', index: 1, desc: '步进电机带动送苗盘转动' },
  forward: { text: '前进落苗', index: 2, desc: '车体前进，使苗落入种植孔' },
  cover: { text: '覆土', index: 3, desc: '覆土机构伸出 / 回收' },
  water: { text: '浇水', index: 4, desc: '水泵开启，浇水完成后计数+1' },
  row_change: { text: '换行', index: 5, desc: '蛇形路线换行调整' },
  complete: { text: '完成', index: 6, desc: '本轮自动种植结束' },
  error: { text: '异常', index: 7, desc: '设备异常或急停' }
}

function pad2(n) {
  return n < 10 ? '0' + n : '' + n
}

function nowText() {
  const d = new Date()
  return `${pad2(d.getHours())}:${pad2(d.getMinutes())}:${pad2(d.getSeconds())}`
}

function clampNumber(v, def = 0) {
  const n = Number(v)
  return Number.isFinite(n) ? n : def
}

function defaultTelemetry() {
  return {
    online: false,
    mode: 'manual',
    stage: 'idle',
    stageText: '待机',
    stageDesc: '等待连接或等待启动',
    plant_count: 0,
    total_count: 12,
    row: 1,
    point: 1,
    progress: 0,
    row_end: false,
    ultrasonic_cm: 0,
    imu: { roll: 0, pitch: 0, yaw: 0, valid: false },
    gps: { lat: 0, lng: 0, speed: 0, fix: false, sats: 0 },
    radar: { left: 0, front: 0, right: 0, leftPercent: 0, frontPercent: 0, rightPercent: 0 },
    battery: { voltage: 24.0, percent: 80 },
    signal: 0,
    latency: 32,
    cpu_temp: 43.6,
    comm_ok: true,
    wheel_speed: { left: 0, right: 0 },
    mission_eta: '--'
  }
}


function getDisplayInfo() {
  let info = { isPad: false, windowWidth: 375, windowHeight: 667, displayMode: 'phone' }
  try {
    const sys = wx.getSystemInfoSync()
    const windowWidth = sys.windowWidth || 375
    const windowHeight = sys.windowHeight || 667
    const isPad = windowWidth >= 700 || (windowWidth >= 600 && windowHeight >= 760)
    info = {
      isPad,
      windowWidth,
      windowHeight,
      displayMode: isPad ? 'pad' : 'phone'
    }
  } catch (e) {}
  return info
}

App({
  onLaunch() {
    const savedHost = wx.getStorageSync('tasha_host') || this.globalData.defaultHost
    const displayInfo = getDisplayInfo()
    this.globalData.host = savedHost
    this.globalData = { ...this.globalData, ...displayInfo }
    if (wx.onWindowResize) {
      wx.onWindowResize((res) => {
        const width = res && res.size ? res.size.windowWidth : this.globalData.windowWidth
        const height = res && res.size ? res.size.windowHeight : this.globalData.windowHeight
        const isPad = width >= 700 || (width >= 600 && height >= 760)
        this.setPartial({
          isPad,
          windowWidth: width,
          windowHeight: height,
          displayMode: isPad ? 'pad' : 'phone'
        })
      })
    }
    this.addLog(displayInfo.isPad ? '系统就绪：已启用平板展示布局' : '系统就绪：已启用手机兼容布局')
  },

  globalData: {
    defaultHost: '192.168.4.1:8765',
    host: '',
    wsUrl: '',
    connected: false,
    connecting: false,
    statusLevel: 'offline',
    lastUpdate: '--:--:--',
    speed: 60,
    telemetry: defaultTelemetry(),
    stageSteps: [
      { key: 'drill', text: '钻孔' },
      { key: 'seed', text: '送苗' },
      { key: 'forward', text: '前进' },
      { key: 'cover', text: '覆土' },
      { key: 'water', text: '浇水' }
    ],
    logList: [],
    activeDir: ''
  },

  subscribers: [],
  wsTask: null,
  heartbeatTimer: null,
  reconnectTimer: null,
  reconnectingByError: false,

  subscribe(fn) {
    if (typeof fn !== 'function') return () => {}
    this.subscribers.push(fn)
    fn(this.getSnapshot())
    return () => {
      this.subscribers = this.subscribers.filter(item => item !== fn)
    }
  },

  notify() {
    const snapshot = this.getSnapshot()
    this.subscribers.forEach(fn => {
      try { fn(snapshot) } catch (e) {}
    })
  },

  getSnapshot() {
    return JSON.parse(JSON.stringify(this.globalData))
  },

  setPartial(patch = {}) {
    this.globalData = { ...this.globalData, ...patch }
    this.notify()
  },

  setHost(host) {
    const clean = (host || '').trim().replace(/^https?:\/\//, '').replace(/^wss?:\/\//, '')
    this.globalData.host = clean
    wx.setStorageSync('tasha_host', clean)
    this.notify()
  },

  setSpeed(speed) {
    this.globalData.speed = Number(speed) || 60
    this.notify()
  },

  setActiveDir(activeDir) {
    this.globalData.activeDir = activeDir || ''
    this.notify()
  },

  connect() {
    const host = (this.globalData.host || '').replace(/^https?:\/\//, '').replace(/^wss?:\/\//, '')
    if (!host) {
      wx.showToast({ title: '请先输入 IP', icon: 'none' })
      return false
    }

    this.setHost(host)
    this.closeSocket(false)

    const wsUrl = `ws://${host}/ws`
    this.globalData.wsUrl = wsUrl
    this.setPartial({ connecting: true })
    this.addLog(`正在连接 ${wsUrl}`)

    this.wsTask = wx.connectSocket({ url: wsUrl })

    this.wsTask.onOpen(() => {
      this.reconnectingByError = false
      this.setPartial({ connected: true, connecting: false, statusLevel: 'online' })
      this.addLog('连接成功，开始接收实时数据')
      this.startHeartbeat()
      this.sendRaw({ type: 'hello', client: 'wechat-mini-program', ts: Date.now() })
    })

    this.wsTask.onMessage((res) => {
      try {
        const msg = JSON.parse(res.data)
        if (msg.type === 'telemetry') {
          this.applyTelemetry(msg.data || msg)
        } else if (msg.type === 'ack') {
          this.addLog(`设备响应：${msg.message || msg.cmd || 'OK'}`)
        } else if (msg.type === 'error') {
          this.addLog(`错误：${msg.message || '未知错误'}`)
        }
      } catch (err) {
        this.addLog('收到非 JSON 数据：' + String(res.data).slice(0, 48))
      }
    })

    this.wsTask.onError(() => {
      this.addLog('连接异常：请检查树莓派 IP、端口和同一 Wi-Fi')
      this.setPartial({ connected: false, connecting: false, statusLevel: 'offline' })
      this.scheduleReconnect()
    })

    this.wsTask.onClose(() => {
      this.stopHeartbeat()
      this.setPartial({ connected: false, connecting: false, statusLevel: 'offline' })
      if (this.reconnectingByError) return
      this.addLog('连接已断开')
    })
    return true
  },

  disconnect() {
    this.closeSocket()
    this.addLog('已手动断开连接')
  },

  closeSocket(showState = true) {
    this.stopHeartbeat()
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer)
      this.reconnectTimer = null
    }
    if (this.wsTask) {
      try { this.wsTask.close() } catch (e) {}
      this.wsTask = null
    }
    if (showState) {
      this.setPartial({ connected: false, connecting: false, statusLevel: 'offline' })
    }
  },

  scheduleReconnect() {
    if (this.reconnectTimer || !this.globalData.host) return
    this.reconnectingByError = true
    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null
      if (!this.globalData.connected) this.connect()
    }, 2500)
  },

  startHeartbeat() {
    this.stopHeartbeat()
    this.heartbeatTimer = setInterval(() => {
      this.sendRaw({ type: 'ping', ts: Date.now() })
    }, 2000)
  },

  stopHeartbeat() {
    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer)
      this.heartbeatTimer = null
    }
  },

  applyTelemetry(input) {
    const stageKey = input.stage || this.globalData.telemetry.stage || 'idle'
    const stageInfo = STAGE_MAP[stageKey] || STAGE_MAP.idle
    const total = clampNumber(input.total_count, 12)
    const count = Math.max(0, Math.min(total, clampNumber(input.plant_count, 0)))
    const progress = total > 0 ? Math.round((count / total) * 100) : 0

    const telemetry = {
      ...this.globalData.telemetry,
      ...input,
      stageText: input.stageText || stageInfo.text,
      stageDesc: input.stageDesc || stageInfo.desc,
      plant_count: count,
      total_count: total,
      progress,
      ultrasonic_cm: clampNumber(input.ultrasonic_cm, this.globalData.telemetry.ultrasonic_cm),
      imu: { ...this.globalData.telemetry.imu, ...(input.imu || {}) },
      gps: { ...this.globalData.telemetry.gps, ...(input.gps || {}) },
      radar: { ...this.globalData.telemetry.radar, ...(input.radar || {}) },
      battery: { ...this.globalData.telemetry.battery, ...(input.battery || {}) },
      wheel_speed: { ...this.globalData.telemetry.wheel_speed, ...(input.wheel_speed || {}) }
    }

    telemetry.radar.leftPercent = Math.min(100, Math.round(clampNumber(telemetry.radar.left, 0) / 3))
    telemetry.radar.frontPercent = Math.min(100, Math.round(clampNumber(telemetry.radar.front, 0) / 3))
    telemetry.radar.rightPercent = Math.min(100, Math.round(clampNumber(telemetry.radar.right, 0) / 3))

    const statusLevel = telemetry.row_end ? 'warn' : (telemetry.stage === 'error' ? 'danger' : 'online')
    this.setPartial({ telemetry, lastUpdate: nowText(), statusLevel })
  },

  sendRaw(obj) {
    if (!this.wsTask || !this.globalData.connected) return false
    try {
      this.wsTask.send({ data: JSON.stringify(obj) })
      return true
    } catch (e) {
      this.addLog('发送失败：连接不可用')
      return false
    }
  },

  sendCmd(cmd, payload = {}) {
    const ok = this.sendRaw({ type: 'cmd', cmd, ...payload, ts: Date.now() })
    if (!ok) wx.showToast({ title: '未连接树莓派', icon: 'none' })
    return ok
  },

  addLog(text) {
    const item = { time: nowText(), text }
    const logList = [item, ...(this.globalData.logList || [])].slice(0, 6)
    this.globalData.logList = logList
    this.notify()
  }
})

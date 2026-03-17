// pages/records/index.js
Page({
  data: {
    records: [],
    loading: false
  },

  onLoad() {
    this.loadRecords()
  },

  // 下拉刷新
  onPullDownRefresh() {
    this.loadRecords().then(() => {
      wx.stopPullDownRefresh()
    })
  },

  // 加载数据
  async loadRecords() {
    this.setData({ loading: true })
    
    try {
      const db = wx.cloud.database()
      const { data } = await db.collection('student_records')
        .orderBy('createTime', 'desc')  // 按时间倒序
        .get()
      
      // 格式化时间
      const records = data.map(item => {
        const date = new Date(item.timestamp || item.createTime)
        return {
          ...item,
          formattedTime: this.formatTime(date)
        }
      })
      
      this.setData({
        records: records,
        loading: false
      })
      
      console.log('加载记录:', records)
      
    } catch (err) {
      console.error('加载失败:', err)
      wx.showToast({
        title: '加载失败',
        icon: 'none'
      })
      this.setData({ loading: false })
    }
  },

  // 格式化时间
  formatTime(date) {
    const year = date.getFullYear()
    const month = (date.getMonth() + 1).toString().padStart(2, '0')
    const day = date.getDate().toString().padStart(2, '0')
    const hour = date.getHours().toString().padStart(2, '0')
    const minute = date.getMinutes().toString().padStart(2, '0')
    const second = date.getSeconds().toString().padStart(2, '0')
    return `${year}-${month}-${day} ${hour}:${minute}:${second}`
  }
})
Page({
  // 页面的初始数据
  data: {
    testResult: ''  // 用来显示测试结果
  },

  // 生命周期函数--监听页面加载
  onLoad: function (options) {
    console.log('页面加载完成')
    // 页面加载时自动测试一次（可选）
    // this.testCloudFunction()
  },

  // 测试调用云函数
  async testCloudFunction() {
    console.log('开始测试云函数...')
    
    try {
      const res = await wx.cloud.callFunction({
        name: 'receiveData',
        data: {
          studentId: '2021001',
          name: '张三',
          timestamp: new Date().getTime()
        }
      })
      
      console.log('调用结果:', res)
      
      // 更新页面显示结果
      this.setData({
        testResult: `成功：${res.result.message}`
      })
      
      wx.showToast({
        title: res.result.message,
        icon: 'success'
      })
      
    } catch (err) {
      console.error('调用失败:', err)
      
      this.setData({
        testResult: `失败：${err.message}`
      })
      
      wx.showToast({
        title: '调用失败',
        icon: 'none'
      })
    }
  }
})
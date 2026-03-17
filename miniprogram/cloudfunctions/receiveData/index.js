// 云函数入口文件
const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()

exports.main = async (event, context) => {
  const { studentId, name, timestamp } = event
  
  console.log('接收到的参数:', event)

  if (!studentId || !name) {
    return { code: 400, message: '缺少参数' }
  }

  try {
    const result = await db.collection('student_records').add({
      data: {
        studentId: studentId,
        name: name,
        timestamp: timestamp || Date.now(),
        createTime: db.serverDate()
      }
    })

    console.log('写入成功，id:', result._id)
    return { code: 200, message: '数据保存成功', id: result._id }
    
  } catch (err) {
    console.error('写入失败:', err)
    return { code: 500, message: '数据库错误', error: err.message }
  }
}
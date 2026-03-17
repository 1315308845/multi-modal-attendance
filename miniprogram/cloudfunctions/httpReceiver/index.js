// 云函数入口文件
const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()

// 主入口 - 处理HTTP请求
exports.main = async (event, context) => {
  
  // 获取HTTP请求参数
  // GET请求用 event.queryStringParameters
  // POST请求用 event.body（需要JSON解析）
  
  let params = {}
  
  // 处理POST请求
  if (event.body) {
    try {
      params = JSON.parse(event.body)
    } catch(e) {
      params = event.body
    }
  }
  
  // 处理GET请求（兼容）
  if (event.queryStringParameters) {
    params = { ...params, ...event.queryStringParameters }
  }
  
  console.log('收到请求:', params)
  
  const { studentId, name, timestamp } = params
  
  // 参数校验
  if (!studentId || !name) {
    return {
      statusCode: 400,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ code: 400, message: '缺少参数 studentId 或 name' })
    }
  }
  
  try {
    // 写入数据库
    const result = await db.collection('student_records').add({
      data: {
        studentId: String(studentId),
        name: String(name),
        timestamp: timestamp || Date.now(),
        createTime: db.serverDate(),
        source: 'ESP32_HTTP'  // 标记来源
      }
    })
    
    console.log('写入成功:', result._id)
    
    return {
      statusCode: 200,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ 
        code: 200, 
        message: '数据保存成功',
        id: result._id 
      })
    }
    
  } catch (err) {
    console.error('写入失败:', err)
    return {
      statusCode: 500,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ 
        code: 500, 
        message: '服务器错误',
        error: err.message 
      })
    }
  }
}
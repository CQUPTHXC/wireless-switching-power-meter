//该文件由embed_files.py自动生成,请勿手动修改
#ifndef index_html_H
#define index_html_H
const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="zh">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>实时电压电流监测</title>
  <style>
    /* 全局样式 */
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      margin: 0;
      padding: 0;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      background-color: #FFFFFF;
    }

    .container {
      width: 90%;
      max-width: 400px;
      background-color: #ffecd1;
      padding: 20px;
      border-radius: 12px;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
      display: flex;
      flex-direction: column;
      align-items: center;
      box-sizing: border-box;
    }

    /* 实时数据样式 */
    .real-time-data {
      width: 100%;
      margin-bottom: 20px;
      text-align: center;
    }

    .real-time-data div {
      font-size: 52px;
      font-weight: 600;
      color: #001524;
      margin: 12px 0;
      padding: 15px;
      background-color: #b7b7a4;
      border-radius: 12px;
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
    }

    .real-time-data span {
      /* 等宽字体 */
      font-family: monospace;
    }

    /* 圆形开关样式 */
    .switch {
      position: relative;
      display: inline-block;
      width: 20vh;
      height: 20vh;
      /* 高度为页面高度的20% */
      max-height: 100px;
      /* 最大高度限制 */
      margin: 20px 0;
      /* 上下留出间距 */
    }

    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      transition: 0.2s;
      border-radius: 50vh;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: calc(100% - 8px);
      /* 滑块高度 */
      width: calc(50% - 4px);
      /* 滑块宽度 */
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: 0.2s;
      border-radius: 50%;
    }

    input:checked+.slider {
      background-color: #00f5d4;
    }

    input:checked+.slider:before {
      transform: translateX(calc(100% - 8px));
      /* 滑块滑动距离 */
    }
  </style>
</head>

<body>
  <div class="container">
    <!-- 实时数据展示 -->
    <div class="real-time-data">
      <div><span id="voltage" style="color: #d90429;">0.000</span>V</div>
      <div><span id="current" style="color: #00f5d4;">0.000</span>A</div>
      <div><span id="power" style="color: #00bbf9;">0.000</span>W</div>
      <div><span id="mah" style="color: #9b5de5;">0.000</span>mAH</div>
    </div>

    <!-- 圆形开关按钮 -->
    <label class="switch">
      <input type="checkbox" id="switchButton" onclick="toggleState()">
      <span class="slider"></span>
    </label>
  </div>

  <script>
    // 格式化浮动数值为固定长度的字符串
    function sprintfFloat(value, len) {
      // 计算最大值
      const max = Math.pow(10, len) - 1;

      // 如果值大于最大值，则将其设置为最大值
      if (value > max) {
        value = max;
      }

      // 如果值小于0，则将其设置为0
      if (value < 0) {
        value = 0;
      }

      // 将浮点数转换为字符串，并保留6位小数
      let formattedValue = value.toFixed(6);

      // 截取字符串的前len+1个字符（包括小数点）
      formattedValue = formattedValue.substring(0, len + 1);

      return formattedValue;
    }
    // 获取后端数据并更新UI
    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('voltage').innerText = sprintfFloat(parseFloat(data.voltage), 4);
          document.getElementById('current').innerText = sprintfFloat(parseFloat(data.current), 4);
          document.getElementById('power').innerText = sprintfFloat(parseFloat(data.voltage) * parseFloat(data.current), 4);
          document.getElementById('mah').innerText = sprintfFloat(parseFloat(data.mah), 4);
        })
        .catch(err => console.error('获取数据失败:', err));
    }

    // 刷新数据
    setInterval(fetchData, 200);

    // 按钮开关状态
    let isOff = true;

    function toggleState() {
      isOff = !isOff;
      const slider = document.querySelector('.slider');
      const button = document.getElementById('switchButton');
      if (isOff) {
        slider.classList.remove('off');
      } else {
        slider.classList.add('off');
      }

      // 发送开关状态到后端
      fetch('/ctrl', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ state: isOff })
      })
        .then(response => response.json())
        .catch(err => console.error('开关状态更新失败:', err));
    }

    fetchData(); // 启动第一次数据加载
  </script>
</body>

</html>
)=====";
#endif

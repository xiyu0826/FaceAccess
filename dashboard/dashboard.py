# -*- coding: utf-8 -*-
"""
智能家居数据看板 + 远程控制
1. 订阅 broker.emqx.io 的 smart_home/test (STM32 发的数据)
2. 网页显示数据 + 报警状态
3. 网页按钮 -> 发布指令到 smart_home/cmd (控制 STM32)
运行: python dashboard.py
浏览器打开 http://电脑IP:8080
"""
import threading
import json
import time
import paho.mqtt.client as mqtt
from flask import Flask, jsonify, request

app = Flask(__name__)

latest = {"temp": "-", "humi": "-", "air": "-", "light": "-", "pir": "-", "alarm": 0, "armed": 0, "door": "-"}
mqttc = None
lock = threading.Lock()


def on_message(client, userdata, msg):
    global latest
    try:
        data = json.loads(msg.payload.decode('utf-8'))
        with lock:
            for k in data:
                latest[k] = data[k]
        print("收到:", data)
    except Exception as e:
        print("解析失败:", e)


def on_connect(client, userdata, flags, rc):
    print("MQTT 已连接, rc=", rc)
    client.subscribe("smart_home/test")


def on_disconnect(client, userdata, rc):
    print("MQTT 断开, rc=", rc)


def mqtt_loop():
    global mqttc
    # 循环重连: 断线后自动重连
    while True:
        try:
            mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
            mqttc.on_message = on_message
            mqttc.on_connect = on_connect
            mqttc.on_disconnect = on_disconnect
            mqttc.connect("broker.emqx.io", 1883, 60)
            print("MQTT 订阅中... topic=smart_home/test")
            mqttc.loop_forever()
        except Exception as e:
            print("MQTT 连接失败, 3秒后重试:", e)
            time.sleep(3)


def publish_cmd(cmd):
    """发布控制指令"""
    if mqttc:
        mqttc.publish("smart_home/cmd", cmd)
        print("下发指令:", cmd)
        return True
    return False


HTML = """
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="Cache-Control" content="no-store, no-cache, must-revalidate">
<title>智能家居监控</title>
<style>
  body { font-family: Arial, sans-serif; background:#1a1a2e; color:#eee;
         display:flex; flex-direction:column; align-items:center; padding:20px; }
  h1 { color:#fff; }
  .grid { display:grid; grid-template-columns:repeat(auto-fit,minmax(150px,1fr));
          gap:15px; width:100%; max-width:600px; }
  .card { background:#16213e; border-radius:12px; padding:20px; text-align:center; }
  .label { font-size:14px; color:#888; }
  .value { font-size:32px; font-weight:bold; margin-top:8px; }
  .temp { color:#ff6b6b; } .humi { color:#4ecdc4; }
  .air { color:#ffe66d; } .light { color:#ffd93d; } .pir { color:#6bff6b; }
  .alarm0 { color:#6bff6b; } .alarm1 { color:#ffe66d; }
  .alarm2 { color:#ffa94d; } .alarm3 { color:#ff6b6b; }
  .ctrl { margin-top:25px; display:flex; gap:15px; flex-wrap:wrap; justify-content:center; }
  .btn { padding:14px 24px; font-size:18px; border:none; border-radius:10px;
         cursor:pointer; color:#fff; }
  .btn-arm { background:#28a745; }
  .btn-disarm { background:#6c757d; }
  .btn-mute { background:#ffc107; color:#333; }
  .btn-unmute { background:#17a2b8; }
  .btn:active { transform:scale(0.95); }
  .armed { color:#6bff6b; } .disarmed { color:#ff6b6b; }
</style>
</head>
<body>
<h1>🏠 智能家居环境监控 <small style="font-size:14px;color:#888;">v7</small></h1>
<div style="color:#888; font-size:12px; margin-bottom:8px;">最后更新: <span id="updated">-</span></div>
<div class="grid">
  <div class="card"><div class="label">温度</div><div class="value temp">{{ temp }}°C</div></div>
  <div class="card"><div class="label">湿度</div><div class="value humi">{{ humi }}%</div></div>
  <div class="card"><div class="label">空气质量</div><div class="value air">{{ air }}</div></div>
  <div class="card"><div class="label">光照</div><div class="value light">{{ light }}</div></div>
  <div class="card"><div class="label">人体</div><div class="value pir">{{ pir }}</div></div>
  <div class="card"><div class="label">报警状态</div><div class="value" id="alarm">正常</div></div>
  <div class="card"><div class="label">布防状态</div><div class="value" id="armed">布防</div></div>
  <div class="card"><div class="label">最近开门</div><div class="value" id="door">-</div></div>
</div>

<div class="ctrl">
  <button class="btn btn-arm" onclick="send('ARM', this)">🔒 布防</button>
  <button class="btn btn-disarm" onclick="send('DISARM', this)">🔓 撤防</button>
  <button class="btn btn-mute" onclick="send('MUTE', this)">🔕 静音</button>
  <button class="btn btn-unmute" onclick="send('UNMUTE', this)">🔔 取消静音</button>
</div>

<div id="toast" style="display:none; position:fixed; bottom:40px; left:50%;
     transform:translateX(-50%); background:#333; color:#fff; padding:12px 24px;
     border-radius:8px; font-size:16px; z-index:999;"></div>

<script>
  var alarmMap = ['正常', '报警L1', '报警L2', '报警L3'];
  var alarmCls = ['alarm0', 'alarm1', 'alarm2', 'alarm3'];
  var airMap = ['优', '良', '差', '危险'];
  var lightMap = ['暗', '较暗', '正常', '亮'];
  var pirMap = ['无人', '有人'];

  function send(cmd, btn) {
    fetch('/cmd/' + cmd + '?_=' + Date.now(), {cache:'no-store'})
      .then(r=>r.json())
      .then(d=>{
        var map = {ARM:'已发送布防指令', DISARM:'已发送撤防指令', MUTE:'已发送静音指令', UNMUTE:'已发送取消静音指令'};
        showToast(map[cmd] || cmd);
        // 不做乐观更新, 状态以 STM32 上报为准 (避免来回跳)
      })
      .catch(function(e){ showToast('发送失败: ' + e); });
  }

  function setArmed(armed) {
    var el = document.getElementById('armed');
    if (armed) {
      el.textContent = '布防';
      el.className = 'value armed';
    } else {
      el.textContent = '撤防';
      el.className = 'value disarmed';
    }
  }

  function showToast(msg) {
    var t = document.getElementById('toast');
    t.textContent = msg;
    t.style.display = 'block';
    setTimeout(function(){ t.style.display = 'none'; }, 1500);
  }

  function refresh() {
    fetch('/data?_=' + Date.now(), {cache:'no-store'}).then(r=>r.json()).then(d=>{
      document.querySelector('.temp').textContent = d.temp + '°C';
      document.querySelector('.humi').textContent = d.humi + '%';
      document.querySelector('.air').textContent = airMap[d.air] || d.air;
      document.querySelector('.light').textContent = lightMap[d.light] || d.light;
      document.querySelector('.pir').textContent = pirMap[d.pir] || d.pir;
      document.getElementById('door').textContent = d.door || '-';
      var a = d.alarm;
      var el = document.getElementById('alarm');
      el.textContent = alarmMap[a] || alarmMap[0];
      el.className = 'value ' + (alarmCls[a] || alarmCls[0]);

      var armedEl = document.getElementById('armed');
      if (d.armed === 1) {
        armedEl.textContent = '布防';
        armedEl.className = 'value armed';
      } else {
        armedEl.textContent = '撤防';
        armedEl.className = 'value disarmed';
      }
      var t = new Date();
      document.getElementById('updated').textContent =
        t.getHours() + ':' + t.getMinutes() + ':' + t.getSeconds();
    }).catch(function(e){
      document.getElementById('updated').textContent = '连接失败: ' + e;
    });
  }

  setInterval(refresh, 1000);
  refresh();
</script>
</body>
</html>
"""


@app.route("/")
def index():
    return HTML


@app.route("/data")
def data():
    with lock:
        return jsonify(dict(latest))


@app.route("/cmd/<cmd>", methods=["GET", "POST"])
def cmd(cmd):
    ok = publish_cmd(cmd)
    print("收到按钮指令:", cmd, "发布结果:", ok)
    return jsonify({"ok": ok})


@app.after_request
def no_cache(resp):
    resp.headers["Cache-Control"] = "no-store, no-cache, must-revalidate, max-age=0"
    resp.headers["Pragma"] = "no-cache"
    resp.headers["Expires"] = "0"
    return resp


if __name__ == "__main__":
    t = threading.Thread(target=mqtt_loop, daemon=True)
    t.start()
    print("网页服务器启动: http://0.0.0.0:8080")
    app.run(host="0.0.0.0", port=8080, threaded=True)

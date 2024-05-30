var ws;

function update_text(text) {
  var chat_messages = document.getElementById("messages");
  chat_messages.innerHTML += text + '<br>';
  chat_messages.scrollTop = chat_messages.scrollHeight;
}

function send_onclick() {
  if(ws != null) {
    var message = document.getElementById("message").value;
    
    if (message) {
      document.getElementById("message").value = "";
      ws.send(message + "\n");
      update_text('<span style="color:navy">' + message + '</span>');
      // You can send the message to the server or process it as needed
    }
  }
}

function connect_onclick() {
  if (window.location.host.length < 1) {
    console.log("Cant connect to ws, reason - no host")
  }
  else {
    if (ws == null) {
      ws = new WebSocket("ws://" + window.location.hostname + ":81");
      ws.onopen = ws_onopen;
      ws.onclose = ws_onclose;
      ws.onmessage = ws_onmessage;
      document.getElementById("ws_state").innerHTML = "CONNECTING";
    } else
      ws.close();
  }
}

function switch_onclick() {
  if (ws != null) {
    const control = /**@type {HTMLInputElement}*/ (document.getElementById("switch-checkbox"))
    ws.send(">s:" + (control.checked ? 1 : 0));
  }
}

function ws_onopen() {
  document.getElementById("ws_state").innerHTML = "<span style='color:blue'>CONNECTED</span>";
  document.getElementById("bt_connect").innerHTML = "Disconnect";
  document.getElementById("messages").innerHTML = "";
}

function ws_onclose() {
  document.getElementById("ws_state").innerHTML = "<span style='color:gray'>CLOSED</span>";
  document.getElementById("bt_connect").innerHTML = "Connect";
  ws.onopen = null;
  ws.onclose = null;
  ws.onmessage = null;
  ws = null;
}

function ws_onmessage(msg) {
  const TEMPERATURE_PREFIX = ">t:", SWITCHSTATE_PREFIX = ">s:"

  if (msg.data.startsWith(TEMPERATURE_PREFIX)) {
    const t = Number.parseFloat(msg.data.substring(TEMPERATURE_PREFIX.length))
    document.getElementById("temperature-value").innerText = t.toString()
    if (t > -127.0) {
      temperatureChart.addSample(Math.round((Date.now() - time) / 1000), t)
      chartPanel.draw()    
    }
  }
  else if (msg.data.startsWith(SWITCHSTATE_PREFIX)) {
    const s = Boolean(Number.parseInt(msg.data.substring(SWITCHSTATE_PREFIX.length)))
    const control = /**@type {HTMLInputElement}*/ (document.getElementById("switch-checkbox"))
    control.checked = s
  }
  else update_text('<span style="color:blue">' + msg.data + '</span>')
}

function sendt(t) {
  ws_onmessage({data:">t:" + t.toString()}) 
}
function sends(ison) {
  ws_onmessage({data:">s:" + (ison ? 1 : 0)}) 
}

const chartPanel = new ChartPanel(
  /**@type {HTMLCanvasElement}*/ (document.getElementById("chart-canvas")),
  400, 250)

chartPanel.defaultBounds.width = 5
chartPanel.defaultBounds.height = 2
const temperatureChart = new Chart("#007bff").withSampleLimit(60)
chartPanel.addChart(temperatureChart)
chartPanel.draw()
const time = Date.now();

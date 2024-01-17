var ws;
var wsm_max_len = 4096; /* bigger length causes uart0 buffer overflow with low speed smart device */

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
      ws = new WebSocket("ws://" + window.location.host + ":81");
      ws.onopen = ws_onopen;
      ws.onclose = ws_onclose;
      ws.onmessage = ws_onmessage;
      document.getElementById("ws_state").innerHTML = "CONNECTING";
    } else
      ws.close();
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

function ws_onmessage(e_msg) {
  // e_msg = e_msg || window.event; // MessageEvent
  console.log(e_msg.data);
  const temperaturePrefix = ">t:"
  if (e_msg.data.startsWith(temperaturePrefix)) {
    const temperature = Number.parseFloat(e_msg.data.substring(temperaturePrefix.length))
    document.getElementById("temperature-value").innerText = temperature.toString()
    if (temperature > -127.0) {
      temperatureChart.addSample(Math.round((Date.now() - time) / 1000), temperature)
      chartPanel.draw()    
    }
      //e_msg.data.substring(temperaturePrefix.length)
  }
  else update_text('<span style="color:blue">' + e_msg.data + '</span>');
}

function sendt(t) {
  ws_onmessage({data:">t:" + t.toString()}) 
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

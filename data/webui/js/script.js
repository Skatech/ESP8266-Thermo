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
  if(ws == null) {
    ws = new WebSocket("ws://" + window.location.host + ":81");
    document.getElementById("ws_state").innerHTML = "CONNECTING";
    ws.onopen = ws_onopen;
    ws.onclose = ws_onclose;
    ws.onmessage = ws_onmessage;
  } else
    ws.close();
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
    document.getElementById("temperature-value").innerText = 
      e_msg.data.substring(temperaturePrefix.length)
  }
  else update_text('<span style="color:blue">' + e_msg.data + '</span>');
}
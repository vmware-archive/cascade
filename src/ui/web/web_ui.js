window.onload = function() {
  var ws = new WebSocket('ws://' + location.host + '/ws');

  ws.onopen = function(ev) { };
  ws.onerror = function(ev) { };
  ws.onclose = function(ev) { };

  ws.onmessage = function(ev) {
    var res = JSON.parse(ev.data);
    if (res.api == 'log') {
      document.getElementById('log').innerHTML += res.val;
    } else if (res.api == 'eval') {
      var list = document.getElementById('list');
      var idx = -1;
      for (var i = 0; i < list.options.length; i++) {
        if (list.options[i].text == res.val.text) {
          idx = i;
          break;
        }
      }
      if (idx == -1) {
        list.options.add(new Option(res.val.text, res.val.value));
        list.options.selectedIndex = list.options.length-1;
      } else {
        list.options[idx] = new Option(res.val.text, res.val.value);
      }
      list.onchange();
    } else if (res.api == 'freq') {
      document.getElementById('freq').innerHTML = res.val;
    }
  };

  var ev = document.getElementById('eval');
  ev.onclick = function() {
    ws.send('eval:' + document.getElementById('edit').value);
  }

  var imp = document.getElementById('import');
  imp.onclick = function() {
    var std = document.getElementById('std').value;
    var target = document.getElementById('target').value;
    var loc = document.getElementById('loc').value;
    var iwidth = document.getElementById('input').value;
    var owidth = document.getElementById('output').value;

    var mid = std.charAt(0).toUpperCase() + std.slice(1);
    var decl = 
      "(*__std=\"" + std + "\", __target=\"" + target + "\", __loc=\"" + loc + "\"*)" +
      "module " + mid + "(" + (iwidth > 0 ? "in_" : "") + (iwidth > 0 && owidth > 0 ? "," : "") + (owidth > 0 ? "out_" : "") + ");" +
      (iwidth > 0 ? "input  wire" : "") + (iwidth == 1 ? "" : iwidth > 1 ? ("[" + (iwidth-1) + ":0]") : "") + (iwidth > 0 ? " in_;" : "") +
      (owidth > 0 ? "output wire" : "") + (owidth == 1 ? "" : owidth > 1 ? ("[" + (owidth-1) + ":0]") : "") + (owidth > 0 ? " out_;" : "") +
      "endmodule";
  
    ws.send('eval:' + decl);    
    if (document.getElementById('instantiate').value == 1) {
      var inst = mid + " " + std + "();";
      ws.send('eval:' + inst);
    }
  }

  var list = document.getElementById('list');
  list.onchange = function() {
    document.getElementById('src').innerHTML = list.options[list.selectedIndex].value;
  }

  var clear = document.getElementById('clear');
  clear.onclick = function() {
    document.getElementById('log').innerHTML = "";
  }

  setInterval(function() {ws.send('freq:');}, 1000);
  setInterval(function() {ws.send('pull:');}, 1000);
}

var v4l = require('../build/Release/v4l');
var http = require('http');

http.createServer(function(req, res) {
  res.writeHead(200, {'Content-Type': 'image/jpg'});
  res.end(v4l.dump("/dev/video0", 640, 480, 3));
}).listen(8421);

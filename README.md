node-v4l - Video 4 Linux in node.js
===


This project aims to be an addon module for node.js for accessing v4l devices natively.

The module provides a simple dump() method, which returns the image in (currently)
jpeg format. There are a couple of parameters which controls the image grabbing process.

The module was written and tested on Raspberry Pi Model B using an usb connected camera.

Version:
---

0.1 Alpha


Syntax (simple example):
---

```
var v4l = require('build/Release/v4l');
v4l.dump(device, width, height, skipFrames); // The dump method will return the binary image data
```

Options:
---

- device: The device to open (default = /dev/video0)
- width: The width for result image (default = 640)
- height: The height for result image (default = 480)
- skipFrames: The number of frames to ignore (default 10)


Credits:
---

* [node.js] - evented I/O for the backend
* [mchehab@infradead.org] - Who wrote the simple example for v4l grabbing


License:
---
MIT

[node.js]:http://nodejs.org
[mchehab@infradead.org]:http://linuxtv.org/downloads/v4l-dvb-apis/v4l2grab-example.html

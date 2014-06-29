var addon = require('../build/Release/v4l');
var fs = require('fs');

fs.writeFile("./cam320.jpg", addon.dump("/dev/video0", 320, 240, 4), function(err) {
  if(err)
  {
    console.log(err);
  }
  else
  {
    console.log("Saved jpg");
  }
});

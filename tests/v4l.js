var addon = require('./build/Release/v4l');
var fs = require('fs');

fs.writeFile("./test.jpg", addon.dump(), function(err) {
  if(err)
  {
    console.log(err);
  }
  else
  {
    console.log("Saved jpg");
  }
});

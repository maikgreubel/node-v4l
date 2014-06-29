{
	"targets": [
		{
			"target_name": "v4l",
			"sources": [ "addon.cc", "v4l.cc", "tojpg.cc", "grab.cc" ],
      "libraries": [ "-lturbojpeg", "-lv4l2" ]
		}
	]
}

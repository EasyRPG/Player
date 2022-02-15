// Move to different directory to prevent save file collisions in IDBFS
FS.mkdir("easyrpg");
FS.chdir("easyrpg");

if (Module.game.length > 0) {
  FS.mkdir(Module.game);
  FS.chdir(Module.game);
}

// Use IDBFS for save file storage when the filesystem was not
// overwritten by a custom emscripten shell file
if (typeof Module.EASYRPG_FS === "undefined") {
  Module.saveFs = IDBFS;
}

// Display the nice end message forever
Module["onExit"] = function() {
  // load image
  let imageContent = FS.readFile("/tmp/message.png");
  var img = document.createElement('img');
  img.id = "canvas";
  img.src = URL.createObjectURL(new Blob([imageContent], {type: "image/png"}));

  // replace canvas
  var cvs = document.getElementById('canvas');
  cvs.parentNode.replaceChild(img, cvs);
}

// Move to different directory to prevent save file collisions in IDBFS
FS.mkdir("easyrpg");
FS.chdir("easyrpg");

if (Module.game.length > 0) {
  FS.mkdir(Module.game);
  FS.chdir(Module.game);
}

// Use IDBFS for save file storage when the filesystem was not
// overwritten by a custom emscripten shell file
if (Module.saveFs === undefined) {
  Module.saveFs = IDBFS;
}

Module.initApi = function() {
  Module.api_private.download_js = function(buffer, size, filename) {
    const blob = new Blob([Module.HEAPU8.slice(buffer, buffer + size)]);
    const link = document.createElement('a');
    link.href = window.URL.createObjectURL(blob);
    link.download = UTF8ToString(filename);
    link.click();
    link.remove();
  }

  Module.api_private.uploadSavegame_js = function(slot) {
    let saveFile = document.getElementById('easyrpg_saveFile');
    if (saveFile == null) {
      saveFile = document.createElement('input');
      saveFile.type = 'file';
      saveFile.id = 'easyrpg_saveFile';
      saveFile.style.display = 'none';
      saveFile.addEventListener('change', function(evt) {
        const save = evt.target.files[0];
        const reader = new FileReader();
        reader.onload = function (file) {
          const result = new Uint8Array(file.currentTarget.result);
          var buf = Module._malloc(result.length);
          Module.HEAPU8.set(result, buf);
          Module.api_private.uploadSavegameStep2(slot, buf, result.length);
          Module._free(buf);
          Module.api.refreshScene();
        };
        reader.readAsArrayBuffer(save);
      });
    }
    saveFile.click();
  }
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

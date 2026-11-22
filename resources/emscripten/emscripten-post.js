Module.initApi = function() {
  Module.api_private.download_js = function (buffer, size, filename) {
    const blob = new Blob([Module.HEAPU8.slice(buffer, buffer + size)]);
    const link = document.createElement('a');
    link.href = window.URL.createObjectURL(blob);
    link.download = UTF8ToString(filename);
    link.click();
    link.remove();
  }

  Module.api_private.createInputElement_js = function (id, event, accept = '') {
    return new Promise((resolve) => {
      const file = document.createElement('input');
      file.type = 'file';
      // Without a type filter, mobile browsers offer the camera next to the files.
      file.accept = accept;
      file.id = id;
      file.style.display = 'none';

      file.addEventListener('change', function (evt) {
        const selected_file = evt.target.files[0];
        if (!selected_file) {
          resolve(false);
          return;
        }
        const reader = new FileReader();
        reader.onload = function (e) {
          const res = event(e, selected_file.name);
          resolve(res !== undefined ? res : true);
        };
        reader.onerror = function () {
          resolve(false);
        };
        reader.readAsArrayBuffer(selected_file);
      }, { once: true });

      file.addEventListener('cancel', function () {
        resolve(false);
      }, { once: true });

      file.click();
    });
  }

  Module.api_private.uploadSavegame_js = function (slot) {
    // iOS drops extensions it has no MIME type for, so `.lsd` alone would still show the camera there.
    // The MIME type lets desktop pickers list more than `.lsd`; the engine rejects what it can't load.
    const savegameTypes = '.lsd,application/octet-stream';
    return Module.api_private.createInputElement_js('easyrpg_saveFile', function (file) {
      const result = new Uint8Array(file.currentTarget.result);
      const buf = Module._malloc(result.length);
      Module.HEAPU8.set(result, buf);
      const success = Module.api_private.uploadSavegameStep2(slot, buf, result.length);
      Module._free(buf);
      if (success) {
        Module.api.refreshScene();
      }
      return success;
    }, savegameTypes);
  }

  Module.api_private.uploadSoundfont_js = function () {
    return Module.api_private.createInputElement_js('easyrpg_sfFile', function (file, name) {
      const result = new Uint8Array(file.currentTarget.result);
      const content_buf = Module._malloc(result.length);
      Module.HEAPU8.set(result, content_buf);
      const success = Module.api_private.uploadSoundfontStep2(name, content_buf, result.length);
      Module._free(content_buf);
      if (success) {
        Module.api.refreshScene();
      }
      return success;
    });
  }

  Module.api_private.uploadFont_js = function () {
    return Module.api_private.createInputElement_js('easyrpg_fontFile', function (file, name) {
      const result = new Uint8Array(file.currentTarget.result);
      const content_buf = Module._malloc(result.length);
      Module.HEAPU8.set(result, content_buf);
      const success = Module.api_private.uploadFontStep2(name, content_buf, result.length);
      Module._free(content_buf);
      if (success) {
        Module.api.refreshScene();
      }
      return success;
    });
  }
}

// Display the nice end message forever
Module["onExit"] = function() {
  // load image
  const imageContent = FS.readFile("/tmp/message.png");
  const img = document.createElement('img');
  img.id = "canvas";
  img.src = URL.createObjectURL(new Blob([imageContent], {type: "image/png"}));

  // replace canvas
  const cvs = document.getElementById('canvas');
  cvs.parentNode.replaceChild(img, cvs);
}

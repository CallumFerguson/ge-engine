self.onmessage = function (event) {
  importScripts("emscriptenUtility.js");

  const imageData = parseHDR(event.data);
  self.postMessage(imageData);
};

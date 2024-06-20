function copyExternalImageToTexture(
  deviceJsHandle,
  textureJsHandle,
  data,
  size,
  shouldGenerateMipmap,
  mipLevel,
  imageType
) {
  const copyExternalImageToTextureAsync = async () => {
    const device = JsValStore.get(deviceJsHandle);
    const texture = JsValStore.get(textureJsHandle);

    const imageTypeString = UTF8ToString(imageType);

    if (imageTypeString === "hdr") {
      const imageData = await new Promise((resolve, reject) => {
        const worker = new Worker("parseHDRWorker.js");

        worker.postMessage(HEAPU8.subarray(data, data + size));

        worker.onmessage = function (event) {
          resolve(event.data);
        };

        worker.onerror = function (error) {
          reject(error);
        };
      });
      device.queue.writeTexture(
        { texture },
        imageData.data.buffer,
        { bytesPerRow: imageData.width * 8 },
        { width: imageData.width, height: imageData.height }
      );
    } else {
      let type = undefined;
      if (imageTypeString === "jpg") {
        type = "image/jpeg";
      } else if (imageTypeString === "png") {
        type = "image/png";
      } else {
        console.log("Unknown imageType: " + imageTypeString);
      }

      const blob = new Blob([HEAPU8.subarray(data, data + size)], {
        type: type,
      });

      const imageBitmap = await createImageBitmap(blob, {
        colorSpaceConversion: "none",
      });

      device.queue.copyExternalImageToTexture(
        { source: imageBitmap, flipY: false },
        { texture, mipLevel },
        { width: imageBitmap.width, height: imageBitmap.height }
      );
    }

    JsValStore.remove(deviceJsHandle);
    Module.ccall(
      "copyExternalImageToTextureFinishCallback",
      null,
      ["number", "boolean"],
      [textureJsHandle, shouldGenerateMipmap]
    );
  };
  copyExternalImageToTextureAsync();
}

function copyExternalImageToTextureFromURL(
  deviceJsHandle,
  textureJsHandle,
  url,
  shouldGenerateMipmap,
  mipLevel
) {
  const copyExternalImageToTextureFromURLAsync = async () => {
    const device = JsValStore.get(deviceJsHandle);
    const texture = JsValStore.get(textureJsHandle);

    const res = await fetch(UTF8ToString(url));
    const blob = await res.blob();
    const imageBitmap = await createImageBitmap(blob, {
      colorSpaceConversion: "none",
    });

    device.queue.copyExternalImageToTexture(
      { source: imageBitmap, flipY: false },
      { texture, mipLevel },
      { width: imageBitmap.width, height: imageBitmap.height }
    );

    JsValStore.remove(deviceJsHandle);
    Module.ccall(
      "copyExternalImageToTextureFinishCallback",
      null,
      ["number", "boolean"],
      [textureJsHandle, shouldGenerateMipmap]
    );
  };
  copyExternalImageToTextureFromURLAsync();
}

mergeInto(LibraryManager.library, {
  copyExternalImageToTexture,
  copyExternalImageToTextureFromURL,
});

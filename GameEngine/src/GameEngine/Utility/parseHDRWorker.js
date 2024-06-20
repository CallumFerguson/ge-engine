// from https://github.com/tchayen/pbr-webgpu/blob/main/src/lib/parseHDR.ts
// Based on https://github.com/vorg/parse-hdr
// https://stackoverflow.com/questions/32633585/how-do-you-convert-to-half-floats-in-javascript
const floatView = new Float32Array(1);
const int32View = new Int32Array(floatView.buffer);

function toHalf(val) {
  floatView[0] = val;
  var x = int32View[0];

  var bits = (x >> 16) & 0x8000; /* Get the sign */
  var m = (x >> 12) & 0x07ff; /* Keep one extra bit for rounding */
  var e = (x >> 23) & 0xff; /* Using int is faster here */

  /* If zero, or denormal, or exponent underflows too much for a denormal
   * half, return signed zero. */
  if (e < 103) {
    return bits;
  }

  /* If NaN, return NaN. If Inf or exponent overflow, return Inf. */
  if (e > 142) {
    bits |= 0x7c00;
    /* If exponent was 0xff and one mantissa bit was set, it means NaN,
     * not Inf, so make sure we set one mantissa bit too. */
    bits |= (e === 255 ? 0 : 1) && x & 0x007fffff;
    return bits;
  }

  /* If exponent underflows but not too much, return a denormal */
  if (e < 113) {
    m |= 0x0800;
    /* Extra rounding may overflow and set mantissa to 0 and exponent
     * to 1, which is OK. */
    bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
    return bits;
  }

  bits |= ((e - 112) << 10) | (m >> 1);
  /* Extra rounding. An overflow will set mantissa to 0 and increment
   * the exponent, which is OK. */
  bits += m & 1;
  return bits;
}

const oneAsHalf = toHalf(1.0);

let radiancePattern = "#\\?RADIANCE";
let commentPattern = "#.*";
let exposurePattern = "EXPOSURE=\\s*([0-9]*[.][0-9]*)";
let formatPattern = "FORMAT=32-bit_rle_rgbe";
let widthHeightPattern = "-Y ([0-9]+) \\+X ([0-9]+)";

function parseHDR(buffer) {
  let fileOffset = 0;
  const bufferLength = buffer.length;

  const NEW_LINE = 10;

  function readLine() {
    let line = "";
    while (++fileOffset < bufferLength) {
      let b = buffer[fileOffset];
      if (b === NEW_LINE) {
        fileOffset += 1;
        break;
      }
      line += String.fromCharCode(b);
    }
    return line;
  }

  let width = 0;
  let height = 0;
  let exposure = 1;
  let gamma = 1;
  // let rle = false;

  for (let i = 0; i < 20; i += 1) {
    let line = readLine();
    let match;
    if ((match = line.match(radiancePattern))) {
    } else if ((match = line.match(formatPattern))) {
      // rle = true;
    } else if ((match = line.match(exposurePattern))) {
      exposure = Number(match[1]);
    } else if ((match = line.match(commentPattern))) {
    } else if ((match = line.match(widthHeightPattern))) {
      height = Number(match[1]);
      width = Number(match[2]);
      break;
    }
  }

  let data = new Uint8Array(width * height * 4);
  let scanlineWidth = width;
  let scanlinesCount = height;

  readPixelsRawRLE(buffer, data, 0, fileOffset, scanlineWidth, scanlinesCount);

  let maxIntensity = 0;

  const channels = 4;
  let floatData = new Uint16Array(width * height * channels);
  // for (let row = 0; row < height; row++) {
  //   for (let col = 0; col < width; col++) {
  for (let offset = 0; offset < data.length; offset += 4) {
    // const offset = (row * width + col) * channels;
    let r = data[offset] / 255;
    let g = data[offset + 1] / 255;
    let b = data[offset + 2] / 255;
    const e = data[offset + 3];
    const scale = Math.pow(2.0, e - 128.0);

    r *= scale;
    g *= scale;
    b *= scale;

    // flip Y
    // const flippedRow = height - row - 1;
    // const floatOffset = (flippedRow * width + col) * channels;

    // flip both
    // const flippedRow = height - row - 1;
    // const flippedCol = width - col - 1;
    // const floatOffset = (flippedRow * width + flippedCol) * channels;

    // don't flip
    const floatOffset = offset;

    floatData[floatOffset] = toHalf(r);
    floatData[floatOffset + 1] = toHalf(g);
    floatData[floatOffset + 2] = toHalf(b);
    floatData[floatOffset + 3] = oneAsHalf;

    const intensity = (r + g + b) / 3;
    if (intensity > maxIntensity) {
      maxIntensity = intensity;
    }
  }
  //   }
  // }

  if (maxIntensity > 25) {
    console.log(
      "hdr image has very high intensities. the intensity will be clamped to 25 for the irradiance map."
    );
    console.log("max intensity: " + maxIntensity);
  }

  return {
    width,
    height,
    exposure,
    gamma,
    data: floatData,
  };
}

function readPixelsRawRLE(
  buffer,
  data,
  offset,
  fileOffset,
  scanlineWidth,
  scanlinesCount
) {
  const rgbe = new Array(4);
  let scanlineBuffer = null;
  let ptr;
  let ptr_end;
  let count;
  const twoBytes = new Array(2);
  const bufferLength = buffer.length;

  function readBuf(buf) {
    let bytesRead = 0;
    do {
      buf[bytesRead++] = buffer[fileOffset];
      fileOffset += 1;
    } while (fileOffset < bufferLength && bytesRead < buf.length);
    return bytesRead;
  }

  function readBufferOffset(buf, offset, length) {
    let bytesRead = 0;
    do {
      buf[offset + bytesRead] = buffer[fileOffset];
      bytesRead += 1;
      fileOffset += 1;
    } while (fileOffset < bufferLength && bytesRead < length);
    return bytesRead;
  }

  function readPixelsRaw(data, offset, numpixels) {
    const numExpected = 4 * numpixels;
    let readCount = readBufferOffset(data, offset, numExpected);
    if (readCount < numExpected) {
      throw new Error(
        "Error reading raw pixels: got " +
          readCount +
          " bytes, expected " +
          numExpected
      );
    }
  }

  while (scanlinesCount > 0) {
    if (readBuf(rgbe) < rgbe.length) {
      throw new Error("Error reading bytes: expected " + rgbe.length);
    }

    if (rgbe[0] !== 2 || rgbe[1] !== 2 || (rgbe[2] & 0x80) !== 0) {
      //this file is not run length encoded
      data[offset + 0] = rgbe[0];
      data[offset + 1] = rgbe[1];
      data[offset + 2] = rgbe[2];
      data[offset + 3] = rgbe[3];
      offset += 4;
      readPixelsRaw(data, offset, scanlineWidth * scanlinesCount - 1);
      return;
    }

    if ((((rgbe[2] & 0xff) << 8) | (rgbe[3] & 0xff)) !== scanlineWidth) {
      throw new Error(
        "Wrong scanline width " +
          (((rgbe[2] & 0xff) << 8) | (rgbe[3] & 0xff)) +
          ", expected " +
          scanlineWidth
      );
    }

    if (scanlineBuffer == null) {
      scanlineBuffer = new Array(4 * scanlineWidth);
    }

    ptr = 0;
    // Read each of the four channels for the scanline into the buffer.
    for (let i = 0; i < 4; i += 1) {
      ptr_end = (i + 1) * scanlineWidth;
      while (ptr < ptr_end) {
        if (readBuf(twoBytes) < twoBytes.length) {
          throw new Error("Error reading 2-byte buffer");
        }
        if ((twoBytes[0] & 0xff) > 128) {
          /* a run of the same value */
          count = (twoBytes[0] & 0xff) - 128;
          if (count === 0 || count > ptr_end - ptr) {
            throw new Error("Bad scanline data");
          }
          while (count-- > 0) {
            scanlineBuffer[ptr++] = twoBytes[1];
          }
        } else {
          /* a non-run */
          count = twoBytes[0] & 0xff;
          if (count === 0 || count > ptr_end - ptr) {
            throw new Error("Bad scanline data");
          }
          scanlineBuffer[ptr++] = twoBytes[1];
          if (--count > 0) {
            if (readBufferOffset(scanlineBuffer, ptr, count) < count) {
              throw new Error("Error reading non-run data");
            }
            ptr += count;
          }
        }
      }
    }

    for (let i = 0; i < scanlineWidth; i += 1) {
      data[offset + 0] = scanlineBuffer[i];
      data[offset + 1] = scanlineBuffer[i + scanlineWidth];
      data[offset + 2] = scanlineBuffer[i + 2 * scanlineWidth];
      data[offset + 3] = scanlineBuffer[i + 3 * scanlineWidth];
      offset += 4;
    }

    scanlinesCount -= 1;
  }
}

self.onmessage = function (event) {
  self.postMessage(parseHDR(event.data));
};

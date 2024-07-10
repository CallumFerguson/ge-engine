
  var Module = typeof Module != 'undefined' ? Module : {};

  if (!Module.expectedDataFileDownloads) {
    Module.expectedDataFileDownloads = 0;
  }

  Module.expectedDataFileDownloads++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'C:/Users/Calxf/Documents/CallumDocs/git/ge-engine/cmake-build-release-emscripten/Sandbox/dist/assets.data';
      var REMOTE_PACKAGE_BASE = 'assets.data';
      if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
        Module['locateFile'] = Module['locateFilePackage'];
        err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
      }
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string') {
          require('fs').readFile(packageName, function(err, contents) {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        var xhr = new XMLHttpRequest();
        xhr.open('GET', packageName, true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
          var url = packageName;
          var size = packageSize;
          if (event.total) size = event.total;
          if (event.loaded) {
            if (!xhr.addedTotal) {
              xhr.addedTotal = true;
              if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
              Module.dataFileDownloads[url] = {
                loaded: event.loaded,
                total: size
              };
            } else {
              Module.dataFileDownloads[url].loaded = event.loaded;
            }
            var total = 0;
            var loaded = 0;
            var num = 0;
            for (var download in Module.dataFileDownloads) {
            var data = Module.dataFileDownloads[download];
              total += data.total;
              loaded += data.loaded;
              num++;
            }
            total = Math.ceil(total * Module.expectedDataFileDownloads/num);
            if (Module['setStatus']) Module['setStatus'](`Downloading data... (${loaded}/${total})`);
          } else if (!Module.dataFileDownloads) {
            if (Module['setStatus']) Module['setStatus']('Downloading data...');
          }
        };
        xhr.onerror = function(event) {
          throw new Error("NetworkError for: " + packageName);
        }
        xhr.onload = function(event) {
          if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            var packageData = xhr.response;
            callback(packageData);
          } else {
            throw new Error(xhr.statusText + " : " + xhr.responseURL);
          }
        };
        xhr.send(null);
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS() {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "assets", true, true);
Module['FS_createPath']("/assets", "engineAssets", true, true);
Module['FS_createPath']("/assets", "models", true, true);
Module['FS_createPath']("/assets/models", "BoomBox", true, true);
Module['FS_createPath']("/assets", "overcast_soil_puresky_2k", true, true);
Module['FS_createPath']("/assets", "shaders", true, true);
Module['FS_createPath']("/assets", "unpacked", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency'](`fp ${this.name}`);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency'](`fp ${that.name}`);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_C:/Users/Calxf/Documents/CallumDocs/git/ge-engine/cmake-build-release-emscripten/Sandbox/dist/assets.data');

      };
      Module['addRunDependency']('datafile_C:/Users/Calxf/Documents/CallumDocs/git/ge-engine/cmake-build-release-emscripten/Sandbox/dist/assets.data');

      if (!Module.preloadResults) Module.preloadResults = {};

      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS();
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/assets/engineAssets/BRDF.getexture", "start": 0, "end": 92345}, {"filename": "/assets/engineAssets/calculateEquirectangularIrradiance.wgsl", "start": 92345, "end": 95391}, {"filename": "/assets/engineAssets/calculateEquirectangularPrefilter.wgsl", "start": 95391, "end": 100379}, {"filename": "/assets/engineAssets/equirectangularSkybox.wgsl", "start": 100379, "end": 102540}, {"filename": "/assets/engineAssets/pbr.wgsl", "start": 102540, "end": 109874}, {"filename": "/assets/engineAssets/skybox.wgsl", "start": 109874, "end": 111246}, {"filename": "/assets/models/BoomBox/BoomBox.gemesh", "start": 111246, "end": 355338}, {"filename": "/assets/models/BoomBox/BoomBox.geprefab", "start": 355338, "end": 355971}, {"filename": "/assets/models/BoomBox/BoomBox_Mat.gematerial", "start": 355971, "end": 356258}, {"filename": "/assets/models/BoomBox/texture_0.getexture", "start": 356258, "end": 934320}, {"filename": "/assets/models/BoomBox/texture_1.getexture", "start": 934320, "end": 1752311}, {"filename": "/assets/models/BoomBox/texture_2.getexture", "start": 1752311, "end": 2038108}, {"filename": "/assets/models/BoomBox/texture_3.getexture", "start": 2038108, "end": 2143137}, {"filename": "/assets/overcast_soil_puresky_2k/overcast_soil_puresky_2k.geenvironmentmap", "start": 2143137, "end": 2143303}, {"filename": "/assets/overcast_soil_puresky_2k/overcast_soil_puresky_2k_irradiance.getexture", "start": 2143303, "end": 2173055}, {"filename": "/assets/overcast_soil_puresky_2k/overcast_soil_puresky_2k_preFilter.getexture", "start": 2173055, "end": 7433849}, {"filename": "/assets/shaders/fullscreen_color.wgsl", "start": 7433849, "end": 7434400}, {"filename": "/assets/shaders/fullscreen_texture.wgsl", "start": 7434400, "end": 7435245}, {"filename": "/assets/shaders/simple_triangle.wgsl", "start": 7435245, "end": 7435795}, {"filename": "/assets/shaders/uniform_color_triangle.wgsl", "start": 7435795, "end": 7436383}, {"filename": "/assets/shaders/unlit_color.wgsl", "start": 7436383, "end": 7437193}, {"filename": "/assets/unpacked/app-icon.png", "start": 7437193, "end": 7438611}], "remote_package_size": 7438611});

  })();

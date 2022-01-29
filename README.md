# occt-import-js

The [emscripten](https://emscripten.org) interface for [OpenCascade](https://www.opencascade.com) import functionalities. It runs entirely in the browser, and allows you to import step files and access the result in JSON format.

## How to install?

You can get occt-import-js from [npm](https://www.npmjs.com/package/occt-import-js):

```
npm install occt-import-js
```

## How to use?

The library runs in the browser and as a node.js module as well.

You will need two files from the `dist` folder: `occt-import-js.js` and `occt-import-js.wasm`. The wasm file is loaded runtime by the js file.

### Use from the browser

First, include the `occt-import-js.js` file in your website.

```html
<script type="text/javascript" src="occt-import-js.js"></script>
```

After that, download the model file, and pass them to occt-import-js.

```js
occtimportjs ().then (async function (occt) {
	let fileUrl = '../test/testfiles/simple-basic-cube/cube.stp';
	let response = await fetch (fileUrl);
	let buffer = await response.arrayBuffer ();
	let fileBuffer = new Uint8Array (buffer);
	let result = occt.ReadStepFile (fileBuffer);
	console.log (result);
});
```

### Use as a node.js module

You should require the `occt-import-js` module in your script.

```js
let fs = require ('fs');
const occtimportjs = require ('occt-import-js')();

occtimportjs.then ((occt) => {
	let fileUrl = '../test/testfiles/simple-basic-cube/cube.stp';
	let fileContent = fs.readFileSync (fileUrl);
	let result = occt.ReadStepFile (fileContent);
	console.log (result);
});
```

## How to build on Windows?

A set of batch scripts are prepared for building on Windows.

### 1. Install Prerequisites

Install [CMake](https://cmake.org) (3.6 minimum version is needed). Make sure that the cmake executable is in the PATH.

### 2. Install Emscripten SDK

Run the Emscripten setup script.

```
tools\setup_emscripten_win.bat
```

### 3. Compile the WASM library

Run the release build script.

```
tools\build_wasm_win_release.bat
```

### 4. Build the native project (optional)

If you want to debug the code, it's useful to build a native project. To do that, just use cmake to generate the project of your choice.

## How to run locally?

To run the demo and the examples locally, you have to start a web server. Run `npm install` from the root directory, then run `npm start` and visit `http://localhost:8080`.

# napi-pcsclite
[![CI](https://github.com/petrzjunior/napi-pcsclite/workflows/CI/badge.svg?branch=master&event=push)](https://github.com/petrzjunior/napi-pcsclite/actions?query=workflow%3ACI)

![Node.js 8.x](https://img.shields.io/badge/Node.js-8.x-success)
![Node.js 10.x](https://img.shields.io/badge/Node.js-10.x-success)
![Node.js 12.x](https://img.shields.io/badge/Node.js-12.x-success)
![Node.js 13.x](https://img.shields.io/badge/Node.js-13.x-success)

Simple yet powerful Node.js library for communicating with SmartCard.

:warning: **This library is under heavy development. API is subject to change every day.**

Promised-based async API is provided on **Windows, macOS and Linux** for SmartCard readers built upon the [PC/SC](https://en.wikipedia.org/wiki/PC/SC) standard. Feel the power of universal N-API no matter if your computer runs Windows [winscard](https://docs.microsoft.com/en-us/windows/win32/api/winscard/) or Unix [pcsclite](https://pcsclite.apdu.fr/).

## Building from source
:rocket: Prebuilt binaries and examples coming soon!

You will need a C++ compiler installed (gcc/clang on Linux, XCode on macOS, Visual Studio on Windows).

There are two ways to build the library:

[node-gyp](https://github.com/nodejs/node-gyp) has been with Node.js since the beginning and is de facto standard. It provides simple API and can be called by  
```console
$ npm install-gyp
```

[cmake-js](https://github.com/cmake-js/cmake-js) was written as a replacement to node-gyp, providing a full CMake experience and direct Electron compatibility. Call it with
```console
$ npm install-cmake
```

Both ways will lead to the same result - binary will be created in `build/Release/` and you can now run the example:
```console
$ npm run exmaple
```

## Compatibility
Library can be built with both node-gyp and cmake-js on any Node.js version >=6. Some tweaking of compiler paths might be needed, we officially support these combinations:

### node-gyp
Can be built on Node.js >= 6.
 
Officially supported:
Node.js | Windows            | macOS              | Linux
------- | ------------------ | ------------------ | ------------------   
6.x     | :x:                | :heavy_check_mark: | :heavy_check_mark:
7.x     | :x:                | :x:                | :x:
8.x     | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
9.x     | :x:                | :heavy_check_mark: | :heavy_check_mark:
10.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
11.x    | :x:                | :heavy_check_mark: | :heavy_check_mark:
12.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
13.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:

### cmake-js
Can be built on Node.js >= 6.

Officially supported:
Node.js | Windows            | macOS              | Linux
------- | ------------------ | ------------------ | ------------------   
6.x     | :x:                | :heavy_check_mark: | :heavy_check_mark:
7.x     | :x:                | :x:                | :x:
8.x     | :x:                | :heavy_check_mark: | :heavy_check_mark:
9.x     | :x:                | :heavy_check_mark: | :heavy_check_mark:
10.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
11.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
12.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
13.x    | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark:
 

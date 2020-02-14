# napi-pcsclite
[![CI](https://github.com/petrzjunior/napi-pcsclite/workflows/CI/badge.svg?branch=master&event=push)](https://github.com/petrzjunior/napi-pcsclite/actions?query=workflow%3ACI)

![Node.js 8.x](https://img.shields.io/badge/Node.js-8.x-success)
![Node.js 10.x](https://img.shields.io/badge/Node.js-10.x-success)
![Node.js 12.x](https://img.shields.io/badge/Node.js-12.x-success)
![Node.js 13.x](https://img.shields.io/badge/Node.js-13.x-success)

Simple yet powerful Node.js library for communicating with SmartCard.

:warning: **This library is under heavy development. API is subject to change every day.**

Provides both blocking and **event-driven API** on **Windows, macOS and Linux** for SmartCard readers built upon the [PC/SC](https://en.wikipedia.org/wiki/PC/SC) standard. Feel the power of universal N-API no matter if your computer runs Windows [winscard](https://docs.microsoft.com/en-us/windows/win32/api/winscard/) or Unix [pcsclite](https://pcsclite.apdu.fr/).

C++ binding is done through N-API which provides binaries compatible across multiple versions from **Node.js 6.x** up to **13.x**. Prebuilt binaries and examples coming soon!

## Building from source
You will need a C++ compiler installed (gcc/clang on Linux, XCode on macOS, Visual Studio on Windows).
```console
$ npm install
```
This will install all dependencies and compile the PC/SC binding with node-gyp. Binary will be created in `build/Release/` and you can now run the example:
```console
$ npm run exmaple
```

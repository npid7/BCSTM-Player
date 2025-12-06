# BCSTM-Player

<a href="https://github.com/NPI-D7/BCSTM-Player/blob/main/LICENSE"> <img height="22" src="https://img.shields.io/badge/License-GPLv3-informational.svg?style=for-the-badge" alt="License: GPLv3"></a> <img height="22" src="https://img.shields.io/github/downloads/NPI-D7/BCSTM-Player/total.svg?style=for-the-badge"> <a href="https://github.com/NPI-D7/BCSTM-Player/releases"><img height="22" src="https://img.shields.io/github/tag/NPI-D7/BCSTM-Player.svg?style=for-the-badge"/></a>&nbsp;
<img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/NPI-D7/BCSTM-Player?style=for-the-badge" height="22">

## Relevant stuff

- [Wiki](https://github.com/npid7/BCSTM-Player/wiki)

## Building

For building you need at least devkitpro with the `3ds-dev` package installed

```bash
cmake -B build
cmake --build build
```

You will find the 3dsx and cia (if makerom and bannertool are installed) inside the bin directory

## Credits

- [tobid7](https://github.com/tobid7): Lead Developer, author of palladium, ctrff
- [devkitpro](https://github.com/devkitpro): libctru, citro3d
- [cheuble](https://github.com/cheuble): Source of BCSTMV1 Decoding code (based on freeshop)
- [3dbrew](https://www.3dbrew.org/wiki/BCSTM): BCSTM Documentation

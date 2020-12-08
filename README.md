# CCHIP8

## Building

```shell
mkdir build
cd build
cmake ..
make
```
## Emulating a ch8 ROM

```shell
build/src/cchip8 rom_name.ch8
```
Example ROMs can be found here:
https://johnearnest.github.io/chip8Archive/?sort=platform#chip8

## Building documentation
```shell
mkdir build
cd build
cmake ..
make doc
```
## Testing
```shell
mkdir build
cd build
cmake ..
make
make test
```

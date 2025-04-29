# meta-kp-leopard

This layer provides boot firmware and linux image for KP Labs' Leopard DPU and Leopard EBB.


## Available machines

- ``leopard-dpu`` - Leopard DPU
- ``leopard-ebb`` - Leopard EBB


## Available Images

- ``leopard-all`` - builds all required boot firmware and software to run on Leopard EBB/DPU (depends on provided machine)


## Dependencies

This layer depends on:

```
URI: https://github.com/kplabs-pl/meta-kp-classes.git
layers: *
```

```
URI: https://git.yoctoproject.org/poky
layers: meta, meta-poky, meta-yocto-bsp
```

```
URI: https://git.openembedded.org/meta-openembedded/
layers: meta-oe, meta-networking
```

```
URI: https://git.yoctoproject.org/meta-mingw/
layers: *
```

```
URI: https://github.com/Xilinx/meta-xilinx.git
layers: meta-xilinx-bsp, meta-xilinx-core, meta-xlinx-standalone
```

```
URI: https://github.com/Xilinx/meta-xilinx-tools.git
layers: *
```
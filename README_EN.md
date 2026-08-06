# UniProton


UniProton is an ultra-low latency, mixed-criticality real-time operating system (RTOS) designed for industrial control, scaling seamlessly from low-power MCUs to high-performance multi-core CPUs.

## Setting Up the UniProton Development Environment

### 1. Source Code Download

```bash
git clone https://gitee.com/openeuler/UniProton.git
```

[Introduction to Source Code Directories](./doc/design/architecture_design.md)

### 2. Project Creation

[Hello Word example](./doc/demo_guide/getting_started.md)

### 3. Compilation

[Compilation Procedure](./doc/demo_guide/UniProton_build.md)

### 4. Running

[Raspberry Pi 4B hybrid deployment example](./doc/demo_guide/raspi4_usage.md)

[Hi3093 hybrid deployment example](./doc/demo_guide/hi3093_usage.md)

[x86_64 hybrid deployment example](./doc/demo_guide/x86_64_usage.md)

[RISC V64 QEMU running example](./doc/demo_guide/riscv64_usage.md)

### 5. Related Documents

[UniProton User Guide](https://docs.openeuler.org/zh/docs/23.09/docs/Embedded/UniProton/UniProton%E7%94%A8%E6%88%B7%E6%8C%87%E5%8D%97-%E6%A6%82%E8%BF%B0.html)

[UniProton Interface Description](https://docs.openeuler.org/zh/docs/23.09/docs/Embedded/UniProton/UniProton%E6%8E%A5%E5%8F%A3%E8%AF%B4%E6%98%8E.html)

[Introduction to the yocto Construction System](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/yocto/index.html)

[oebuild Container Guide](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/oebuild/index.html)

[Hybrid Deployment of the MCS Repository](https://gitee.com/openeuler/mcs)

[Git Commit Submission Specifications](./doc/contribute_guide/Uniproton_contribute.md)

[Multi-OS Hybrid Criticality Deployment Framework](https://pages.openeuler.openatom.cn/embedded/docs/build/html/master/features/mica/index.html)

[QEMU Simulation](./doc/qemu/)

## Function Description

- [Task](./doc/design/task.md)
- [Interrupting](./doc/design/hwi.md)
- [Event](./doc/design/event.md)
- [Queue](./doc/design/queue.md)
- [Semaphores](./doc/design/sem.md)
- [Memory management](./doc/design/mem.md)
- [Software timer](./doc/design/timer.md)
- [Exception](./doc/design/exc.md)
- [Error handling](./doc/design/err.md)
- [CPU usage](./doc/design/cpup.md)

## Disclaimer

1. The current open-source version supports the cortex_m4, armv8, x86_64, and riscv64 chips. By default, the secure compilation option of the compilation script supports only stack protection. Other options can be added as required.
2. This project is licensed under the MulanPSL2.

## How to Contribute

We welcome new contributors to the project and are pleased to provide guidance and assistance to new contributors. Before you can contribute the code, you need to sign [CLA](https://clasign.osinfra.cn/sign/Z2l0ZWUlMkZvcGVuZXVsZXI=).

For details about how to join openEuler, see [How to Contribute](https://www.openeuler.org/en/community/contribution).

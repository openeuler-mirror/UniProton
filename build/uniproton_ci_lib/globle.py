#!/usr/bin/env python3
# coding=utf-8
# The build entrance of UniProton.
# Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.

import os

build_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
home_path = os.path.dirname(build_dir)
config_dir = os.path.join(home_path,'config.xml')
kbuild_path = '%s/cmake/common/build_auxiliary_script' % home_path

cpus_ = {'all': ['clean', 'm4', 'raspi4', 'hi3093', 'atlasa1', 'kp920', 'x86_64', 'rk3568_jailhouse', 'rk3588', 'ascend310b', 'uvp', 'rv64virt', 'ds-d1s', 'e2000q'],
         'clean': ['clean'],
         'm4': ['m4'],
         'raspi4': ['raspi4'],
         'hi3093': ['hi3093'],
         'atlasa1': ['atlasa1'],
         'kp920': ['kp920'],
         'x86_64': ['x86_64'],
         'rk3568_jailhouse': ['rk3568_jailhouse'],
         'rk3588': ['rk3588'],
         'ascend310b': ['ascend310b'],
         'uvp': ['uvp'],
         'rv64virt': ['rv64virt'],
         'ds-d1s': ['ds-d1s'],
         'e2000q': ['e2000q'],
	 'milkvduol' : ['milkvduol']
         }

cpu_plat = {'m4': ['cortex'],
            'raspi4': ['armv8'],
            'hi3093': ['armv8'],
            'atlasa1': ['armv8'],
            'kp920': ['armv8'],
            'x86_64': ['x86_64'],
            'rk3568_jailhouse': ['armv8'],
            'rk3588': ['armv8'],
            'ascend310b': ['armv8'],
            'uvp': ['x86_64'],
            'rv64virt': ['riscv64'],
            'ds-d1s': ['riscv64'],
            'e2000q': ['armv8'],
	    'milkvduol' : ['riscv64']
           }

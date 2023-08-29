#!/usr/bin/env python3
# coding=utf-8
# The build entrance of UniProton.
# Copyright © Huawei Technologies Co., Ltd. 2010-2020. All rights reserved.
import os


build_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
home_path = os.path.dirname(build_dir)
config_dir = os.path.join(home_path,'config.xml')
kbuild_path = '%s/cmake/common/build_auxiliary_script' % home_path

cpus_ = {'all': ['clean', 'm4', 'raspi4', 'hi3093', 'x86_64', 'rk3568_jailhouse'],
         'clean':['clean'],
         'm4': ['m4'],
         'raspi4': ['raspi4'],
         'hi3093': ['hi3093'],
         'x86_64':['x86_64'],
         'rk3568_jailhouse':['rk3568_jailhouse']
         }

cpu_plat = {'m4': ['cortex'],
            'raspi4': ['armv8'],
            'hi3093': ['armv8'],
            'x86_64':['x86_64'],
            'rk3568_jailhouse':['armv8']
           }

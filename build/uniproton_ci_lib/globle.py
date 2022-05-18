#!/usr/bin/env python3
# coding=utf-8
# The build entrance of UniProton.
# Copyright © Huawei Technologies Co., Ltd. 2010-2020. All rights reserved.
import os


build_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
home_path = os.path.dirname(build_dir)
config_dir = os.path.join(home_path,'config.xml')
kbuild_path = '%s/cmake/common/build_auxiliary_script' % home_path

cpus_ = {'all': ['clean', 'm4'],
         'clean':['clean'],
         'm4': ['m4']
         }

cpu_plat = {'m4': ['cortex']
           }

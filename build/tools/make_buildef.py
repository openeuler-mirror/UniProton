#!/usr/bin/env python3
# coding=utf-8
# Transfer kconfig configuration file to buildef/config file.
# Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.

import os
import sys
import logging
from Kconfig2macro import do_cmd


logging.basicConfig(stream=sys.stdout, level=logging.NOTSET) 

def make_buildef(home_path, kconf_dir, choice):
    kconfig_dir = "{}/build/uniproton_config/config_{}/defconfig".format(home_path,kconf_dir)
    buildef_file = "{}/build/uniproton_config/config_{}/prt_buildef.h".format(home_path,kconf_dir)
    if choice == "CREATE":
        paras = ["-f", kconfig_dir, "-o", buildef_file]
        if do_cmd(paras, False) != 0:
            logging.info("build prt_buildef.h failed.")
            return False
        logging.info("build prt_buildef.h succeed.")
    elif choice == "EXPORT":
        paras = ["-e", "-f", kconfig_dir, "-o", buildef_file]
        if do_cmd(paras, False) != 0:
            logging.info("export prt_buildef.h failed.")
            return False
        logging.info("export prt_buildef.h succeed.")
    else:
        os.remove(buildef_file)
    return True

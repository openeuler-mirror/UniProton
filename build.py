#!/usr/bin/env python3
# coding=utf-8
# The build entrance of UniProton.
# Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.

import os
import sys
import time
import shutil
import subprocess
import platform
from sys import argv
UniProton_home = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, "%s/cmake/common/build_auxiliary_script"%UniProton_home)
from make_buildef import make_buildef
sys.path.insert(0, "%s/build/uniproton_ci_lib"%UniProton_home)
import globle
from logs import BuilderNolog, log_msg
from get_config_info import *


class Compile:

    # 根据makechoice获取config的配置的环境，compile_mode， lib_type,
    def get_config(self, cpu_type, cpu_plat):
        self.compile_mode = get_compile_mode()
        self.lib_type, self.plam_type, self.hcc_path, self.kconf_dir, self.system, self.core = get_cpu_info(cpu_type, cpu_plat, self.build_machine_platform)
        if platform.uname()[-1] == 'riscv64':
            self.hcc_path = os.path.dirname(
                subprocess.check_output(['which', 'gcc'], text=True).strip()
            )
        if not self.compile_mode and self.lib_type and self.plam_type and self.hcc_path and self.kconf_dir:
            log_msg('error', 'load config.xml env error')
            sys.exit(0)
        self.config_file_path = '%s/build/uniproton_config/config_%s'%(self.home_path, self.kconf_dir)

        self.objcopy_path = self.hcc_path

    def setCmdEnv(self):
        self.build_time_tag = time.strftime('%Y-%m-%d_%H:%M:00')
        self.log_dir = '%s/logs/%s' % (self.build_dir, self.cpu_type)
        self.log_file = '%s.log' % self.kconf_dir

    def SetCMakeEnviron(self):
        os.environ["CPU_TYPE"] = self.cpu_type
        os.environ["PLAM_TYPE"] = self.plam_type
        os.environ["LIB_TYPE"] = self.lib_type
        os.environ["COMPILE_OPTION"] = self.compile_option
        os.environ["HCC_PATH"] = self.hcc_path
        os.environ["UNIPROTON_PACKING_PATH"] = self.UniProton_packing_path
        os.environ["CONFIG_FILE_PATH"] = self.config_file_path
        os.environ["LIB_RUN_TYPE"] = self.lib_run_type
        os.environ["HOME_PATH"] = self.home_path
        os.environ["COMPILE_MODE"] = self.compile_mode
        os.environ["BUILD_MACHINE_PLATFORM"] = self.build_machine_platform
        os.environ["SYSTEM"] = self.system
        os.environ["CORE"] = self.core
        os.environ["OBJCOPY_PATH"] = self.objcopy_path
        os.environ['PATH'] = '%s:%s' % (self.hcc_path, os.getenv('PATH'))

    # 环境准备，准备执行cmake，make，makebuildfile，CmakeList需要的环境
    # 每次compile之前请调用该函数
    def prepare_env(self, cpu_type, choice):
        # makebuildfile需要的环境kconf_dir
        # cmake需要的环境cmake_env_path，home_path（cmakelist所在的路径）,home_path,
        # make cmd拼接需要的环境：home_path,UniProton_make_jx,log_dir,log_file，build_time_tag， UniProton_make_jx

        #根据cpu_type, choice从config文件中获取并初始化初始化hcc_path， plam_type, kconf_dir
        #根据输入分支获取
        #从编译镜像环境获取
        self.get_config(cpu_type, choice)
        self.setCmdEnv()
        self.SetCMakeEnviron()

    #获取编译环境是arm64还是x86，用户不感知，并将其写入环境变量。
    def getOsPlatform(self):
        self.cmake_env_path = get_tool_info('cmake', 'tool_path')

        if platform.uname()[-1] == 'aarch64':
            self.build_machine_platform = 'arm64'
        elif platform.uname()[-1] == 'x86_64':
            self.build_machine_platform = 'x86'
        else:
            self.build_machine_platform = 'riscv64'
            self.cmake_env_path = os.path.dirname(
                subprocess.check_output(['which', 'cmake'], text=True).strip()
            )

    # 获取当前编译的路径信息，配置文件信息，编译选项信息
    def __init__(self, cpu_type: str, make_option="normal", lib_run_type="FPGA", choice="ALL", make_phase="ALL",
                 UniProton_packing_path=""):
        # 当前路径信息
        self.system = ""
        self.objcopy_path = ""
        self.builder = None
        self.compile_mode = ""
        self.core = ""
        self.plam_type = ""
        self.kconf_dir = ""
        self.build_tmp_dir = ""
        self.log_dir = ""
        self.lib_type = ""
        self.hcc_path = ""
        self.log_file = ""
        self.config_file_path = ""
        self.build_time_tag = ""
        self.build_dir = globle.build_dir
        self.home_path = globle.home_path
        self.kbuild_path = globle.kbuild_path
        # 当前选项信息
        self.cpu_type = cpu_type
        self.compile_option = make_option
        self.lib_run_type = lib_run_type
        self.make_choice = choice.lower()
        self.make_phase = make_phase
        self.UniProton_packing_path = UniProton_packing_path if make_phase == "CREATE_CMAKE_FILE" else '%s/output'%self.home_path
        self.UniProton_binary_dir = os.getenv('RPROTON_BINARY_DIR')
        self.UniProton_install_file_option = os.getenv('RPROTON_INSTALL_FILE_OPTION')
        self.UniProton_make_jx = 'VERBOSE=1' if self.UniProton_install_file_option == 'SUPER_BUILD' else 'VERBOSE=1 -j' + str(os.cpu_count())
        # 当前编译平台信息
        self.getOsPlatform()

    #调用cmake生成Makefile文件，需要
    def CMake(self):
        if self.UniProton_binary_dir:
            self.build_tmp_dir = '%s/output/tmp/%s' % (self.UniProton_binary_dir, self.kconf_dir)
        else:
            self.build_tmp_dir = '%s/output/tmp/%s' % (self.build_dir, self.kconf_dir)
        os.environ['BUILD_TMP_DIR'] = self.build_tmp_dir

        if not os.path.exists(self.build_tmp_dir):
            os.makedirs(self.build_tmp_dir)
        if not os.path.exists(self.log_dir):
            os.makedirs(self.log_dir)

        log_msg('info', 'BUILD_TIME_TAG %s' % self.build_time_tag)
        self.builder = BuilderNolog(os.path.join(self.log_dir, self.log_file))
        if self.make_phase in ['CREATE_CMAKE_FILE', 'ALL']:
            real_path = os.path.realpath(self.build_tmp_dir)
            if os.path.exists(real_path):
                shutil.rmtree(real_path)
            os.makedirs(self.build_tmp_dir)
            #拼接cmake命令
            if self.compile_option == 'fortify':
                cmd = '%s/cmake %s -DCMAKE_TOOLCHAIN_FILE=%s/cmake/tool_chain/uniproton_tool_chain.cmake ' \
                      '-DCMAKE_C_COMPILER_LAUNCHER="sourceanalyzer;-b;%sproject" ' \
                      '-DCMAKE_INSTALL_PREFIX=%s &> %s/%s' % (
                self.cmake_env_path, self.home_path, self.home_path, self.cpu_type,
                self.UniProton_packing_path, self.log_dir, self.log_file)
            elif self.compile_option == 'hllt':
                cmd = '%s/cmake %s -DCMAKE_TOOLCHAIN_FILE=%s/cmake/tool_chain/uniproton_tool_chain.cmake ' \
                      '-DCMAKE_C_COMPILER_LAUNCHER="lltwrapper" -DCMAKE_INSTALL_PREFIX=%s &> %s/%s' % (
                self.cmake_env_path, self.home_path, self.home_path, self.UniProton_packing_path, self.log_dir, self.log_file)
            else:
                cmd = '%s/cmake %s -DCMAKE_TOOLCHAIN_FILE=%s/cmake/tool_chain/uniproton_tool_chain.cmake ' \
                      '-DCMAKE_INSTALL_PREFIX=%s &> %s/%s' % (
                self.cmake_env_path, self.home_path, self.home_path, self.UniProton_packing_path, self.log_dir, self.log_file)
            #执行cmake命令
            if self.builder.run(cmd, cwd=self.build_tmp_dir, env=None):
                log_msg('error', 'generate makefile failed!')
                return False

        log_msg('info', 'generate makefile succeed.')
        return True

    def UniProton_clean(self):
        for foldername,subfoldernames,filenames in os.walk(self.build_dir):
            for subfoldername in subfoldernames:
                if subfoldername in ['logs','output','__pycache__']:
                    folder_path = os.path.join(foldername,subfoldername)
                    shutil.rmtree(os.path.relpath(folder_path))
            for filename in filenames:
                if filename == 'prt_buildef.h':
                    file_dir = os.path.join(foldername,filename)
                    os.remove(os.path.relpath(file_dir))
        if os.path.exists('%s/cmake/common/build_auxiliary_script/__pycache__'%self.home_path):
            shutil.rmtree('%s/cmake/common/build_auxiliary_script/__pycache__'%self.home_path)
        if os.path.exists('%s/output'%self.home_path):
            shutil.rmtree('%s/output'%self.home_path)
        if os.path.exists('%s/tools/SRE/x86-win32/sp_makepatch/makepatch'%self.home_path):
            os.remove('%s/tools/SRE/x86-win32/sp_makepatch/makepatch'%self.home_path)
        if os.path.exists('%s/build/prepare/__pycache__'%self.home_path):
            shutil.rmtree('%s/build/prepare/__pycache__'%self.home_path)
        return True


    def make(self):
        if self.make_phase in ['EXECUTING_MAKE', 'ALL']:
            self.builder.run('make clean', cwd=self.build_tmp_dir, env=None)
            tmp = sys.argv
            if self.builder.run(
                    'make all %s &>> %s/%s' % (
                    self.UniProton_make_jx, self.log_dir, self.log_file), cwd=self.build_tmp_dir, env=None):
                log_msg('error', 'make %s %s  failed!' % (self.cpu_type, self.plam_type))
                return False
            sys.argv = tmp
            if self.compile_option in ['normal', 'coverity', 'single']:
                if self.builder.run('make install %s &>> %s/%s' % (self.UniProton_make_jx, self.log_dir, self.log_file), cwd=self.build_tmp_dir, env=None):
                    log_msg('error', 'make install failed!')
                    return False
            if os.path.exists('%s/%s' % (self.log_dir, self.log_file)):
                self.builder.log_format()
        
        log_msg('info', 'make %s %s succeed.' % (self.cpu_type, self.plam_type))
        return True

    def SdkCompaile(self)->bool:
        # 判断该环境中是否需要编译
        if self.hcc_path == 'None':
            return True

        self.MakeBuildef()
        if self.CMake() and self.make():
            log_msg('info', 'make %s %s lib succeed!' % (self.cpu_type, self.make_choice))
            return True
        else:
            log_msg('info', 'make %s %s lib failed!' % (self.cpu_type, self.make_choice))
            return False

    # 对外函数，调用后根据类初始化时的值进行编译
    def UniProtonCompile(self):
        #清除UniProton缓存
        if self.cpu_type == 'clean':
            log_msg('info', 'UniProton clean')
            return self.UniProton_clean()
        # 根据cpu的编译平台配置相应的编译环境。
        if self.make_choice == "all":
            for make_choice in globle.cpu_plat[self.cpu_type]:
                self.prepare_env(self.cpu_type, make_choice)
                if not self.SdkCompaile():
                    return False
        else:
            self.prepare_env(self.cpu_type, self.make_choice)
            if not self.SdkCompaile():
                return False
        return True

    def MakeBuildef(self):

        if not make_buildef(globle.home_path,self.kconf_dir,"CREATE"):
            sys.exit(1)
        log_msg('info', 'make_buildef_file.sh %s successfully.' % self.kconf_dir)

# argv[1]: cpu_plat 表示要编译的平台：
# argv[2]: compile_option 控制编译选项，调用不同的cmake参数，目前只有normal coverity hllt fortify single(是否编译安全c，组件化独立构建需求)
# argv[3]: lib_run_type lib库要跑的平台 faga sim等
# argv[4]: make_choice 
# argv[5]: make_phase 全量构建选项
# argv[6]: UniProton_packing_path lib库的安装位置
if __name__ == "__main__":
    default_para = ("all", "normal", "FPGA", "ALL", "ALL", "")
    if len(argv) == 1:
        para = [default_para[i] for i in range(0, len(default_para))]
    else:
        para = [argv[i+1] if i < len(argv) - 1 else default_para[i] for i in range(0,len(default_para))]

    cur_cpu_type = para[0].lower()
    cur_compile_option = para[1].lower()
    cur_lib_run_type = para[2]
    cur_make_choice = para[3]
    cur_make_phase = para[4]
    cur_UniProton_packing_path = para[5]
    for plat in globle.cpus_[cur_cpu_type]:
        UniProton_build = Compile(plat, cur_compile_option, cur_lib_run_type, cur_make_choice, cur_make_phase, cur_UniProton_packing_path)
        if not UniProton_build.UniProtonCompile():
            sys.exit(1)
    sys.exit(0)


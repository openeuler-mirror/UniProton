#!/usr/bin/env python3
# coding=utf-8
# The build entrance of UniProton.
# Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
import xml.dom.minidom
import sys
import logging
import globle
from logs import BuilderNolog, log_msg


logging.basicConfig(stream=sys.stdout, level=logging.NOTSET) 
logging.info(globle.config_dir)
config_tree = xml.dom.minidom.parse(globle.config_dir)
EXCEPTION_LIST = (AssertionError, AttributeError, IOError,
                  ImportError, IndentationError,
                  IndexError, KeyError, NameError,
                  SyntaxError, TypeError, ValueError)

def getNodeValue(node, index = 0):
    return node.childNodes[index].nodeValue

def getXmlNode(node, name):
    return node.getElementsByTagName(name)

def getNodeAtrrValue(node, attrname):
    return node.getAttribute(attrname)

def getChildNodeValueByNodeName(farthe_node, nodename, index = 0):
    return farthe_node.getElementsByTagName(nodename)[0].childNodes[index].nodeValue

def getChildNodeByNodeAtrr(farthe_node, node_name, atrr_name, strr_vlue):
    for node in farthe_node.getElementsByTagName(node_name):
        if node.getAttribute(atrr_name).startswith(strr_vlue):
            return node
    return False

# 根据cputype获取hcc_path, kconf_dir, plam_type
def get_cpu_info(cpu_type, cpu_plat, os_plat):
    nameNode = "project"
    cpu_list = config_tree.documentElement.getElementsByTagName(nameNode)
    hcc_path, kconf_dir, plam_type, lib_type = "", "", "", ""
    try:
        for cpu in cpu_list:
            if getNodeAtrrValue(cpu, "cpu_type") == cpu_type:
                lib_type = getChildNodeValueByNodeName(cpu, "lib_type")
                plam_type = getChildNodeValueByNodeName(getChildNodeByNodeAtrr(cpu, "platform", "plat_name", cpu_plat), "name")
                hcc_path = getChildNodeValueByNodeName(getChildNodeByNodeAtrr(cpu, "platform", "plat_name", cpu_plat), "compile_path_{}".format(os_plat))
                kconf_dir = getChildNodeValueByNodeName(getChildNodeByNodeAtrr(cpu, "platform", "plat_name", cpu_plat), "kconf_dir")
    except EXCEPTION_LIST:
        log_msg('error', "get cpu {} config info error:{} {}".format(cpu_type, cpu_plat, os_plat))
        sys.exit(0)
    
    system, core = "None", "None"
    try:
        for cpu in cpu_list:
            if getNodeAtrrValue(cpu, "cpu_type") == cpu_type:
                core = getChildNodeValueByNodeName(getChildNodeByNodeAtrr(cpu, "platform", "plat_name", cpu_plat), "core")
                system = getChildNodeValueByNodeName(cpu, "system")
    except EXCEPTION_LIST:
        system = "None"
    return lib_type, plam_type, hcc_path, kconf_dir, system, core

def get_tool_info(tool_name, node_name):
    tool_list = config_tree.documentElement.getElementsByTagName("tool")
    try:
        for tool in tool_list:
            if getNodeAtrrValue(tool, "tool_name") == tool_name:
                return getChildNodeValueByNodeName(tool, node_name)
    except EXCEPTION_LIST:
        log_msg("error", "get tool config info error:{} {}".format(tool_name, node_name))
        sys.exit(0)
    return False


def get_compile_mode():
    nameNode = "UniProton_compile_mode"
    cpu_list = config_tree.documentElement.getElementsByTagName(nameNode)
    compile_mode = getNodeValue(cpu_list[0])
    return compile_mode

#!/usr/bin/env python3
# coding=utf-8
# 通过readelf获取elf的程序头信息并填充到指定的g_phdrs数组中

import os
import struct
import sys,getopt

PI_TYPE = 0       # Type
PI_OFFSET = 1     # Offset
PI_VIRT_ADDR = 2  # VirtAddr
PI_PHYS_ADDR = 3  # PhysAddr
PI_FILE_SIZ = 4   # FileSiz
PI_MEM_SIZ = 5    # MemSiz
PI_FLG = 6        # Flg
PI_ALIGN = 7      # Align

PT_LOAD = 1                   # Loadable program segment
PT_NOTE = 4                   # Auxiliary information
PT_GNU_EH_FRAME = 0x6474e550  # GCC .eh_frame_hdr segment
PT_GNU_STACK = 0x6474e551     # Indicates stack executability
PT_GNU_PROPERTY = 0x6474e553  # GNU property

PF_X = (1 << 0)    # Segment is executable
PF_W = (1 << 1)    # Segment is writable
PF_R = (1 << 2)    # Segment is readable

class BinHelper:
    p_flags_map = {'R':PF_R, 'W':PF_W, 'E':PF_X}
    p_flags_str_map = {PF_R:'PF_R', PF_W:'PF_W', PF_X:'PF_X'}
    pt_type_map = {
        'LOAD':PT_LOAD,
        'NOTE':PT_NOTE,
        'GNU_EH_FRAME':PT_GNU_EH_FRAME,
        'GNU_STACK':PT_GNU_STACK,
        'GNU_PROPERTY':PT_GNU_PROPERTY
        }
    pt_type_str_map = {
        PT_LOAD:'PT_LOAD',
        PT_NOTE:'PT_NOTE',
        PT_GNU_EH_FRAME:'PT_GNU_EH_FRAME',
        PT_GNU_STACK:'PT_GNU_STACK',
        PT_GNU_PROPERTY:'PT_GNU_PROPERTY'
        }
    def extract(self, str):
        fidx = 0
        fields = str.split(" ")
        p_flags = 0
        output = []

        for i in range(len(fields)):
            fval = fields[i]
            if len(fval) == 0:
                continue
            if fidx == 0:
                if (fval not in self.pt_type_map):
                    print("ignore segment type {0}".format(fval))
                    return output
                output.append(self.pt_type_map[fval])
            elif fidx >= 6:
                if fval[0:2] == '0x':
                    output.append(p_flags)
                    output.append(int(fval, base = 16))
                    break
                elif len(fval) <= 3:
                    for fch in fval:
                        if (fch in self.p_flags_map):
                            p_flags |= self.p_flags_map[fch]
            else:
                output.append(int(fval, base = 16))
            fidx = fidx + 1
        return output

    def get_addr(self, symbol):
        # format: 192: 0000000f02a01800   336 OBJECT  LOCAL  DEFAULT    3 g_phdrs
        strs = os.popen("{0} -s {1} | grep {2}".format(self.readelf_path, self.path, symbol)).readlines()
        if len(strs) == 0:
            print("phdrs array not found")
            sys.exit(-1)
        if len(strs) > 1:
            print("warn ")
        return strs[0].strip().split(" ")[1]

    def parse_phdrs(self):
        lines = os.popen("{0} -Wl {1}".format(self.readelf_path, self.path)).readlines()
        state = 0
        phdrs = []
        for l in lines:
            if state == 0:
                if (l.find('Entry point') >= 0):
                    entry_point = int(l.split(" ")[2], base = 16)
                    state = 1
            elif state == 1:
                if l.find('Type') >= 0:
                    state = 2
            elif state == 2:
                if len(l.strip()) == 0:
                    break
                else:
                    info = self.extract(l)
                    if len(info) == 0:
                        continue
                    phdrs.append(info)
        return entry_point, phdrs

    def gen_file(self, offset, phdrs, nocopy):
        dstpath = self.path
        if not nocopy:
            basename = os.path.basename(self.path)
            dirname = os.path.dirname(self.path)
            filename = basename.split(".")[0]
            suffix = basename.split(".")[1]
            dstpath = "{0}/{1}_cpp.{2}".format(dirname, filename, suffix)
            ret = os.system('cp {0} {1}'.format(self.path, dstpath))
            if ret != 0:
                print("copy failed")
                sys.exit(ret)
        dstfile = open(dstpath,'rb+')
        dstfile.seek(offset, 0)
        for phdr in phdrs:
            dstfile.write(struct.pack('2I6L',
                        phdr[PI_TYPE],
                        phdr[PI_FLG],
                        phdr[PI_OFFSET],
                        phdr[PI_VIRT_ADDR],
                        phdr[PI_PHYS_ADDR],
                        phdr[PI_FILE_SIZ],
                        phdr[PI_MEM_SIZ],
                        phdr[PI_ALIGN]))
        print('update {0} finish, offset: {1}'.format(dstpath, offset))
        dstfile.close()

    def fill_phdrs(self, nocopy):
        phdrs = self.parse_phdrs()[1]
        phdrs_addr = int(self.get_addr(self.phdrsym), base = 16)

        offset, vaddr = self.find_segment(phdrs_addr, phdrs)
        if offset == -1:
            print("invald addr/phdrs")
            sys.exit(-1)

        self.gen_file(phdrs_addr + offset - vaddr, phdrs, nocopy)

    def find_segment(self, pc, phdrs):
        for phdr in phdrs:
            if (pc >= phdr[PI_VIRT_ADDR] and pc < phdr[PI_VIRT_ADDR] + phdr[PI_FILE_SIZ]):
                return (phdr[PI_OFFSET], phdr[PI_VIRT_ADDR])
        return -1, -1

    def get_flags_str(self, flags):
        list = []
        for f in self.p_flags_map.values():
            if f & flags:
                list.append(self.p_flags_str_map[f])
        if len(list) == 0:
            return '0'
        return '|'.join(list)

    def print_phdrs(self):
        phdrs = self.parse_phdrs()[1]
        pattern = '''\n\t.p_type = {0},\n\t.p_flags = {1},\n\t.p_offset = 0x{2:x},\n\t.p_vaddr = 0x{3:x},
\t.p_paddr = 0x{4:x},\n\t.p_filesz = 0x{5:x},\n\t.p_memsz = 0x{6:x},\n\t.p_align = 0x{7:x}'''
        for phdr in phdrs:
            print('{',pattern.format(
                    self.pt_type_str_map[phdr[PI_TYPE]],
                    self.get_flags_str(phdr[PI_FLG]),
                    phdr[PI_OFFSET],
                    phdr[PI_VIRT_ADDR],
                    phdr[PI_PHYS_ADDR],
                    phdr[PI_FILE_SIZ],
                    phdr[PI_MEM_SIZ],
                    phdr[PI_ALIGN]),'\n},')

    def __init__(self, path, readelf_path, phdrsym):
        self.path = path
        self.readelf_path = readelf_path
        self.phdrsym = phdrsym

def do_cmd(argv):
    inputfile = ''
    debugmode = False
    nocopy = False
    try:
        opts, args = getopt.getopt(argv, "hf:d", ["nocopy"])
    except getopt.GetoptError:
        print("bin_helper.py -f <inputfile>")
        sys.exit(-1)
    for opt, arg in opts:
        if opt == "-h":
            print("bin_helper.py -f <inputfile>")
            sys.exit()
        elif opt == "-f":
            inputfile = arg
        elif opt == "-d":
            debugmode = True
        elif opt == "--nocopy":
            nocopy = True
    helper = BinHelper(inputfile, 'readelf', 'g_phdrs')
    if debugmode:
        helper.print_phdrs()
    helper.fill_phdrs(nocopy)

if __name__ == "__main__":
    do_cmd(sys.argv[1:])

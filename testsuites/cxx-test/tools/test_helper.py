#!/usr/bin/env python3
# coding=utf-8
import os
import sys, getopt
import yaml
import re
class TestHelper:
    def traverse_dir(self, path, callback):
        if os.path.isfile(path):
            callback(path)
            return
        for root, dirs, files in os.walk(path):
            for file in files:
                callback(root + "/"+file)

    def removesuffix(self, str, suffix):
        if suffix and str.endswith(suffix):
            return str[:-len(suffix)]
        return str

    def removeprefix(self, str, prefix):
        if prefix and str.startswith(prefix):
            return str[len(prefix):]
        return str

    def gen_fun_name(self, path):
        fun_name = self.removesuffix(self.removeprefix(path[len(self.testcases_base):], "/"), ".cc")
        for key in {"/","-","+"}:
            fun_name = fun_name.replace(key, "_")
        fun_name = "test_" + fun_name
        return fun_name

    def update_fun(self, path):
        if (not path.endswith(".cc") or self.contains(path)):
            return
        fun_name = self.gen_fun_name(path)
        cmd = "grep '^main()' {1}".format(fun_name, path)
        ret = os.popen(cmd).readlines()
        if len(ret) > 0:
            #gnu c style
            cmd = "sed -i 's/^main()/{0}()/g' {1}".format(fun_name, path)
        else:
            cmd = "sed -i 's/int\ main()/int\ {0}()/g' {1}".format(fun_name, path)
        os.popen(cmd).close()

    def update(self):
        for path in self.testcases:
            self.traverse_dir(path, self.update_fun)

    test_list = []
    src_list = []

    def contains(self, path):
        for prefix in self.excludes:
            if (path[0:len(prefix)] == prefix):
                return True
        return False
    def collect_fun(self, path):
        if (not path.endswith(".cc") or self.contains(path)):
            return
        fun_name = self.gen_fun_name(path)
        cmd = "grep {0} {1}".format(fun_name, path)
        ret = os.popen(cmd).readlines()
        if len(ret) == 0: #ignore
            return
        self.test_list.append(fun_name)
        self.src_list.append(path)

    def collect(self):
        self.test_list.clear()
        self.src_list.clear()
        for path in self.testcases:
            self.traverse_dir(path, self.collect_fun)

        macros = set()
        includes = []
        ts = "    "
        with open(self.run_test_tmpl, "r") as src, open(self.run_test, "w+") as dst:
            testwrapper = "FUNCTION_TEST"
            for line in src.readlines():
                if line.strip().startswith("#define"):
                    ret = re.findall(r"#define\s+(.+?)\(", line.strip())
                    if len(ret) > 0:
                        macros.add(ret)
                elif line.strip().startswith("#include"):
                    includes.append(line)

            if not testwrapper in macros:
                dst.writelines("#define " + testwrapper + "(f) do{ f(); printf(#f \" finish\\n\"); }while(0)\n")
            src.seek(0, 0)
            for line in src.readlines():
                if line.strip().startswith("#FUNCTION_DECALER"):
                    for f in self.test_list:
                        dst.writelines("extern int {0}();\n".format(f))
                elif line.strip().startswith("#FUNCTION_CALL"):
                    for f in self.test_list:
                        dst.writelines("{0}{1}({2});\n".format(ts, testwrapper, f))
                else:
                    dst.writelines(line)
        
        print("\n".join(self.src_list))

    def get_abs_path(self, path, base):
        if path.startswith("/"):
            return path
        return base + "/" + path
    
    def do_cmd(self, argv):
        cur_path = os.path.abspath(__file__)
        dirname = os.path.dirname(cur_path)
        config_path = dirname + "/config.yaml"

        try:
            opts, args = getopt.getopt(argv, "f:",)
        except getopt.GetoptError:
            print("update.py -f <config_path>")
            sys.exit(-1)
        for opt, arg in opts:
            if opt == "-f":
                config_path = arg
            else:
                print("update.py -f <config_path>")
                sys.exit(-1)
        with open(config_path, "r") as file:
            data = yaml.load(file, Loader=yaml.FullLoader)
            self.testcases = [] 
            self.excludes = []
            for case in data['testcases']:
                path = case
                if not case.startswith("/"):
                    path = dirname + "/" + path
                if not os.path.exists(path):
                    continue    
                self.testcases.append(os.path.realpath(path))
            self.testcases_base = os.path.realpath(self.get_abs_path(data['testcases_base'], dirname))
            self.run_test_tmpl = self.get_abs_path(data['run_test_tmpl'], dirname)
            self.run_test = self.get_abs_path(data['run_test'], dirname)
            for prefix in data["exclude_prefix"]:
                self.excludes.append(os.path.realpath(self.get_abs_path(prefix, dirname)))
        self.collect()
        
if __name__ == "__main__":
    TestHelper().do_cmd(sys.argv[1:])
    pass

#!/usr/bin/python3
# coding=utf-8

import os

interfaceList = [ "pthread_attr_init",
                  "pthread_attr_destroy",
                  "pthread_attr_setstackaddr",
                  "pthread_attr_getstackaddr",
                  "pthread_attr_getstacksize",
                  "pthread_attr_setstacksize",
                  "pthread_attr_getinheritsched",
                  "pthread_attr_setinheritsched",
                  "pthread_attr_getschedpolicy",
                  "pthread_attr_setschedpolicy",
                  "pthread_attr_getdetachstate",
                  "pthread_attr_setdetachstate",
                  "pthread_create",
                  "pthread_cancel",
                  "pthread_testcancel",
                  "pthread_setcancelstate",
                  "pthread_setcanceltype",
                  "pthread_exit",
                  "pthread_cleanup_push",
                  "pthread_cleanup_pop",
                  "pthread_setschedprio",
                  "pthread_self",
                  "pthread_equal",
                  "sched_yield",
                  "sched_get_priority_max",
                  "sched_get_priority_min",
                  "pthread_join",
                  "pthread_detach",
                  "pthread_key_create",
                  "pthread_setspecific",
                  "pthread_getspecific",
                  "pthread_key_delete",
                  "sem_init",
                  "sem_destroy",
                  "sem_open",
                  "sem_close",
                  "sem_wait",
                  "sem_trywait",
                  "sem_timedwait",
                  "sem_post",
                  "sem_getvalue",
                  "pthread_mutexattr_init",
                  "pthread_mutexattr_destroy",
                  "pthread_mutexattr_settype",
                  "pthread_mutexattr_gettype",
                  "pthread_mutexattr_setprioceiling",
                  "pthread_mutexattr_getprioceiling",
                  "pthread_mutexattr_setprotocol",
                  "pthread_mutexattr_getprotocol",
                  "pthread_mutex_init",
                  "pthread_mutex_destroy",
                  "pthread_mutex_lock",
                  "pthread_mutex_trylock",
                  "pthread_mutex_timedlock",
                  "pthread_mutex_unlock",
                  "pthread_rwlock_init",
                  "pthread_rwlock_destroy",
                  "pthread_rwlock_rdlock",
                  "pthread_rwlock_tryrdlock",
                  "pthread_rwlock_timedrdlock",
                  "pthread_rwlock_wrlock",
                  "pthread_rwlock_trywrlock",
                  "pthread_rwlock_timedwrlock",
                  "pthread_rwlock_unlock",
                  "clock_gettime",
                  "clock_settime",
                  "clock_getres",
                  "nanosleep",
                  "sleep",
                  "timer_create",
                  "timer_delete",
                  "timer_settime",
                  "timer_gettime",
                  "malloc",
                  "free" ]

def get_all_c_file(interfacePath):
    ret_files = []
    allFiles = os.listdir(interfacePath)
    for oneFile in allFiles:
        file_full_path = os.path.join(interfacePath, oneFile)
        if os.path.isdir(file_full_path):
            tmp_files = get_all_c_file(file_full_path)
            if len(tmp_files) == 0:
                continue
            ret_files.extend(tmp_files)
        fileSplit = os.path.splitext(oneFile)
        if fileSplit[1] != ".c" and fileSplit[1] != ".h":
            continue
        if oneFile[0] not in ["0","1","2","3","4","5","6","7","8","9"]:
            print(file_full_path)
            with open(file_full_path, "r", encoding="utf-8") as f:
                file_data=""
                for line in f:
                    if "exit(" in line:
                        index = line.find("exit(")
                        if index < 0:
                            file_data += line
                            continue
                        if index > 0 and line[index - 1] not in [" ", "\t"]:
                            file_data += line
                            continue 
                        print(line)
                        index = line.find(")")
                        line = line[:index] + " " + line[(index + 1):]
                        line = line.replace("exit(", "return ")
                        print(line)
                    file_data += line
            with open(file_full_path, "w", encoding="utf-8") as f:
                f.write(file_data)
            continue
        ret_files.append(file_full_path)
    return ret_files
        

def main():
    for oneInterface in interfaceList:
        interfacePath = os.path.join("..", "conformance","interfaces", oneInterface)
        if not os.path.exists(interfacePath) or not os.path.isdir(interfacePath) :
            continue
        allFiles = get_all_c_file(interfacePath)
        for oneFile in allFiles:
            file_data = ""
            fileSplit = os.path.splitext(os.path.basename(oneFile))
            # print("%s_%s"%(oneInterface, fileSplit[0].replace("-","_")))
            with open(oneFile, "r", encoding="utf-8") as f:
                for line in f:
                    if "int main(" in line or "int main (" in line:
                        newName = "%s_%s"%(oneInterface, fileSplit[0].replace("-","_"))
                        line = line.replace("main", newName)
                        print(newName)
                    if "exit(" in line:
                        index = line.find("exit(")
                        if index < 0:
                            file_data += line
                            continue
                        if index > 0 and (line[index - 1] not in [" ", "\t"]):
                            file_data += line
                            continue 
                        print(line)
                        index = line.find(")")
                        line = line[:index] + " " + line[(index + 1):]
                        line = line.replace("exit(", "return ")
                        print(line)
                    file_data += line
            with open(oneFile, "w", encoding="utf-8") as f:
                f.write(file_data)

if __name__ == "__main__":
    main()
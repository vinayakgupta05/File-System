#!/usr/bin/env python

import os
import filecmp
import sys
if __name__ == '__main__':
    op = ">&"
    print( "############################")
    print( "# Grading Script Started #")
    print( "############################")
    print( "############################")
    os.system('cp cmpsc473-filesys.c copy.c')
    f = open("cmpsc473-filesys.c", "r")
    contents = f.readlines()
    f.close()
    index = contents.index("int fileOpen( char *name, unsigned int flags )\n")
    index = contents.index("\tfd = fsAddProcFile( fs->proc, file );\n",index)
    with open('testing-xattr.c', 'r') as file:
        data = file.read().replace('\n', '')
        contents.insert(index+1,data)
    f = open("cmpsc473-filesys.c", "w")
    contents = "".join(contents)
    f.write(contents)
    f.close()
    points = 0
    cmd3_get = 0
    cmd3_set = 0
    cmd4_get = 0
    cmd4_set = 0
    cmd5_get = 0
    cmd5_set = 0
    l_f0_n=[]
    l_f0_v=[]

    l_f1_n=[]
    l_f1_v=[]

    l_f2_n=[]
    l_f2_v=[]
    error3 = 0
    e3 = 0
    error4 = 0
    e4 = 0
    e5 = 0
    error5 = 0
    os.system('rm test-out*')
    os.system('make clean')

    os.system('rm test_fs')
    os.system('make')
    run1 = "./cmpsc473-p4 test_fs cmd1 " + op + " test-out1.txt"
    run2 = "./cmpsc473-p4 test_fs cmd2 " + op + " test-out2.txt"
    run3 = "./cmpsc473-p4 test_fs cmd3 " + op + " test-out3.txt"
    run4 = "./cmpsc473-p4-test test_fs cmd3.test " + op + " test-set-out3.txt"
    run5 = "./cmpsc473-p4 test_fs cmd3.test " + op + " test-st-set-out3.txt"
    run6 = "./cmpsc473-p4 test_fs cmd4 " + op + " test-out4.txt"
    run7 = "./cmpsc473-p4-test test_fs cmd4.test " + op + " test-set-out4.txt"
    run8 = "./cmpsc473-p4 test_fs cmd4.test " + op + " test-st-set-out4.txt"
    run9 = "./cmpsc473-p4 test_fs cmd5 " + op + " test-out5.txt"
    run10 = "./cmpsc473-p4 test_fs cmd5.test " + op + " test-xattr-out5.txt"
    run11 = "./cmpsc473-p4-test test_fs cmd5.test " + op + " test-set-out5.txt"
    run12 = "./cmpsc473-p4 test_fs cmd5.test " + op + " test-st-set-out5.txt"
    os.system(run1)
    os.system(run2)
    os.system(run3)
    os.system(run4)
    os.system(run5)
    os.system(run6)
    os.system(run7)
    os.system(run8)
    os.system(run9)
    os.system(run10)
    os.system(run11)
    os.system(run12)
    # os.system('./cmpsc473-p4 test_fs cmd2 %s test-out2.txt')
    # os.system('./cmpsc473-p4 test_fs cmd3 >& test-out3.txt')
    # os.system('./cmpsc473-p4-test test_fs cmd3.test >& test-set-out3.txt')
    # os.system('./cmpsc473-p4 test_fs cmd4 >& test-out4.txt')
    # os.system('./cmpsc473-p4-test test_fs cmd4.test >& test-set-out4.txt')
    # os.system('./cmpsc473-p4 test_fs cmd5 >& test-out5.txt')
    # os.system('./cmpsc473-p4 test_fs cmd5.test >& test-xattr-out5.txt')
    # os.system('./cmpsc473-p4-test test_fs cmd5.test >& test-set-out5.txt')
#luper =f0 super = f1 duper = f2

    os.system('cp copy.c cmpsc473-filesys.c')
    f = open("cmd3", "r")
    for i,s in enumerate(f):
        s = [x.strip() for x in s.split(',')]
        if s[0]=="setxattr 0":
            l_f0_n.append(s[1])
            l_f0_v.append(s[2])







    print( "####################################################   cmd3   ####################################################")
    print( "--------------------------------------------------")
    print( "SETXATTR PART:")
    print( "--------------------------------------------------")
    cmd3_st_set = 0
    cmd3_our_set = 0
    f = open("test-st-set-out3.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR" :
            continue
        if s[0:15] =="getxattr: error":
            continue
        s = s.split('=')
        for name in l_f0_n:
            if s[0] == name:
                if s[1] == l_f0_v[l_f0_n.index(name)]:
                    cmd3_st_set = cmd3_st_set + 2

    f = open("test-set-out3.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR" :
            continue
        if s[0:15] =="getxattr: error":
            continue
        s = s.split('=')
        for name in l_f0_n:
            if s[0] == name:
                if s[1] == l_f0_v[l_f0_n.index(name)]:
                    cmd3_our_set = cmd3_our_set + 2

    if cmd3_st_set > cmd3_our_set:
        f = open("test-st-set-out3.txt", "r")
    else:
        f = open("test-set-out3.txt", "r")

    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR" :
            continue
        if s[0:15] =="getxattr: error":
            print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue
        s = s.split('=')
        for name in l_f0_n:
            if s[0] == name:
                if s[1] == l_f0_v[l_f0_n.index(name)]:
                    cmd3_set = cmd3_set + 2
                    print ("[CORRECT(POINTS:+2):] \t ATTR NAME: %s \t VALUE: %s" %(s[0],s[1]))
                else:
                    print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[0]))

    print( "------------------------------------------------------------------------------------")
    print ("[TOTAL POINTS FOR %s] : [%dpts]/[%dpts]" %("SETXATTR PART", cmd3_set, 8))
    print( "------------------------------------------------------------------------------------")


    print( "--------------------------------------------------")
    print( "GETXATTR PART:")
    print( "--------------------------------------------------")
    f = open("test-out3.txt", "r")

    for i,s in enumerate(f):
        flag = 0
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR":
            continue
        if s[0:15] =="getxattr: error":
            print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            for name in l_f0_n:
                if s[0] == name:
                    if s[1] == l_f0_v[l_f0_n.index(name)]:
                        cmd3_get = cmd3_get + 1
                        print ("[CORRECT(POINTS:+1):] \t ATTR NAME: %s \t VALUE: %s" %(s[0],s[1]))
                    else:
                        print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[0]))

    print( "------------------------------------------------------------------------------------")
    print ("[TOTAL POINTS FOR %s] : [%dpts]/[%dpts]" %("GETXATTR PART", cmd3_get, 3))
    print( "------------------------------------------------------------------------------------")


    #print( "#####################################################################################################################")
    f = open("cmd4", "r")
    for i,s in enumerate(f):
        s = [x.strip() for x in s.split(',')]
        add_to_list = 1
        change_list = 0
        if s[0]=="setxattr 0":#duper
            if int(s[3])==1:#not previously existing
                #check if in list
                #print("at cmd4 setxattr 0 flag 1")
                add_to_list = 1
                for name in l_f2_n:
                    if name == s[1]:
                         add_to_list = 0
                if add_to_list == 0:
                    error4 = error4 + 1
                if add_to_list == 1:
                    l_f2_n.append(s[1])
                    l_f2_v.append(s[2])
                         #break
            if int(s[3])==2:#should be previously existing
                #check if in list
                #print("at cmd4 setxattr 0 flag 2")
                change_list = 0
                for name in l_f2_n:
                    if name == s[1]:
                         change_list = 1
                         l_f2_v[l_f2_n.index(name)]=s[2]
                if change_list == 0:
                    error4 = error4 + 1


        if s[0]=="setxattr 1":#super
            if int(s[3])==1:#not previously existing
                #check if in list
                add_to_list = 1
                for name in l_f1_n:
                    if name == s[1]:
                         add_to_list = 0
                if add_to_list == 0:
                    error4 = error4 + 1
                if add_to_list == 1:
                    l_f1_n.append(s[1])
                    l_f1_v.append(s[2])
                         #break
            if int(s[3])==2:#should be previously existing
                #check if in list
                change_list = 0
                for name in l_f1_n:
                    if name == s[1]:
                         change_list = 1
                         l_f1_v[l_f1_n.index(name)]=s[2]
                if change_list == 0:
                    error4 = error4 + 1

        if s[0]=="setxattr 2":#luper
            if int(s[3])==1:#not previously existing
                #check if in list
                add_to_list = 1
                for name in l_f0_n:
                    if name == s[1]:
                         add_to_list = 0
                if add_to_list == 0:
                    error4 = error4 + 1
                if add_to_list == 1:
                    l_f0_n.append(s[1])
                    l_f0_v.append(s[2])
                         #break
            if int(s[3])==2:#should be previously existing
                #check if in list
                change_list = 0
                for name in l_f0_n:
                    if name == s[1]:
                         change_list = 1
                         l_f0_v[l_f0_n.index(name)]=s[2]
                if change_list == 0:
                    error4 = error4 + 1
    print( "####################################################   cmd4   ####################################################")
    print( "--------------------------------------------------")
    print( "SETXATTR PART:")
    print( "--------------------------------------------------")
    cmd4_st_set = 0
    cmd4_our_set = 0
    f = open("test-st-set-out4.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR" :
            continue
        if s[0:15] =="getxattr: error":
            #print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue

        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1

                        if s[1] == l_f2_v[l_f2_n.index(name)]:

                            cmd4_st_set = cmd4_st_set + 2
                            #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break

            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f1_v[l_f1_n.index(name)]:

                            cmd4_st_set = cmd4_st_set + 2
                            #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "security":

                            if s[1] == l_f0_v[l_f0_n.index(name)]:
                                cmd4_st_set = cmd4_st_set + 2
                                #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %( fd, s[0],s[1]))
                                turn = -1
                                break

                        else:
                            if s[1] == l_f0_v[l_f0_n.index(name)]:

                                cmd4_st_set = cmd4_st_set + 1
                                #print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %( fd, s[0],s[1]))
                                turn = -1
                                break

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd4_st_set = cmd4_st_set + 2

                    turn = -1
                    #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE:" %(fd, s[0]))
                else:
                    turn = -1
                    #print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd, s[0]))

    f = open("test-set-out4.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR" :
            continue
        if s[0:15] =="getxattr: error":
            #print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue

        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1

                        if s[1] == l_f2_v[l_f2_n.index(name)]:

                            cmd4_our_set = cmd4_our_set + 2
                            #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break

            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f1_v[l_f1_n.index(name)]:

                            cmd4_our_set = cmd4_our_set + 2
                            #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "security":

                            if s[1] == l_f0_v[l_f0_n.index(name)]:
                                cmd4_our_set = cmd4_our_set + 2
                                #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %( fd, s[0],s[1]))
                                turn = -1
                                break

                        else:
                            if s[1] == l_f0_v[l_f0_n.index(name)]:

                                cmd4_our_set = cmd4_our_set + 1
                                #print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %( fd, s[0],s[1]))
                                turn = -1
                                break

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd4_our_set = cmd4_our_set + 2

                    turn = -1
                    #print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE:" %(fd, s[0]))
                else:
                    turn = -1
                    #print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd, s[0]))





    if cmd4_st_set > cmd4_our_set:
        f = open("test-st-set-out4.txt", "r")
    else:
        f = open("test-set-out4.txt", "r")

    turn = -1
    fd = -1
    #f = open("test-set-out4.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR" :
            continue
        if s[0:15] =="getxattr: error":
            print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue

        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1

                        if s[1] == l_f2_v[l_f2_n.index(name)]:

                            cmd4_set = cmd4_set + 2
                            print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break

            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f1_v[l_f1_n.index(name)]:

                            cmd4_set = cmd4_set + 2
                            print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "security":

                            if s[1] == l_f0_v[l_f0_n.index(name)]:
                                cmd4_set = cmd4_set + 2
                                print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %( fd, s[0],s[1]))
                                turn = -1
                                break

                        else:
                            if s[1] == l_f0_v[l_f0_n.index(name)]:

                                cmd4_set = cmd4_set + 1
                                print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %( fd, s[0],s[1]))
                                turn = -1
                                break

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd4_set = cmd4_set + 2

                    turn = -1
                    print ("[CORRECT(POINTS:+2):] fd = %d \t ATTR NAME: %s \t VALUE:" %(fd, s[0]))
                else:
                    turn = -1
                    print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd, s[0]))



    print( "------------------------------------------------------------------------------------")
    print ("[TOTAL POINTS FOR %s] : [%dpts]/[%dpts]" %("SETXATTR PART", cmd4_set, 13))
    print( "------------------------------------------------------------------------------------")


    print( "--------------------------------------------------")
    print( "GETXATTR PART:")
    print( "--------------------------------------------------")

    f = open("test-out4.txt", "r")
    turn = -1
    fd = -1
    for i,s in enumerate(f):
        flag = 0
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR":
            continue
        if s[0:32] == "CMPSC473 Error: fileSetAttr fail":
            e4 = e4 + 1
        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue
        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f2_v[l_f2_n.index(name)]:
                            cmd4_get = cmd4_get + 1
                            print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                            turn = -1
                            break
                        else:
                            turn = -1
                            print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))


            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f1_v[l_f1_n.index(name)]:
                            cmd4_get = cmd4_get + 1
                            print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                            turn = -1
                            break
                        else:
                            turn = -1
                            print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f0_v[l_f0_n.index(name)]:
                            cmd4_get = cmd4_get + 1
                            print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd, s[0],s[1]))
                            turn = -1
                            break
                        else:
                            turn = -1
                            print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd, s[0]))

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd4_get = cmd4_get + 1
                    turn = -1
                    print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE:" %(fd,s[0]))
                else:
                    turn = -1
                    print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))



    cmd4_e = 0
    if e4 == error4:
        cmd4_e = 7

    print( "------------------------------------------------------------------------------------")
    print ("[TOTAL POINTS FOR %s] : [%dpts]/[%dpts]" %("GETXATTR PART", cmd4_get, 4))
    print( "------------------------------------------------------------------------------------")

    print( "------------------------------------------------------------------------------------")
    print( "Correct Error messages for cmd4:")
    print ("[CORRECT ERROR BASED ON FLAGS:]  [%dpts]/[%dpts]" %(cmd4_e, 7))



    f = open("cmd5", "r")
    for i,s in enumerate(f):
        s = [x.strip() for x in s.split(',')]
        add_to_list = 1
        if s[0]=="setxattr 0":#super
            if int(s[3])==1:#not previously existing
                #check if in list
                add_to_list = 1
                for name in l_f1_n:
                    if name == s[1]:
                         add_to_list = 0
                if add_to_list == 0:
                    error5 = error5 + 1
                if add_to_list == 1:
                    l_f1_n.append(s[1])
                    l_f1_v.append(s[2])
                         #break
            if int(s[3])==2:#should be previously existing
                #check if in list
                change_list = 0
                for name in l_f1_n:
                    if name == s[1]:
                         change_list = 1
                         l_f1_v[l_f1_n.index(name)]=s[2]
                if change_list == 0:
                    error5 = error5 + 1

        if s[0]=="setxattr 1":#luper
            if int(s[3])==1:#not previously existing
                #check if in list
                add_to_list = 1
                for name in l_f0_n:
                    if name == s[1]:
                         add_to_list = 0
                if add_to_list == 0:
                    error5 = error5 + 1
                if add_to_list == 1:
                    l_f0_n.append(s[1])
                    l_f0_v.append(s[2])
                         #break
            if int(s[3])==2:#should be previously existing
                #check if in list
                change_list = 0
                for name in l_f0_n:
                    if name == s[1]:
                         change_list = 1
                         l_f0_n.append(s[1])
                         l_f0_v.append(s[2])
                         break
                if change_list == 0:
                    error5 = error5 + 1
    print( "####################################################   cmd5   ####################################################")
    print( "--------------------------------------------------")
    print( "SETXATTR PART:")
    print( "--------------------------------------------------")
    cmd5_st_set =0
    cmd5_our_set=0
    f = open("test-st-set-out5.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR":
            continue

        if s[0:15] =="getxattr: error":
            #print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue

        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f2_v[l_f2_n.index(name)]:
                            cmd5_st_set = cmd5_st_set + 2
                            #print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                            turn = -1
                            break

            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "article.venues":
                            if s[1] == l_f1_v[l_f1_n.index(name)]:
                                cmd5_st_set = cmd5_st_set + 1
                                #print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break


                        else:
                            if s[1] == l_f1_v[l_f1_n.index(name)]:
                                cmd5_st_set = cmd5_st_set + 2
                                #print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "article.venues":
                            if s[1] == '':
                                cmd5_st_set = cmd5_st_set + 1
                                #print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break

                        else:
                            if s[1] == l_f0_v[l_f0_n.index(name)]:
                                cmd5_st_set = cmd5_st_set + 1
                                #print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd5_st_set = cmd5_st_set + 1
                    turn = -1
                    #print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE:" %(fd,s[0]))
                else:
                    turn = -1
                    #print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))

    f = open("test-set-out5.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR":
            continue

        if s[0:15] =="getxattr: error":
            #print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue

        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f2_v[l_f2_n.index(name)]:
                            cmd5_our_set = cmd5_our_set + 2
                            #print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                            turn = -1
                            break

            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "article.venues":
                            if s[1] == l_f1_v[l_f1_n.index(name)]:
                                cmd5_our_set = cmd5_our_set + 1
                                #print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break


                        else:
                            if s[1] == l_f1_v[l_f1_n.index(name)]:
                                cmd5_our_set = cmd5_our_set + 2
                                #print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "article.venues":
                            if s[1] == '':
                                cmd5_our_set = cmd5_our_set + 1
                                #print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break

                        else:
                            if s[1] == l_f0_v[l_f0_n.index(name)]:
                                cmd5_our_set = cmd5_our_set + 1
                                #print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd5_our_set = cmd5_our_set + 1
                    turn = -1
                    #print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE:" %(fd,s[0]))
                else:
                    turn = -1
                    #print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))



    if cmd5_st_set > cmd5_our_set:
        f = open("test-st-set-out5.txt", "r")
    else:
        f = open("test-set-out5.txt", "r")

    turn = -1
    #f = open("test-set-out5.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR":
            continue

        if s[0:15] =="getxattr: error":
            print ("[INCORRECT(POINTS:0):] \t ATTR NAME: %s" %(s[35:-1]))
            continue

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "getxattr: fd: 2":
            turn = 2
            fd = 2
            continue

        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f2_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f2_v[l_f2_n.index(name)]:
                            cmd5_set = cmd5_set + 2
                            print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                            turn = -1
                            break

            elif turn == 1:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "article.venues":
                            if s[1] == l_f1_v[l_f1_n.index(name)]:
                                cmd5_set = cmd5_set + 1
                                print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break


                        else:
                            if s[1] == l_f1_v[l_f1_n.index(name)]:
                                cmd5_set = cmd5_set + 2
                                print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break


            elif turn == 2:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if str(s[0]) == "article.venues":
                            if s[1] == '':
                                cmd5_set = cmd5_set + 1
                                print ("[CORRECT(POINTS:+2):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break

                        else:
                            if s[1] == l_f0_v[l_f0_n.index(name)]:
                                cmd5_set = cmd5_set + 1
                                print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break

            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd5_set = cmd5_set + 1
                    turn = -1
                    print ("[CORRECT(POINTS:+1):] fd = %d \t NAME: %s \t VALUE:" %(fd,s[0]))
                else:
                    turn = -1
                    print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))


    print( "------------------------------------------------------------------------------------")
    print ("[TOTAL POINTS FOR %s] : [%dpts]/[%dpts]" %("SETXATTR PART", cmd5_set, 12))
    print( "------------------------------------------------------------------------------------")


    print( "--------------------------------------------------")
    print( "GETXATTR PART:")
    print( "--------------------------------------------------")
    f = open("test-out5.txt", "r")
    set = 0
    for i,s in enumerate(f):
        flag = 0

        s = s.rstrip('\n')
        if s[0:4] == "open" or s[0:5]=="XATTR":
            continue

        if s[0:32] == "CMPSC473 Error: fileSetAttr fail":
            e5 = e5 + 1

        if s[0:15] == "getxattr: fd: 0":
            turn = 0
            fd = 0
            continue

        if s[0:15] == "getxattr: fd: 1":
            turn = 1
            fd = 1
            continue

        if s[0:15] == "setxattr: fd: 1":
            set = 1
            continue
        none = -1
        flag = s.find('=')
        s = s.split('=')
        if flag != -1:
            if turn == 0:

                for name in l_f1_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f1_v[l_f1_n.index(name)]:
                            cmd5_get = cmd5_get + 1
                            print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                            turn = -1
                            break
                        else:
                            turn = -1
                            print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))



            elif turn == 1:

                for name in l_f0_n:
                    if s[0] == name:
                        none = 1
                        if s[1] == l_f0_v[l_f0_n.index(name,set*l_f0_n.index(name)+set*1)]:
                                cmd5_get = cmd5_get + 1
                                print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE: %s" %(fd,s[0],s[1]))
                                turn = -1
                                break
                        else:
                            turn = -1
                            print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))


            if turn != -1:
                if s[1] == '' and none == -1:
                    cmd5_get = cmd5_get + 1
                    turn = -1
                    print ("[CORRECT(POINTS:+1):] fd = %d \t ATTR NAME: %s \t VALUE:" %(fd,s[0]))
                else:
                    turn = -1
                    print ("[INCORRECT(POINTS:0):] fd = %d \t ATTR NAME: %s" %(fd,s[0]))


    cmd5_e = 0
    if e5 == error5:
        cmd5_e = 3

    print( "------------------------------------------------------------------------------------")
    print ("[TOTAL POINTS FOR %s] : [%dpts]/[%dpts]" %("GETXATTR PART", cmd5_get, 8))
    print( "------------------------------------------------------------------------------------")

    print( "Correct Error messages for cmd5:")
    print ("[CORRECT ERROR BASED ON FLAGS:]  [%dpts]/[%dpts]" %(cmd5_e, 3))
    print( "#####################################################################################################################")
    print( "[CORRECTNESS: XATTR NUMBER OF NAMES AND SPACE OCCUPIED:]")

    f2 = open("correctness_yours.txt", "w")
    f = open("test-xattr-out5.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:5]=="XATTR":
            print("YOUR "+s)
            f2.write(s+"\n")
    f2.close()
    print( "--------------------------------------------------------------------------------------------------------------------")

    f2 = open("correctness_ours.txt", "w")
    f = open("test-set-out5.txt", "r")
    for i,s in enumerate(f):
        s = s.rstrip('\n')
        if s[0:5]=="XATTR":
            print("CORRECT "+s)
            f2.write(s+"\n")

    f2.close()
    comp=str( filecmp.cmp('correctness_ours.txt','correctness_yours.txt'))
    if comp == "True":
        diff = 0
    else:
        diff = 1

    print( "#####################################################################################################################")

    print( "[FINAL SCORE:]")
    points = (-5)*diff + 2 + cmd5_e + cmd4_e + cmd4_get + cmd4_set + cmd3_get +cmd3_set + cmd5_get +cmd5_set
    print ("[INFO]  %s [%dpts]/[%dpts]" %("TOTAL POINTS EARNED = ", points, 60))
    print( "#####################################################################################################################")

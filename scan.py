import os
import re
import sys
import argparse

suffix = ['.c','.h']
red_name = ["strcpy", "wcscpy","strcat","wcscat","sprintf","vsprintf",
                "strtok","sscanf","vsscanf","gets","ato[a-z]"]
exp_dict = {"strcpy":"strncpy",
           "wcscpy": "strncpy",
           "strcat": "strncat",
           "wcscat": "strncat",
           "sprintf": "snprintf",
           "vsprintf": "vsnprintf",
           "vsnprintf": "?",
           "strtok": "strtok_r,strsep",
           "sscanf":"?",
           "vsscanf": "?",
           "gets":"?",
           "ato[a-z]":"strto",
           "strlen": "strnlen",
           "wcslen": "strnlen",
           "alloca": "?"}

yello_name = ["vsnprintf","strlen","wcslen","alloca"]

exc_list=["/*","*\\","DPRINTF"]

def parse_init():
    parser= argparse.ArgumentParser()
    parser.add_argument('-p', '--path',
                        required=True,
                        help='please give the path')
    return parser

def main():
    parser = parse_init()
    args = parser.parse_args()
    path = args.path
    if not os.path.exists(path):
        print("{} not exist!!")
        sys.exit()


    for root,dirs,files in os.walk(path):
        for f in files:
            if files and os.path.splitext(f)[-1] in suffix:
                f_path = "{}/{}".format(root,f)
                with open(f_path) as f:
                    for line in f.readlines():
                        for e in exc_list:
                            if e in line:
                                break
                        else:
                            for red in red_name:
                                pa = re.compile(r'[^a-zA-Z0-9_]{}[^a-zA-Z0-9]'.format(red))
                                pb = re.compile(r'\A{}[^a-zA-Z0-9_]'.format(red))
                                if pa.findall(line) or pb.findall(line):
                                    exp = exp_dict[red]
                                    print('\033[4;31;40m'+red+'\033[0m'+
                                          "->\033[4;32;40m{}\033[0m".format(exp) +
                                          "  {},{}".format(f_path,line[:-1]))
                            for yel in yello_name:
                                pa = re.compile(r'[^a-zA-Z0-9_]{}[^a-zA-Z0-9]'.format(yel))
                                pb = re.compile(r'\A{}[^a-zA-Z0-9_]'.format(yel))
                                if pa.findall(line) or pb.findall(line):
                                    exp = exp_dict[yel]
                                    print('\033[4;33;40m'+yel+'\033[0m'+
                                          "->\033[4;32;40m{}\033[0m".format(exp) +
                                          "  {},{}".format(f_path,line[:-1]))

if __name__ == "__main__":
    main()


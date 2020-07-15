import sys
import os

COMMON_DIR = "common"
COMMON_DIR_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), COMMON_DIR)

# Code shared between the testcases should be put in COMMON_DIR folder.
# Appending this path to the python runtime environment makes sure that
# we can import any of the modules in COMMON_DIR from any testcase.
sys.path.append(COMMON_DIR_PATH)

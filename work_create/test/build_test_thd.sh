#!/bin/bash

gcc ./test_thd.c `find ../threads -name "*.c"`  `find ../xqueue -name "*.c"` -I ../threads  -I ../xqueue -o test_thd -pthread

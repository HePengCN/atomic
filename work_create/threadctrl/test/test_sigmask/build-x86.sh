#!/bin/bash

test_name="test_sigmask_x86"

script_dir=`dirname $(readlink -f $0)`

project_dir=$script_dir/../../../..


gcc -rdynamic -g -o $test_name `find $script_dir/src  -name "*.c"` $project_dir/middleware/threadctrl/thd_ctrl.c $project_dir/middleware/xqueue/xqueue.c  -I $script_dir/src -I $project_dir/include/middleware/threadctrl -I $project_dir/middleware/threadctrl/include -I $project_dir/include/middleware/xqueue -I $project_dir/middleware/xqueue/include/ -pthread

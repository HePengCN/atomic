#!/bin/bash

test_name="test_sigterm_arm"

script_dir=`dirname $(readlink -f $0)`

project_dir=$script_dir/../../../..


$CC -g -rdynamic -funwind-tables -ffunction-sections -fexceptions -o $test_name `find $script_dir/src -name "*.c"` $project_dir/middleware/sigterm_handler/sigterm_handler.c $project_dir/middleware/threadctrl/thd_ctrl.c $project_dir/middleware/xqueue/xqueue.c -I $project_dir/middleware/sigterm_handler/include -I $script_dir/src -I $project_dir/include/middleware/threadctrl -I $project_dir/middleware/threadctrl/include -I $project_dir/include/middleware/xqueue -I $project_dir/middleware/xqueue/include/ -I $project_dir/include/middleware/sigterm_handler  -pthread

#!/bin/bash

test_name="test_update_as_desc_x86"

script_dir=`dirname $(readlink -f $0)`

project_dir=$script_dir/../../../..


gcc -DTEST_update_as_desc -g -o $test_name `find $script_dir/src  -name "*.c"` `find $project_dir/middleware/cardata_proc/hash_table $project_dir/middleware/cardata_proc/adaption -name "*.c"`  $project_dir/middleware/cardata_proc/thds/local_data_update.c $project_dir/middleware/threadctrl/thd_ctrl.c $project_dir/middleware/xqueue/xqueue.c $project_dir/middleware/sigterm_handler/sigterm_handler.c  -I $script_dir/src -I $project_dir/include/middleware/threadctrl -I $project_dir/middleware/threadctrl/include -I $project_dir/include/middleware/xqueue -I $project_dir/middleware/xqueue/include -I $project_dir/middleware/sigterm_handler/include -I $project_dir/include/middleware/sigterm_handler  -I $project_dir/middleware/threadctrl/test/test_timer/src/timer -I $project_dir/middleware/cardata_proc/include/car_data -I $project_dir/middleware/cardata_proc/include/hash_table -I $project_dir/middleware/cardata_proc/adaption -I $project_dir/middleware/cardata_proc/include/thds -I $project_dir/middleware/cardata_proc/include/imsgc -I $project_dir/middleware/cardata_proc/include/timer -I $project_dir/middleware/cardata_proc/include  -I $project_dir/middleware/cardata_proc/tool_gen -I $project_dir/middleware/mcus/include/cont -pthread -lrt

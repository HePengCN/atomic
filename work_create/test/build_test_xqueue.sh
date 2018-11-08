#!/bin/bash

gcc test_xqueue.c ../xqueue/xqueue.c ../xqueue/xqueue_impl.c  -I ../xqueue -pthread -o  test_xqueue

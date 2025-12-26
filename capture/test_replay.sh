#!/bin/bash
cd /media/data/programming/zwergII/capture

python3 replay_exact_android.py > replay_result.log 2>&1
echo "Exit code: $?" >> replay_result.log

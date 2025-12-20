#!/bin/bash
# input: $1 -- cpu set, e.g. '1', '1-3', '5,6'
# input: $2 -- PID that sets
systemctl set-property --runtime system.slice AllowedCPUs=0
systemctl set-property --runtime user.slice AllowedCPUs=0

if [ ! -e /sys/fs/cgroup/shield ]; then
	mkdir /sys/fs/cgroup/shield
fi
echo $1 > /sys/fs/cgroup/shield/cpuset.cpus
echo $2 > /sys/fs/cgroup/shield/cgroup.procs

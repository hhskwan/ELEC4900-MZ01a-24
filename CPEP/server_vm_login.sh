#!/bin/bash
echo "Try log in and execute script"

PARAMS="$@"
echo "Executing server script..."
timeout 60s vagrant ssh serverTest -c "cd /vagrant/foggytcpTest && chmod +x ./server_script_cpep.sh && ./server_script_cpep.sh $PARAMS"

server_status=$?

echo "Server script exit code: $server_status"

if [[ $server_status -eq 0 ]]; then
    echo "Execution success"
    exit 0

elif [[ $server_status -eq 124 ]]; then
    echo "Timeout, cannot login to server VM, or server script does not executed until the end"
    exit 124

elif [[ $server_status -eq 255 ]]; then
    echo "Log in failed, please check your VM status"
    exit 255

elif [[ $server_status -eq 126 ]]; then
    echo "Permission denied, please check your accessibility to server_script_cpep.sh via VM"
    exit 126

elif [[ $server_status -eq 127 ]]; then
    echo "server_script_cpep.sh not found within the path /foggytcpTest"
    exit 127

else
    echo "unknown error"
    exit -1

fi



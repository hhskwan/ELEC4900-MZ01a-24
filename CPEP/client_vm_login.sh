#!/bin/bash
echo "Try log in and execute script"

PARAMS="$@"
echo "Executing client script..."
timeout 60s vagrant ssh clientTest -c "cd /vagrant/foggytcpTest && chmod +x ./client_script_cpep.sh && ./client_script_cpep.sh $PARAMS"

client_status=$?

echo "client script exit code: $client_status"

if [[ $client_status -eq 0 ]]; then
    echo "Execution success"
    exit 0

elif [[ $client_status -eq 124 ]]; then
    echo "Timeout, cannot login to client VM, or client script does not executed until the end"
    exit 124

elif [[ $client_status -eq 255 ]]; then
    echo "Log in failed, please check your VM status"
    exit 255

elif [[ $client_status -eq 126 ]]; then
    echo "Permission denied, please check your accessibility to client_script_cpep.sh via VM manually"
    exit 126

elif [[ $client_status -eq 127 ]]; then
    echo "client_script_cpep.sh not found within the path /foggytcpTest"
    exit 127

else
    echo "unknown error"
    exit -1

fi



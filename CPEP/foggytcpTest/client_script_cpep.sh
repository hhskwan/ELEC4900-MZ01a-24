#!/usr/bin/env bash
echo "Client VM script executing"
cd /vagrant/foggytcpTest

if [ $? -ne 0 ]; then
    echo "Error: Failed to change directory"
    exit -1
fi

if [ ! -d build ]; then
    mkdir build
fi

set_network_conditions() {
    BANDWIDTH=$1
    UNIT=$2
    DELAY=$3
    LOSS=$4
    INTERFACE=$5
    FLUCTUATION=$6

    if [ "$FLUCTUATION" == "true" ]; then
        echo "Fluctuation enabled: Dynamic fluctuation applied in real-time."

        BANDWIDTH=$(($BANDWIDTH + $(($RANDOM % 20 - 10))))
        DELAY=$(($DELAY + $(($RANDOM % 50 - 25))))
        LOSS=$(($LOSS + $(($RANDOM % 5 - 2))))

        BANDWIDTH=$((BANDWIDTH < 0 ? 0 : BANDWIDTH))
        DELAY=$((DELAY < 0 ? 0 : DELAY))
        LOSS=$((LOSS < 0 ? 0 : LOSS))

        sudo tcset "$INTERFACE" --rate "${BANDWIDTH}${UNIT}" --loss "${LOSS}%" --delay "${DELAY}ms" --overwrite
        sudo tcshow "$INTERFACE"
        echo "Initial network conditions: Delay=${DELAY}ms, Loss=${LOSS}%, Bandwidth=${BANDWIDTH}${UNIT}"

        sleep 1

    else
        if [ "$DELAY" -ge 0 ]; then
            if [ "$LOSS" -ge 0 ]; then
                if [ "$BANDWIDTH" -gt 0 ]; then
                    sudo tcset "$INTERFACE" --rate "${BANDWIDTH}${UNIT}" --loss "${LOSS}%" --delay "${DELAY}ms" --overwrite
                    sudo tcshow "$INTERFACE"
                fi
            fi
        fi
    fi

    echo "Network conditions applied: Delay=${DELAY}ms, Loss=${LOSS}%, Bandwidth=${BANDWIDTH}${UNIT}"
}

START_TIME=0
CURRENT_TIME=0
timeout=8

while [ ! -f ./ready.flag ]; do
    sleep 1
    ((CURRENT_TIME++))
    elapsed_time=$((CURRENT_TIME - START_TIME))

    if [ $elapsed_time -ge  $timeout ]; then
        echo "timeout"
        exit 8
    fi
done

BANDWIDTH=${1:-100}
UNIT=${2:-"Mbps"}
DELAY=${3:-0}
LOSS=${4:-0}
INTERFACE=${5:-"enp0s8"}
FLUCTUATION=${6:-"false"}
CONCURRENT=${7:-"false"}

set_network_conditions "$BANDWIDTH" "$UNIT" "$DELAY" "$LOSS" "$INTERFACE"

sudo ./utils/capture_packets.sh start client.pcap
echo "debug: capture_packets started"

TIME_LIMIT=10
START_TIME=$(date +%s)

if [ "$FLUCTUATION" == "true" ]; then
    while true; do
        CURRENT_TIME=$(date +%s)
        ELAPSED_TIME=$((CURRENT_TIME - START_TIME))

        if [ "$ELAPSED_TIME" -ge "$TIME_LIMIT" ]; then
            echo "Time limit reached, stopping execution after 5 seconds."
            sleep 10
            break
        fi

        ./client 10.0.1.1 3120 src/client.cc &
    done
fi
#done
if [ "$CONCURRENT" == "false" ]; then
    timeout 10s ./client 10.0.1.1 3120 Confuse.jpg
fi
CLIENT_PID=$!
echo "debug 2"

wait $CLIENT_PID

sleep 8 # wait for all packets to be captured
sudo ./utils/capture_packets.sh stop client.pcap

echo "Client exit successfully"
exit 0
fi



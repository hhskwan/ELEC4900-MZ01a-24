#!/usr/bin/env bash

echo "Server VM script executing"
cd /vagrant/foggytcpTest

if [ $? -ne 0 ]; then
    echo "Error: Failed to change directory"
    exit 1
fi

if [ ! -d build ]; then
    mkdir build
fi

#make foggy1

set_network_conditions() {
    BANDWIDTH=$1
    UNIT=$2
    DELAY=$3
    LOSS=$4
    INTERFACE=$5
    FLUCTUATION=$6

    if [ "$FLUCTUATION" == "true" ]; then

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

BANDWIDTH=${1:-100}
UNIT=${2:-"Mbps"}
DELAY=${3:-0}
LOSS=${4:-0}
INTERFACE=${5:-"enp0s8"}
FLUCTUATION=${6:-"false"}

set_network_conditions "$BANDWIDTH" "$UNIT" "$DELAY" "$LOSS" "$INTERFACE" "$FLUCTUATION"

sudo ./utils/capture_packets.sh start server.pcap

timeout 30s ./server 10.0.1.1 3120 test.out &
echo "Server started"
SERVER_PID=$!
sleep 2

touch ./ready.flag
echo "debug"
wait $SERVER_PID

sudo ./utils/capture_packets.sh stop server.pcap

echo "Server exit successfully"
rm ./ready.flag
exit 0
fi

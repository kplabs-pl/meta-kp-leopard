#!/bin/bash

function _print_help {
    MY_NAME=$(basename $0)
    echo -e "ssd tool usage:"
    echo -e "\t$MY_NAME power [a|b] [off|on]"
    echo -e "\t$MY_NAME status"
}

function _print_status {
    SSD_A_STATUS=$(cat /sys/class/leds/ssd_a/brightness)
    SSD_B_STATUS=$(cat /sys/class/leds/ssd_b/brightness)
    echo "Legend: 0=OFF, 1=ON"
    echo "SSD A power status: $SSD_A_STATUS"
    echo "SSD B power status: $SSD_B_STATUS"
}

function _wait_for_dir_exist_change {
    EXPECTED_EXIST_STATE=$1
    SSD_ID=$2
    WAIT_TIME=$3

    while [ $WAIT_TIME -gt 0 ]; do
        if [[ $EXPECTED_EXIST_STATE -eq 0 ]]; then
            if [ ! -e "/dev/ssd/$SSD_ID" ]; then
                return 0
            fi
        else
            if [ -e "/dev/ssd/$SSD_ID" ]; then
                return 0
            fi
        fi

        ((WAIT_TIME--))
        sleep 1
    done

    return 1
}

function _enable_ssd_disk {
    SSD_ID=$1

    if [[ -e "/dev/ssd/$SSD_ID" ]]; then
        echo "Disk already enabled"
        exit 0
    fi

    echo 1 >> /sys/class/leds/ssd_$SSD_ID/brightness

    WAIT_TIME=35
    RESULT= _wait_for_dir_exist_change 1 $SSD_ID $WAIT_TIME

    if [[ $RESULT -ne 0 ]]; then
        echo "Disk failed to enable"
        echo 0 > /sys/class/leds/ssd_$SSD_ID/brightness
        exit 1
    fi

    echo "Disk enabled"
    exit 0
}

function _disable_ssd_disk {
    SSD_ID=$1

    if [[ ! -e "/dev/ssd/$SSD_ID" ]]; then
        echo "Disk already disabled"
        exit 0
    fi

    sync
    DELETE_NODE="/sys/$(udevadm info --query=path /dev/ssd/$SSD_ID)/device/delete"
    echo 1 > $DELETE_NODE

    WAIT_TIME=10
    RESULT= _wait_for_dir_exist_change 0 $SSD_ID $WAIT_TIME

    if [[ $RESULT -ne 0 ]]; then
        echo "Disk failed to disable"
        exit 1
    fi

    echo 0 > /sys/class/leds/ssd_$SSD_ID/brightness
    echo "Disk disabled"
    exit 0
}

function _set_ssd_disk_power {
    SSD_ID=$1
    EXPECTED_EXIST_STATE=$2

    case $EXPECTED_EXIST_STATE in
        0|off)
            _disable_ssd_disk $SSD_ID
        ;;

        1|on)
            _enable_ssd_disk $SSD_ID
        ;;

        *)
            echo "\nUnsupported driver state: \"$EXPECTED_EXIST_STATE\". Supported states: <0|off | 1|on>"
            exit 1
            ;;
    esac
}

# Main
COMMAND=$1
case $COMMAND in
    -h|--help)
        _print_help
    ;;

    power)
        _set_ssd_disk_power $2 $3
    ;;

    status)
        _print_status
    ;;

    *)
        echo "Unknown command: \"$COMMAND\"."
        _print_help
    ;;
esac

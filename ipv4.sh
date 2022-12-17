#! /bin/bash

# IPv4 RegEx
ipv4="^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$"

read -p "Enter IPv4 KERNEL: " kernel

if [[ "$kernel" =~ $ipv4  ]]; then
        find . -name '*.config' -exec sed -i "s/IP_KERNEL=[0-9\.]*/IP_KERNEL=$kernel/g" {} \;
        echo "IPv4 KERNEL has been successfully updated"
else
        echo "Invalid IPv4 address"
        exit 1
fi

read -p "Enter IPv4 CPU: " cpu

if [[ "$cpu" =~ $ipv4  ]]; then
        find . -name '*.config' -exec sed -i "s/IP_CPU=[0-9\.]*/IP_CPU=$cpu/g" {} \;
        echo "IPv4 CPU has been successfully updated"
else
        echo "Invalid IPv4 address"
        exit 1
fi

read -p "Enter IPv4 MEMORIA: " memory

if [[ "$memory" =~ $ipv4  ]]; then
        find . -name '*.config' -exec sed -i "s/IP_MEMORIA=[0-9\.]*/IP_MEMORIA=$memory/g" {} \;
        echo "IPv4 MEMORY has been successfully updated"
else
        echo "Invalid IPv4 address"
        exit 1
fi
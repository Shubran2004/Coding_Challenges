#!/bin/bash

files=( * )
for item in "${files[@]}"; do
    if [[ -f "$item" && "$item" == *.json ]]; then
        echo "$item" | ./main.exe
    fi
done
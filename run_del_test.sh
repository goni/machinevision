#!/bin/bash

# Usage: ./script.sh <delete_interval_sec> <delete_size_kilobytes>
directory="/share/F0/data/MV-CH120-10GC (02F89270002)"
sec=$1      # 삭제 주기 (초)
size=$2     # 삭제 기준 용량 (kB)

# 마지막 이동 작업 수행 시각 (epoch 초)
last_move_time=0

while true; do
    echo "[INFO] Sleeping for $sec seconds before next cycle..."
    sleep "$sec"

    cd "$directory" || { echo "[ERROR] Directory $directory not found"; exit 1; }

    # Step 1: 삭제 작업 (최신 2개 제외)
    files_to_consider=$(find . -maxdepth 1 -name '*.png' -size -${size}k -type f -printf '%T@ %p\n' | sort -n)
    files_to_delete=$(echo "$files_to_consider" | head -n -2 | cut -d ' ' -f 2-)

    if [ -n "$files_to_delete" ]; then
        echo "[INFO] Deleting the following files:"
        echo "$files_to_delete"
        rm -f $files_to_delete
    else
        echo "[INFO] No files to delete."
    fi

    # Step 2: 1시간마다 파일 이동
    current_time=$(date +%s)
    elapsed=$((current_time - last_move_time))

    if [ "$elapsed" -ge 3600 ]; then
        echo "[INFO] Performing file move operation..."

        # 이동 대상 PNG 전체 중 최신 2개 제외
        all_png=$(find . -maxdepth 1 -name '*.png' -type f -printf '%T@ %p\n' | sort -n)
        files_to_move=$(echo "$all_png" | head -n -2 | cut -d ' ' -f 2-)

        for file in $files_to_move; do
            ymd=$(date -r "$file" "+%y%m%d")
            target_dir="../$ymd"
            calib_dir="$target_dir/Calib"
            mkdir -p "$target_dir"

            # 파일 이동
            mv "$file" "$target_dir/"

            # 이동 후 10MB 이상이면 Calib 하위로
            moved_file="$target_dir/$(basename "$file")"
            if [ -f "$moved_file" ] && [ $(stat -c%s "$moved_file") -ge $((10*1024*1024)) ]; then
                mkdir -p "$calib_dir"
                mv "$moved_file" "$calib_dir/"
                echo "[INFO] Moved $(basename "$file") to $calib_dir/"
            fi
        done

        last_move_time=$current_time
    else
        echo "[INFO] Skipping move (only $elapsed seconds since last move)."
    fi
done


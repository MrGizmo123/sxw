cat /proc/meminfo | grep -E '^MemTotal|^MemFree|^Buffers|^Cached' | tr -s ' ' | cut -d ' ' -f2 | tr '\n' ' ' | awk -F ' ' ' { print $1-$2-$3-$4"000" } ' | numfmt --to=iec-i




git fetch -q && [ "$(git rev-parse HEAD)" = "$(git rev-parse @{u})" ] && { echo "Sin cambios"; exit 0; } || { git pull && make clean && make -j4 && make run; }

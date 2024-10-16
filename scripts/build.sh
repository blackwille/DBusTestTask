#!/bin/bash

# По умолчанию - папка проекта, которая на 1 уровень выше
folder_with_cmakelists='..'

# Если хотите, можно этот скрипт настроить в IDE, явно указав папку до cmakelists проекта
if [[ $1 ]]; then 
    folder_with_cmakelists=$1
    echo -e "\e[0;34m""Using user specified dir with cmakelists: "$1"\e[0m"
fi

# Генерируем сборочные файлы проекта в Debug режиме 
# и экспортируем compile_commands для адекватной работы clangd
cmake -S${folder_with_cmakelists} -B${folder_with_cmakelists}'/build' \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_TOOLCHAIN_FILE=${folder_with_cmakelists}'/build/conan_toolchain.cmake'

# Собственно, собираем сам проект
cmake --build ${folder_with_cmakelists}'/build'
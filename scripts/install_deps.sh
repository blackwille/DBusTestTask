#!/bin/bash

# По умолчанию - папка проекта, которая на 1 уровень выше
folder_with_conanfile="$(dirname $(dirname $(realpath -- $0)))"
echo $folder_with_conanfile

# Если хотите, можно этот скрипт настроить в IDE, явно указав папку до conanfile проекта
# в качестве первого аргумента
if [[ $1 ]]; then 
    folder_with_conanfile=$1
    echo -e "\e[0;34m""Using user specified dir with conanfile: "$1"\e[0m"
fi

# Команда установки зависимостей с помощью пакетного манеджера 
# (в первый раз долго из-за сборки, далее берется из кеша)
# Стоит учитывать, что профиль по умолчанию - релизный, потому не получится продебажиться на нем
conan install ${folder_with_conanfile} --output-folder=${folder_with_conanfile}'/build' --build=missing
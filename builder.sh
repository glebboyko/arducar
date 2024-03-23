#!/bin/bash

git_libs=(c_tcp_lib clogger_lib ptit_lib)

libs_dir="$(pwd)/libs"
rm -rf "${libs_dir}"
mkdir "${libs_dir}"

rm -rf "${libs_dir}/built"
mkdir "${libs_dir}/built"

git_link="https://github.com/glebboyko"
for lib in ${git_libs[*]}
do
  rm -rf "${libs_dir}/${lib}"
  git clone "${git_link}/${lib}.git" "${libs_dir}/${lib}"
  (cd "${libs_dir}/${lib}"; /bin/bash cpp_lib_get.sh; /bin/bash cpp_lib_build.sh; /bin/bash cpp_build.sh "${libs_dir}/built")
done


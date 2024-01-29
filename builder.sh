#!/bin/bash

git_libs=(c_tcp_lib clogger_lib primitives-to-image-translator)

libs_dir="$(pwd)/libs"

rm -rf "${libs_dir}/built"
mkdir "${libs_dir}/built"

git_link="https://github.com/glebboyko"
for lib in ${git_libs[*]}
do
  rm -rf "${libs_dir}/${lib}"
  git clone "${git_link}/${lib}.git" "${libs_dir}/${lib}"
  cmake_dir=""
  if [[ ${lib} == "c_tcp_lib" ]]; then
    cmake_dir="cpp"
  fi
  "${libs_dir}/lib_builder.sh" "${libs_dir}/${lib}/${cmake_dir}" "${libs_dir}/built" ".a"
done


#!/bin/bash
# Converts a given (100% compatible) SSA subtitle file to SRT format


if [ "${1}" != "" ] ; then
  if [ "${2}" != "" ] ; then
    events=0
    c=2
    echo 1 > ${2}
    cat "${1}" \
    | grep "Dialogue" \
    | sed "/^Dialogue/s:\([^,]*,\)\([^,]*\),\([^,]*\)\(,.*\):0\20 --> 0\30\n\4\n:" \
    | sed "s:^,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,::" \
    | sed "s:\\\[nN]:\n:" \
    | while read s ; do
            if [ "${s}" != "" ] ; then
              echo $s
            else
              echo
              echo ${c}
              c=`expr ${c} + 1`
              fi
              done \
    | sed '$d' \
    >> "${2}"
  else
    echo Need an output file!
  fi
else
  echo Need an input file!
fi

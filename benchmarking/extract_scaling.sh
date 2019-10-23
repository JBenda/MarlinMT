#!/bin/bash


if [ ! "$#" == "5" ]
then
  echo "Usage ./extract_scaling.sh <max-n-cores> <crunch-sigma-percent> <lazy-unpack> <trigger-unpacking> <input-file>"
  echo "Example:  ./extract_scaling.sh 40 0.1 true true /path/to/lcio-file.slcio"
  exit 1
fi

if [ -z "$MARLIN_DIR" ]
then
  echo "MARLIN_DIR not set. Please setup your environment!"
  exit 1
fi

maxcores=$1
sigmapercent=$2
lazyUnpack="$3"
triggerUnpacking="$4"
inputfile="$5"
cores=`seq 2 ${maxcores}`
crunchtimes=`seq 500 500 2500`

lazyUnpackStr=""
triggerUnpackingStr=""

if [ "${lazyUnpack}" == "true" ]
then
  lazyUnpackStr="lazy"
else
  lazyUnpackStr="nolazy"
fi

if [ "${triggerUnpacking}" == "true" ]
then
  triggerUnpackingStr="trig"
else
  triggerUnpackingStr="notrig"
fi

output="scaling_output_${lazyUnpackStr}_${triggerUnpackingStr}_${sigmapercent}.txt"

if [ -f ${output} ]
then
  rm ${output}
fi

for c in ${cores}
do
  for cr in ${crunchtimes}
  do
    crunchSigma=`echo "(${cr}*${sigmapercent})" | bc`
    MarlinMT \
    $MARLIN_DIR/benchmarking/cpu_crunching.xml \
    --constant.TriggerUnpacking="${triggerUnpacking}" \
    --datasource.LCIOInputFiles="${inputfile}" \
    --datasource.LazyUnpack="${lazyUnpack}" \
    --CPUCrunch.CrunchTime=${cr} \
    --CPUCrunch.CrunchSigma=${crunchSigma} \
    --global.Verbosity=MESSAGE \
    --global.Concurrency=${c} > scaling_temp.log 
    serial=$( cat scaling_temp.log | grep serial | awk '{print $7}' )
    parallel=$( cat scaling_temp.log | grep serial | awk '{print $9}' )
    scaling=$( cat scaling_temp.log | grep serial| awk '{print $11}' )
    echo "-- N cores:        ${c}"
    echo "-- Crunch time:    ${cr}"
    echo "-- Crunch sigma:   ${crunchSigma}"
    echo "-- Serial time:    ${serial}"
    echo "-- Parallel time:  ${parallel}"
    echo "-- Scaling:        ${scaling}"
    echo ""
    echo "${c} ${cr} ${crunchSigma} ${serial} ${parallel} ${scaling}" >> ${output}
  done
done

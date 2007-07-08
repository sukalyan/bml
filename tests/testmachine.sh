#!/bin/bash
# $Id: testmachine.sh,v 1.1 2007-05-13 17:17:05 ensonic Exp $
# iterates over the given directory and tries all buzz machines
#
# if elements hang, they can be skipped by pressing ctrl-c
#
# ./testmachine.sh "machines/*.dll"
# ./testmachine.sh "/home/ensonic/buzztard/lib/Gear-real/Effects/*.dll"
# ./testmachine.sh "/home/ensonic/buzztard/lib/Gear-real/Generators/*.dll"
# ./testmachine.sh "/home/ensonic/buzztard/lib/Gear-real/*/*.dll"
#
# analyze results
#
# search for unk_XXX -> unknown symbols
#  grep -Hn "unk_" *.fail
# search for FIXME -> unimplemented buzz host entries
#  grep -Hn "FIXME" *.fail
# search for "wine/module: Win32 LoadLibrary failed to load:"
#  grep -ho "wine/module: Win32 LoadLibrary failed to load:.*" *.fail | sort | uniq
# stats
#  ls -1 testmachine/*.okay | wc -l
#  ls -1 testmachine/*.fail | wc -l

. ./bt-cfg.sh

if [ -z "$1" ] ; then
  echo "Usage: $0 <directory>";
  exit
fi

# initialize

machine_glob="$1";
mkdir -p testmachine
m_okay=0;
m_fail=0;
trap "sig_segv=1" SIGSEGV
trap "sig_int=1" INT
touch testmachine.failtmp

# run test loop

for machine in $machine_glob ; do
  name=`basename "$machine" "$machine_glob"`
  log_name="./testmachine/$name.txt"
  rm -f "$log_name" "$log_name".okay "$log_name".fail
  sig_segv=0
  sig_int=0
  #env >testmachine.log 2>&1 LD_LIBRARY_PATH="../src/" ../src/bmltest_info "$machine"
  #res=$?
  # this suppresses the output of e.g. "Sementation fault"
  res=`env >testmachine.log 2>&1 LD_LIBRARY_PATH="../src/:$LIBDIR" ../src/bmltest_info "$machine"; echo $?`
  cat testmachine.log | grep >"$log_name" -v "Warning: the specified"
  if [ $sig_int -eq "1" ] ; then res=1; fi
  if [ $res -eq "0" ] ; then
    echo "okay : $machine";
    m_okay=$((m_okay+1))
    mv "$log_name" "$log_name".okay
  else
    echo "fail : $machine";
    m_fail=$((m_fail+1))
    mv "$log_name" "$log_name".fail
    reason=`tail -n1 "$log_name".fail | strings`;
    echo "$reason :: $name" >>testmachine.failtmp
  fi
done

# cleanup and report

rm testmachine.log
sort testmachine.failtmp >testmachine.fails
rm testmachine.failtmp

m_all=$((m_fail+m_okay))
echo "Of $m_all machine(s) $m_okay machine(s) worked and $m_fail machine(s) failed."
echo "See testmachine.fails for details"

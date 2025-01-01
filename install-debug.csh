#!/bin/csh -f

set app=llpeak
set dstdir=~/opt/bin

# xcodebuild -list -project $app.xcodeproj
# rm -rf DerivedData/
# xcodebuild -configuration Release -alltargets clean
xcodebuild -scheme $app -configuration Debug clean build
if ($status != 0) then
  say -v karen "Failed to build $app"
  exit -1
endif

# echo -------------------
# find ./DerivedData -type f -name $app -perm +111 -ls
set src=./DerivedData/Build/Products/Debug/$app

echo
echo "---Install $src"
cp $src ${dstdir}/

echo
echo "---Files "
ls -al $src  ${dstdir}/$app

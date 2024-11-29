#!/bin/csh -f

set app=llpeak
xcodebuild -list -project $app.xcodeproj

# rm -rf DerivedData/
xcodebuild -scheme $app -configuration Debug clean build
# xcodebuild -configuration Release -alltargets clean


find ./DerivedData -type f -name $app -perm +111 -ls
set src=./DerivedData/Build/Products/Debug/$app

echo "File ß$src"
ls -al $src
cp $src ~/opt/bin/
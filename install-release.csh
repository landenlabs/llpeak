#!/bin/csh -f

set name=llpeak
find ./DerivedData -type f -name ${name} -perm +444 -ls 

set src=Release
echo
ls -al ./DerivedData/llpeak/Build/Products/${src}/${name}
cp ./DerivedData/llpeak/Build/Products/${src}/${name} /usr/local/opt/

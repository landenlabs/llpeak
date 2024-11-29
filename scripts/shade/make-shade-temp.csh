
# 
#  Shade Temperature
#

/usr/local/opt/llpeak -config=~/patent-2021/data/temperature-palette.json -shade ~/patent-2021/data/temp/

rename -N 0001 -X -e '$_ = "Temp-$N"' *.png
# Build movie fast, but enourmous output
#ffmpeg -framerate 10 -i 'Temp-%04d.png' -r 5 -y -vcodec png test.mp4

# Create full movie, not frame rate reduction, default compression
# ffmpeg -framerate 10 -i 'Temp-%04d.png' -r 10 -y temp-full.mp4
ffmpeg -framerate 10 -i 'Temp-%04d.png' -r 10 -y temp-full.avi


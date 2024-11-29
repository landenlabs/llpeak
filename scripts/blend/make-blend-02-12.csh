
#
# Blend Radar
#

/usr/local/opt/llpeak -config=~/patent-2021/data/radar-palette.json -blend  ~/patent-2021/data/radar/2021_07/02-12/

rename -N 0001 -X -e '$_ = "Radar-$N"' *.png

# Full screen
ffmpeg -i 'Radar-%4d.png' -framerate 10 -y -vcodec png test.mp4
ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-blue.avi
rm test.mp4

# ffmpeg -i 'Radar-%4d.png' -filter:v "crop=4400:2800:1000:0" -framerate 10 -y -vcodec png test.mp4

ffmpeg -i 'Radar-%4d.png' -filter:v "crop=1270:1040:4230:360" -framerate 10 -y -vcodec png test.mp4
ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-blue-ur.mp4
rm test.mp4

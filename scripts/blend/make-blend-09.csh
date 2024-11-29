
#
# Blend Radar 2021-09
#

/usr/local/opt/llpeak -config=~/patent-2021/data/radar-palette.json -blend  ~/patent-2021/data/radar/2021_09/

rename -N 0001 -X -e '$_ = "Radar-$N"' *.png

# Full screen
ffmpeg -i 'Radar-%4d.png' -framerate 10 -y -vcodec png test.mp4
ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-blue.avi
rm test.mp4

# Upper right
ffmpeg -i 'Radar-%4d.png' -filter:v "crop=2500:1500:3100:0" -framerate 10 -y -vcodec png test.mp4
ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-blue-ur.mp4
ffmpeg -i radar-blue-ur.mp4 radar-blue-ur.avi
rm test.mp4


mkdir hardlinks
rename -L -N 001 -X -e '$_ = "Radar-$N"' *.png
mv Radar-* hardlinks/
cd hardlinks/

ffmpeg -i Radar-%3d.png -filter:v "crop=1600:1800:600:700" -framerate 10 -y -vcodec png test.mp4
ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-blue-ll.mp4

ffmpeg -i radar-blue-ll.mp4 radar-blue-ll.avi
ln radar-blue-ll.avi radar-2021-07-17xx-ll-std.avi
 
ffmpeg -i radar-2021-07-17xx-ll-std.avi -i ~/patent-2021/blend/radar/2021_07/17-23/radar-2021-07-17xx-ll-blend.avi -filter_complex '[0:v][1:v]hstack=inputs=2[v]' -map '[v]' side-by-side-ll.avi

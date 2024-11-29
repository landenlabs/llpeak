
mkdir hardlinks
rename -L -N 001 -X -e '$_ = "Radar-$N"' *.png
mv Radar-* hardlinks/
cd hardlinks/

ffmpeg -i Radar-%3d.png -filter:v "crop=1270:1040:4230:360" -framerate 10 -y -vcodec png test.mp4
# ffmpeg -i test.mp4 -filter_complex "[0]split=2[bg][fg];[bg]drawbox=c=blue@1:replace=1:t=fill[bg];[bg][fg]overlay=format=auto" radar-blue-ur.mp4

# ffmpeg -i test.mp4 -filter_complex "[0:v]chromakey=0xfefefe:0.05:0[out]" -map "[out]" foo.mp4
# ffmpeg -i test.mp4 -filter_complex '[0:v]colorkey=0xfefefe[out]' -map "[out]" foo.mp4

convert -size 1270x1040 xc:blue blue.png
ffmpeg -y -i blue.png -i test.mp4 -filter_complex '[1:v]colorkey=0xfefefe[ckout];[0:v][ckout]overlay=format=auto'  radar-blue2.mp4
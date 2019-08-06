# Black Grayscale Line Following Example


import sensor, image, time , math ,struct ,json,pyb,tool,find_line
from pyb import UART,LED
from struct import pack, unpack

# Tracks a black line. Use [(128, 255)] for a tracking a white line.
GRAYSCALE_THRESHOLD = [(0, 64)]
#设置阈值，如果是黑线，GRAYSCALE_THRESHOLD = [(0, 64)]；
#如果是白线，GRAYSCALE_THRESHOLD = [(128，255)]


ROIS = [ # [ROI, weight]
        (0, 100, 160, 30, 0.1), # You'll need to tweak the weights for you app
        (0, 050, 160, 30, 0.5), # depending on how your robot is setup.
        (0, 000, 160, 30, 0.7)
       ]
#roi代表三个取样区域，（x,y,w,h,weight）,代表左上顶点（x,y）宽高分别为w和h的矩形，
#weight为当前矩形的权值。注意本例程采用的QQVGA图像大小为160x120，roi即把图像横分成三个矩形。
#三个矩形的阈值要根据实际情况进行调整，离视野最近的矩形权值要最大，
#如上图的最下方的矩形，即(0, 100, 160, 20, 0.7)

# Compute the weight divisor (we're computing this so you don't have to make weights add to 1).
weight_sum = 0 #权值和初始化
for r in ROIS: weight_sum += r[4] # r[4] is the roi weight.
#计算权值和。遍历上面的三个矩形，r[4]即每个矩形的权值。

class line_info(object):
    angle_err = 0  #角度值
    rho_err = 0  # 直线与图像中央的距离


def partition_find_line(picture,ctrl,uart):

    centroid_sum = 0
    #利用颜色识别分别寻找三个矩形区域内的线段
    for r in ROIS:
        img = picture
        blobs = img.find_blobs(GRAYSCALE_THRESHOLD, roi=r[0:4], merge=True)
        # r[0:4] is roi tuple.
        #找到视野中的线,merge=true,将找到的图像区域合并成一个

        #目标区域找到直线
        if blobs:
            # Find the index of the blob with the most pixels.
            most_pixels = 0
            largest_blob = 0
            for i in range(len(blobs)):
            #目标区域找到的颜色块（线段块）可能不止一个，找到最大的一个，作为本区域内的目标直线
                if blobs[i].pixels() > most_pixels:
                    most_pixels = blobs[i].pixels()
                    #merged_blobs[i][4]是这个颜色块的像素总数，如果此颜色块像素总数大于
                    largest_blob = i

            # Draw a rect around the blob.
            img.draw_rectangle(blobs[largest_blob].rect())
            #将此区域的像素数最大的颜色块画矩形和十字形标记出来
            img.draw_cross(blobs[largest_blob].cx(),
                           blobs[largest_blob].cy())

            centroid_sum += blobs[largest_blob].cx() * r[4] # r[4] is the roi weight.
            #计算centroid_sum，centroid_sum等于每个区域的最大颜色块的中心点的x坐标值乘本区域的权值

    center_pos = (int)((centroid_sum / weight_sum)-90) # 求取中心直线

    print("Angle: %f" % (center_pos))

    line_info.angle_err = pack('h',center_pos) #short 类型 打包
    line_info.angle_err = bytearray(line_info.angle_err)

    line_info.rho_err = pack('h',0) #short 类型
    line_info.rho_err = bytearray(line_info.rho_err)
    uart.write(find_line.pack_line_data(line_info,ctrl,1))# 标志位为1

        #else:
            #uart.write(find_line.pack_line_data(line_info,ctrl,0))# 标志位为1


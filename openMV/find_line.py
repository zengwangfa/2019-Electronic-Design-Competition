RED_THRESHOLD = (0, 50, -128, 30, -30, 31)#(5, 89, 35, 75, -8, 50)#(28, 86, 16, 82, 14, 63) # Grayscale threshold for dark things...

BLACK_WHITE_THRESHOLD = (0, 100) # Grayscale threshold for dark things...
import sensor, image, time , math ,struct ,json,pyb,tool
from pyb import UART,LED
from struct import pack, unpack


LEDG = pyb.LED(3)
class line_info(object):
    angle_err = 0  #角度值
    rho_err = 0  # 直线与图像中央的距离






# 色块检测数据打包
def pack_line_data(line_info,ctrl,flag):

    #---包头------功能位----数据个数位---------------------------------数据包------------------------------------------------累加和校验
    #【AA 55】    【01】     【0x0A】      【角度值高8位、角度值低8位、直线与图像中央的距离、res】     【sum】

    datalist = [0xAA,0x55,ctrl.WorkMode,0x0A,
    ctrl.Threshold_index,
    line_info.angle_err[1],
    line_info.angle_err[0],
    line_info.rho_err[1],
    line_info.rho_err[0],
    0x00,0x00,
    0x00,0x00,
    0x00,flag] # 定义返回数据列表

    print(datalist)
    datalist.append(tool.sum_checkout(datalist))# 在list尾插入累加和校验
    data = bytearray(datalist)
    #for res in data:
        #print("%x" %res)

    return data


line_info = line_info()


def find_line(picture,ctrl,uart):

    img = picture.binary([RED_THRESHOLD])#二值化
    line = img.get_regression([(100,100,0,0,0,0)], robust = True) # get_regression 线性回归计算,这一计算通过最小二乘法进行
    if (line):
        img.draw_line(line.line(), color = 127) # 画一条绿线
        rho_err = int(abs(line.rho()) - sensor.width()/2) #计算一条直线与图像中央的距离

        #xy轴的角度变换
        theta_err = line.theta() # 0~180
        if line.theta()> 90:    #左为负
            angle_err = line.theta() -180
        else:
            angle_err = line.theta()  #右为正

        line_info.rho_err = pack('h',rho_err) #char 类型
        line_info.rho_err = bytearray(line_info.rho_err)
        line_info.angle_err = pack('h',angle_err) #short 类型 其类型为一个列表
        line_info.angle_err  = bytearray(line_info.angle_err)
        uart.write(pack_line_data(line_info,ctrl,1))#未识别 标志位为1
        LEDG.on()
    else:

        uart.write(pack_line_data(line_info,ctrl,0))#未识别 标志位为0
        LEDG.off()







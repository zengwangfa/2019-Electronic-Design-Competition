import sensor, image, time, math , pyb
from pyb import LED
LEDB = pyb.LED(1)
LEDR = pyb.LED(2)
LEDG = pyb.LED(3)
flag = 0
def InitSuccess_LED():
    for i in range(2):
        LEDG.on()
        time.sleep(100)
        LEDG.off()
        time.sleep(100)
def find_max(blobs):
    max_size = 0
    for blob in blobs:
        if blob.w() * blob.h() > max_size:
            max_blob = blob
            max_size = blob.w()*blob.h()
    return max_blob
def sum_checkout(data_list):
    data_sum = 0
    for temp in data_list:
        data_sum += temp
    return (data_sum)
def get_length(blob):
    K_value = 1800
    blob_R = (blob.w()+blob.h())/2
    length = K_value/blob_R
    length = length /((blob.density()/2)+0.5)
    return int(length)
def pack_blob_data(blob,ctrl,Threshold_index,flag):
    datalist = [0xAA,0x55,ctrl.WorkMode,0x0A,
    Threshold_index,
    blob.count(),
    int(blob.density()*100),
    get_length(blob),
    blob.cx()>>8,blob.cx(),
    blob.cy()>>8,blob.cy(),
    0x00,flag]
    print(datalist)
    datalist.append(sum_checkout(datalist))
    data = bytearray(datalist)
    for res in data:
        a = 0
        print("%x" %res)
    return data
def pack_no_blob_data(ctrl,flag):
    datalist = [0xAA,0x55,ctrl.WorkMode,0x0A,
    ctrl.Threshold_index,
    0x00,
    0x00,
    0x00,
    0x00,160,
    0x00,120,
    0x00,flag]
    print(datalist)
    datalist.append(sum_checkout(datalist))
    data = bytearray(datalist)
    for res in data:
        a = 0
    return data
def single_blob_LEDStatus(blobs,ctrl):
    if blobs:
        if ctrl.Threshold_index == 0:
            LEDR.on()
        elif ctrl.Threshold_index == 1:
            LEDG.on()
        elif ctrl.Threshold_index == 2:
            LEDB.on()
    else:
        LEDR.off()
        LEDG.off()
        LEDB.off()
def single_blob(img,ctrl,thresholds,threshold_index,uart):
    blobs = img.find_blobs([thresholds[threshold_index]], pixels_threshold=20, area_threshold=10, merge=True)
    if blobs:
        flag = 0x01
        max_blob = find_max(blobs)
        img.draw_rectangle(max_blob.rect())
        img.draw_cross(max_blob.cx(), max_blob.cy())
        uart.write(pack_blob_data(max_blob,ctrl,threshold_index,flag))
    else:
        flag = 0x00
        print("No Find")
        uart.write(pack_no_blob_data(ctrl,flag))
    single_blob_LEDStatus(blobs,ctrl)

# 把除校验位的数据包全部累加 生成 累加和校验位
def sum_checkout(data_list):
    data_sum = 0
    for temp in data_list:
        data_sum += temp
    return (data_sum) #返回16进制
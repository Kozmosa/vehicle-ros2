import os
import logging
import time
from datetime import datetime

# get date and time as yyyy-mm-dd-hh-mm
now = datetime.now()
datetimeStr = now.strftime("%Y-%m-%d_%H:%M:%S")

logging.basicConfig(filename=f'{datetimeStr}.log',
                    level=logging.INFO)

logging.info("UART Sender start running.")

# connect with com port
comPath = input('请输入要连接的设备的COM路径：')


import sys
import cv
import cv2
import numpy
import base64
from websocket import create_connection
#connection to websocket server
ws = create_connection('ws://localhost:9090/ws')
#JPEG encode_param
encode_param=[int(cv2.IMWRITE_JPEG_QUALITY),30]
#select first VideoCapture 
capture = cv2.VideoCapture(0)
#set WebCame Frame Width
capture.set(3,640)
#set WebCam Frame Height
capture.set(4,480)
#defind function
def repeat():
    # Capture frame-by-frame
    ret, frame = capture.read()
    cv2.imshow('camera',frame)
    #encode to jpg
    if (type(ret)!=type(None)):
        result,imgencode=cv2.imencode('.jpg',frame,encode_param )
    # to numpy array
        data = numpy.array(imgencode)
    # tostring
        stringData = data.tostring()
    #encode to base64
        encoded = base64.b64encode(stringData)
    #send
        ws.send(encoded)
    #sleep 60millisecond
        cv2.waitKey(60)
while True:
    repeat()

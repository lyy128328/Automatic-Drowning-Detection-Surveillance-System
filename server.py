
#NOTE: You will have to update the ip address in index.html

import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.template
import base64
import cStringIO
from PIL import Image
import cv
import cv2
import time
import os
import Tkinter
import sys
import ImageTk
import numpy
def button_click_exit_mainloop (event):
    event.widget.quit() 
class MainHandler(tornado.web.RequestHandler):
  def get(self):
    loader = tornado.template.Loader(".")
    self.write(loader.load("index.html").generate())

class WSHandler(tornado.websocket.WebSocketHandler):
  
  def open(self):
    print 'connection opened...'
    self.write_message("The server says: 'Hello'. Connection was accepted.")

  def on_message(self, message):

	
# 	image_string = cStringIO.StringIO(base64.b64decode(message))
# 	image = Image.open(image_string)
# 	image.show()

	
	decoded = base64.b64decode(message)
	numpyArr = numpy.fromstring(decoded,dtype='uint8')
	img = cv2.imdecode(numpyArr,1)

  	cv2.imshow('image',img)
 	cv2.waitKey(60)

#	print img.format, img.size, img.mode


  def on_close(self):
    print 'connection closed...'

application = tornado.web.Application([
  (r'/ws', WSHandler),
  (r'/', MainHandler),
  (r"/(.*)", tornado.web.StaticFileHandler, {"path": "./resources"}),
])

if __name__ == "__main__":
  application.listen(9090)
  tornado.ioloop.IOLoop.instance().start()


import serial
import array

class ICSC:
	port = 0
	station = 0

	rxstate = 0
	rxsoh = 0
	rxdestination = 0
	rxsource = 0
	rxcommand = 0
	rxlen = 0
	rxsot = 0
	rxcs = 0
	rxdata = array.array('B')
	rxgot = 0
	rxcsin = 0

	functions = {}

	def begin(self, port, baud, station):
		self.port = serial.Serial(port = port, baudrate = baud, timeout = 0)
		self.station = station
		self.port.close()
		self.port.open()
		self.functions[5] = self.respondToPing


	def send(self, destination, command, data):

		sendpacket = array.array('B',
			[1, 1, 1, 1, 1,
			destination,
			self.station,
			command,
			len(data),
			2]
		)

		cs = destination
		cs += self.station
		cs += command
		cs += len(data)

		for d in data:
			sendpacket.append(d)
			cs += d

		sendpacket.append(3)
		sendpacket.append(cs % 256)
		sendpacket.append(4)

		self.port.write(sendpacket.tostring())

	def process(self):
		try:
			char = self.port.read()
		except:
			return

		if (self.rxstate == 0):
			self.rxsoh = self.rxdestination
			self.rxdestination = self.rxsource
			self.rxsource = self.rxcommand
			self.rxcommand = self.rxlen
			self.rxlen = self.rxsot
			self.rxsot = char

			if (
				(self.rxsoh == 1) and 
				(self.rxsot == 2) and
				(self.rxdestination == self.station)
			):
				self.rxcs = self.rxdestination
				self.rxcs += self.rxsource
				self.rxcs += self.rxcommand
				self.rxcs += self.rxlen
				self.rxdata = array.array('B')
				self.rxcsin = 0
				self.rxgot = 0
				if (self.rxlen == 0):
					self.rxstate = 2
				else:
					self.rxstate = 1
		elif (self.rxstate == 1):
			self.rxdata.append(char)
			self.rxcs += char
			self.rxgot += 1
			if (self.rxgot == self.rclen):
				self.rxstate = 2
		elif (self.rxstate == 2):
			if (char == 3):
				self.rxstate = 3
			else:
				self.rxstate = 0
		elif (self.rxstate == 3):
			self.rxcsin = char
			self.rxstate = 4
		elif (self.rxstate == 4):
			if (char == 4):
				if (self.rxcs % 256 == self.rxcsin):
					try:
						self.functions[self.rxcommand](self.rxsource, self.rxcommand, self.rxdata)
					except:
						pass
			self.rxstate = 0

	def addCommand(c, f):
		functions[c] = f

	def respondToPing(src, cmd, len):
		self.send(src, 6, [])

			


#foo = ICSC()
#foo.begin("/dev/ttyUSB0", 2000000, 99)
#foo.send(100, 84, [0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x24, 0x00, 0x00, 0xf8, 0xff, 0xff, 70, 111, 111, 0x00])

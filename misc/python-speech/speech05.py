import sys
import os
buf_arg = 0
if sys.version_info[0] == 3:
    os.environ['PYTHONUNBUFFERED'] = '1'
    buf_arg = 1
sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', buf_arg)
sys.stderr = os.fdopen(sys.stderr.fileno(), 'w', buf_arg)

import speech_recognition as sr
r = sr.Recognizer()
r.dynamic_energy_threshold = False
r.energy_threshold = 400
value=1
while(value):
	try:
		with sr.Microphone() as source:
			audio = r.listen(source, timeout=2)
			try:
				print("You said " + r.recognize_google(audio))
				sys.stdout.flush()

			except:                           
				print("Could not understand audio")
				sys.stdout.flush()

	except:
		print("Time out")
		sys.stdout.flush()

else:
	print("done")
    
    
import sys
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
				
			except OSError as err:
				print("OS error: {0}".format(err))
				
			except:                           
				print("Could not understand audio")    
				print("Unexpected error:", sys.exc_info()[0])
				sys.stdout.flush()

	except:
		print("Time out")
		sys.stdout.flush()

else:
	print("done")
    
    
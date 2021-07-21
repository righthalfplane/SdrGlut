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
				print(r.recognize_google(audio))
				sys.stdout.flush()
				
			except OSError as err:
				print("OS error: {0}".format(err))
				
			except:                           
				print("timeout")    
		#		print("Unexpected error:", sys.exc_info()[0])
		#		sys.stdout.flush()

	except:
		print("timeout")
		sys.stdout.flush()

else:
	print("done")
    
    
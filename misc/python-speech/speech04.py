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
			except:                           
				print("Could not understand audio")
	except:
		print("Time out")
else:
	print("done")
    
    
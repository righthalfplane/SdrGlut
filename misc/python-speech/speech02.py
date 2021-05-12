import speech_recognition as sr
r = sr.Recognizer()

for x in range(5):
	with sr.Microphone() as source:
		audio = r.listen(source)
		try:
			print("You said " + r.recognize_google(audio))
		except:                           
			print("Could not understand audio")
else:
	print("done")
    
    
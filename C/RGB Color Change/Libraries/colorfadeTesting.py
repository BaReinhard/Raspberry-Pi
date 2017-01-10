## Program:		RGB COLOR CHANGE
## Programmer:	Brett Reinhard
## Date:		11/09/2016
## Board:		Raspberry Pi, Python
## Imports Necessary Libraries for PWM
import RPi.GPIO as GPIO
import time
## Initializes Pin Numbers for  Raspbery Pi GPIO
groundPin = 18
ledPinRed = 23
ledPinBlue = 24
ledPinGreen = 25
## Sets Pin Numbers to BCM instead of GPIO.BOARD
GPIO.setmode(GPIO.BCM)
## Sets Up Previously initializes pins as OUTPUT 
GPIO.setup(ledPinRed, GPIO.OUT)
GPIO.setup(ledPinBlue, GPIO.OUT)
GPIO.setup(ledPinGreen, GPIO.OUT)
GPIO.setup(groundPin, GPIO.OUT)
## Sets the Pins as an instance of PWM and sets initial Frequency
## Could use other frequency, used 255 for nostalgic purposes
pwmRed = GPIO.PWM(ledPinRed,255)
pwmBlue = GPIO.PWM(ledPinBlue,255)
pwmGreen = GPIO.PWM(ledPinGreen,255)
## Sets ground as Low
GPIO.output(groundPin,GPIO.LOW)
## Sets all Duty Cycles at 0, range is (0-100)
pwmRed.start(0)
pwmGreen.start(0)
pwmBlue.start(0)
## Defines a function called RGB() and changes duty cycles based on parameters passed
def RGB(red, green, blue):
	pwmRed.ChangeDutyCycle(red)
	pwmGreen.ChangeDutyCycle(green)
	pwmBlue.ChangeDutyCycle(blue)
	return
## Outputs to Terminal, so user knows its started and how to exit
print("And so it begins... Press CTRL+C to exit")
try:
	while 1:## Starts a while loop to continue forever
		## Performs the color fade
		for i in range(0,100):
			RGB(100,i,0)
			time.sleep(0.1) ## Important to see the fade
			## Without Time.Sleep it appears Red always
		for i in range(100,0,-1):
			RGB(i,100,0)
			time.sleep(0.1)
		for i in range(0,100):
			RGB(0,100,i)
			time.sleep(0.1)
		for i in range(100,0,-1):
			RGB(0,i,100)
			time.sleep(0.1)
		for i in range(0,100):
			RGB(i,0,100)
			time.sleep(0.1)
		for i in range(100,0,-1):
			RGB(100,0,i)
			time.sleep(0.1)
		time.sleep(1)

except KeyboardInterrupt:
	pwmRed.stop()
	pwmGreen.stop()
	pwmBlue.stop()
	GPIO.cleanup()


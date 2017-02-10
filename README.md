# paper-display

Show tweets or whatever you'd like on an E-ink display on your wall

![Frame](https://github.com/bwhitman/paper-display/blob/master/images/IMG_2109.jpg?raw=true)

## Parts needed

There's only a few parts you need to build this:

* A serial e-paper display. I am using a Waveshare 800x600 display, which you can get in a number of places. [Amazon](https://www.amazon.com/Waveshare-4-3inch-e-Paper-Resolution-Electronic/dp/B00VV5IMN0/) has them for $58 with Prime shipping.
* An ESP8266 board with pins broken out for serial and RST. I like the [Sparkfun Thing](https://www.sparkfun.com/products/13231), it's $16 and works great. But the internet is riddled with these and you may already have something similar. 
* If your board needs it for programming and you don't already have one, a serial to USB connector. The Thing requires one, and [Sparkfun sells one that fits neatly on the board](https://www.sparkfun.com/products/9873) for $15
* A li-po battery. I'm using [this 2000mAh one from Sparkfun](https://www.sparkfun.com/products/8483) ($13) but you can use almost anything. The 2000mAh should last over a year between charges. You may want to measure the thickness of the battery you want to make sure it fits in the frame.
* Some hookup wire (just one short one), solder, soldering iron, header pins if you need it for the serial programmer.
* A frame!


## Hookup

The ePaper Display (hereafter `EPD`) comes with a convenient 6-pin wire harness that snaps in on the display and comes out to female header pins. While you're ensuring everything is hooked up right, I would solder male header pins to the ESP8266 (hereafter `MCU`) so that you can unplug and plug the `EPD` using jumper cables while debugging and development. Also, the same pins that are used for programming on the Sparkfun Thing are needed for the `EPD`, so you don't want to permanently attach the `EPD` in a way that prevents use of the pins until you are certain. 

![EPD](https://github.com/bwhitman/paper-display/blob/master/images/K9PSMkF.png?raw=true)

Wire the `EPD` to the `MCU` as so:

* **RST** to nowhere, you can chop this off at the plug end if you'd like
* **WAKE_UP** to `pin 4`
* **DIN** to `TXO`
* **DOUT** to `RXI`
* **GND** to `GND`
* **VCC** to `VIN`

You also need to run a short wire between `XPD` and `DTR` on the `MCU`. This allows the `MCU` to enter and wake up from "deep sleep."

Lastly, connect the battery to the `MCU`. The micro USB jack on the `MCU` charges the battery. Let it charge fully once, and it should be good for over a year on no external power if running correctly.

![Frame](https://github.com/bwhitman/paper-display/blob/master/images/IMG_1491.jpg?raw=true)

## Web service

The `MCU` will connect to a webservice you define every time it wakes up. I made a very simple one, hosted for free on Google App Engine, that retrieves a random tweet from the last 200 of a certain username. I've included the source code to that service in this repository as `main.py`. You can host this service anywhere you'd like: but Google App Engine was particularly easy to get going and hasn't cost me anything yet. The web service just needs to return plain ASCII text (no HTML) that will fit on the `EPD` whenever the URL is accessed. 

My web service is pretty straightforward, using the Tweepy API. If you want your frame to return tweets like this one, you can see in `main.py` that after setting up a Twitter API account and pasting in the authentication strings, the request handler is just:

```python
@app.route('/message')
def message():
	tweets = api.user_timeline(screen_name='cookbook', count=200, exclude_replies = True, include_rts = False)
	shuffle(tweets)
	text = tweets[0].text
	text = unidecode(text)
	text = text.replace('&amp;','&')
	text = text.replace('&lt;','<')
	text = text.replace('&gt;','>')
	text = text.replace('&quot;','"')
	text = text.replace('&apos;',"'")
	text = re.sub(r"http.*? ", "", text+" ")[:-1]
	text = "\n".join(textwrap.wrap(text, 25))
	return text
```

Note I'm removing XML cruft like "&amp;", deleting links, wrapping the text to 25 columns using the `textwrap` python library, flattening the codespace to ASCII using `unidecode`, and not including replies or retweets using Tweepy. The simpler the text your webservice returns the less chance the `EPD` will have issues displaying it.


## Firmware

First set up the MCU for Arduino. [Sparkfun has a great tutorial for the Thing.](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon) I'm sure other boards have similar ones. 

You'll next want to install the ArduinoEpd library. It's hard to find so I've included it in this repository. 

Load the `paper_display.ino` file from this repository into Arduino. Create a new `auth.h` file in the same project, and fill it with your WiFi credentials as well as your web services' host name and full URL path (without the http://). For example:

```C++
const char WiFiSSID[] = "mywifi";
const char WiFiPSK[] = "password";
const char hostname[] = "my-web-service-112117.appspot.com";
const char path[] = "my-web-service-112117.appspot.com/message";
```

Compile and program your sketch to the `MCU`. If connected properly, the `EPD` should flash, the `MCU`'s LED should light briefly and then you will see the contents of your web service on the `EPD`. At this point, everything will turn off and wait for `sleepTimeS` seconds until it starts up again. Since e-ink is still visible when it's not powered, you've got a incredibly low power always-on display!

![Frame](https://github.com/bwhitman/paper-display/blob/master/images/IMG_2107.jpg?raw=true)

At this point, if you've been using jumper cables, I would take the time to solder them onto the pins or headers of the `MCU` so that they don't fall out. Make sure to solder them in a way that you can still access the pins for programming. `TXO`, `RXI` and `DTR` all are needed by both the frame and the programmer. 

## Put it together

I employed the services of a profressional framer to put everything in a nice deep wooden frame we could hang on the wall. Use your imagination here. All the components should fit pretty neatly behind a 4 x 6 inch frame.

![Frame](https://github.com/bwhitman/paper-display/blob/master/images/IMG_2104.jpg?raw=true)





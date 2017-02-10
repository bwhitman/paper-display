from flask import Flask
from flask import request
app = Flask(__name__)
app.config['DEBUG'] = True
from random import shuffle
from unidecode import unidecode
import tweepy
import textwrap
import re

# All your twitter API stuff goes here
consumer_key = ""
consumer_secret = ""
access_token = ""
access_token_secret = ""
auth = tweepy.OAuthHandler(consumer_key, consumer_secret)
auth.set_access_token(access_token, access_token_secret)
api = tweepy.API(auth)

# Get a random tweet from a twitter account
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


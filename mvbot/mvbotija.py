#!/usr/bin/python
# -*- coding: utf-8 -*-
import tweepy
from tweepy.streaming import StreamListener
from tweepy import OAuthHandler
from tweepy import Stream
import json
import sys
import random
import httplib
import time
import math

# Go to http://dev.twitter.com and create an app. 
# The consumer key and secret will be generated for you after
consumer_key=""
consumer_secret=""

# After the step above, you will be redirected to your app's page.
# Create an access token under the the "Your access token" section
access_token="-"
access_token_secret=""

track_strings = ['me aburro', 'vaya aburrimiento', 'menudo aburrimiento']

def ratio():
    if random.randrange(1, 10000) < 100:
        return True
    else:
        return False



class StdOutListener(StreamListener):
    """ A listener handles tweets are the received from the stream. 
    This is a basic listener that just prints received tweets to stdout.

    """
    count = 0
    total_likes = 0

    def thread_rating_v1(self, t):
        rating =   math.log(int(t['votes']),2) - \
                math.log((float(self.total_likes)),(float(t['votes'])/2)+1) - \
                math.pow((time.time() - int(t['date']))/(24*3600), 1.5)
        return round(rating, 3)        
    

    def getThread(self):
        try:
            conn = httplib.HTTPConnection('www.mediavida.com:80')
            conn.request('GET', '/util/pop.php')
            threads = json.loads(conn.getresponse().read())
            conn.close()
            self.total_likes = 0
            top = {}
            for thread in threads:
                self.total_likes += int(thread['votes'])
            for thread in threads:
                top[self.thread_rating_v1(thread)] = thread

            threads_s = sorted(top)
            threads_s = list(reversed(threads_s))
            thread = top[threads_s[0]]
            msg = 'Te aburres?, echa un vistazo a este hilo de Mediavida: '+ 'http://www.mediavida.com/vertema.php?tid=%s  ' % ( str(thread['tid']))
        except:
            print "getThread peto"
            msg = ''
        finally:
            return msg


    def reply(self, data):
        msg = self.getThread()
        if len(msg) == 0:
            pass
        else:
            #self.api.update_status('@%s %s' % (data['user']['screen_name'], self.getThread()), data['id'])
            print '\033[91mTweet sent\033[0m: @%s %s' % (data['user']['screen_name'], self.getThread())



    def on_data(self, data):
        data = json.loads(data)
        self.count += 1
        print "[%i] \033[92m@%s\033[0m : %s" %(self.count, data['user']['screen_name'], data['text'])
        if (len(data['text']) < 30 and self.count > 500)\
                or self.count > 1000:
            if not data['text'].startswith('RT'):
                self.count = 0
                self.reply(data)
        return True

    def on_error(self, status):
        print status

if __name__ == '__main__':

    while True:

        try:
            l = StdOutListener()
            auth = OAuthHandler(consumer_key, consumer_secret)
            auth.set_access_token(access_token, access_token_secret)

            api = tweepy.API(auth)
            l.api = api

            stream = Stream(auth, l)    

            stream.filter(track=track_strings)

            time.sleep(999999999999999)

        except Exception, e:
            print e
            pass



'''
data = {"place":null,"in_reply_to_status_id":null,"text":"Me aburro, en el medico de ma\u00f1ana y pa clases.... #ADV","in_reply_to_user_id_str":null,"in_reply_to_screen_name":null,"favorited":false,"contributors":null,"truncated":false,"source":"\u003Ca href=\"http:\/\/twitter.com\/download\/android\" rel=\"nofollow\"\u003ETwitter for Android\u003C\/a\u003E","retweet_count":0,"coordinates":null,"geo":null,"retweeted":false,"created_at":"Thu Aug 02 06:52:52 +0000 2012","user":{"contributors_enabled":false,"geo_enabled":false,"profile_text_color":"333333","screen_name":"serginn96","default_profile_image":false,"notifications":null,"profile_background_image_url":"http:\/\/a0.twimg.com\/images\/themes\/theme1\/bg.png","location":"","following":null,"favourites_count":7,"profile_link_color":"0084B4","show_all_inline_media":false,"description":"","friends_count":172,"profile_background_color":"C0DEED","profile_background_tile":false,"profile_background_image_url_https":"https:\/\/si0.twimg.com\/images\/themes\/theme1\/bg.png","listed_count":0,"follow_request_sent":null,"verified":false,"time_zone":null,"profile_sidebar_fill_color":"DDEEF6","profile_image_url_https":"https:\/\/si0.twimg.com\/profile_images\/1607304691\/IMG00068-20111001-2003_normal.jpg","followers_count":143,"protected":false,"profile_image_url":"http:\/\/a0.twimg.com\/profile_images\/1607304691\/IMG00068-20111001-2003_normal.jpg","default_profile":true,"statuses_count":571,"created_at":"Tue Oct 25 10:02:12 +0000 2011","profile_sidebar_border_color":"C0DEED","name":"Sergio Rodr\u00edguez ","is_translator":false,"url":null,"id":397916587,"id_str":"397916587","lang":"es","profile_use_background_image":true,"utc_offset":null},"in_reply_to_user_id":null,"id":230919073763037184,"in_reply_to_status_id_str":null,"id_str":"230919073763037184","entities":{"urls":[],"user_mentions":[],"hashtags":[{"text":"ADV","indices":[50,54]}]}}
'''

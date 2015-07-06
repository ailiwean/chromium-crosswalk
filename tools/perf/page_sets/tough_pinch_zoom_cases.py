# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from telemetry.page import page as page_module
from telemetry.page import shared_page_state
from telemetry import story


class ToughPinchZoomCasesPage(page_module.Page):

  def __init__(self, url, page_set, name=''):
    super(ToughPinchZoomCasesPage, self).__init__(
        url=url, page_set=page_set, name=name,
        shared_page_state_class=shared_page_state.SharedDesktopPageState,
        credentials_path = 'data/credentials.json')
    self.archive_data_file = 'data/tough_pinch_zoom_cases.json'

  def RunPageInteractions(self, action_runner):
    with action_runner.CreateGestureInteraction('PinchAction'):
      action_runner.PinchPage()


class GoogleSearchPage(ToughPinchZoomCasesPage):

  """ Why: top google property; a google tab is often open. """

  def __init__(self, page_set):
    super(GoogleSearchPage, self).__init__(
      url='https://www.google.com/#hl=en&q=barack+obama',
      page_set=page_set)

  def RunNavigateSteps(self, action_runner):
    super(GoogleSearchPage, self).RunNavigateSteps(action_runner)
    action_runner.WaitForElement(text='Next')


class GmailPage(ToughPinchZoomCasesPage):

  """ Why: productivity, top google properties """

  def __init__(self, page_set):
    super(GmailPage, self).__init__(
      url='https://mail.google.com/mail/',
      page_set=page_set)

    self.credentials = 'google'

  def RunNavigateSteps(self, action_runner):
    super(GmailPage, self).RunNavigateSteps(action_runner)
    action_runner.WaitForJavaScriptCondition(
        'window.gmonkey !== undefined &&'
        'document.getElementById("gb") !== null')


class GoogleCalendarPage(ToughPinchZoomCasesPage):

  """ Why: productivity, top google properties """

  def __init__(self, page_set):
    super(GoogleCalendarPage, self).__init__(
      url='https://www.google.com/calendar/',
      page_set=page_set)

    self.credentials = 'google'

  def RunNavigateSteps(self, action_runner):
    super(GoogleCalendarPage, self).RunNavigateSteps(action_runner)
    action_runner.Wait(2)

  def RunPageInteractions(self, action_runner):
    with action_runner.CreateGestureInteraction('PinchAction'):
      action_runner.PinchPage(left_anchor_ratio=0.1, top_anchor_ratio=0.3)


class GoogleImageSearchPage(ToughPinchZoomCasesPage):

  """ Why: tough image case; top google properties """

  def __init__(self, page_set):
    super(GoogleImageSearchPage, self).__init__(
      url='https://www.google.com/search?q=cats&tbm=isch',
      page_set=page_set)

    self.credentials = 'google'


class GooglePlusPage(ToughPinchZoomCasesPage):

  """ Why: social; top google property; Public profile; infinite scrolls """

  def __init__(self, page_set):
    super(GooglePlusPage, self).__init__(
      url='https://plus.google.com/+BarackObama/posts',
      page_set=page_set)

    self.credentials = 'google'

  def RunNavigateSteps(self, action_runner):
    super(GooglePlusPage, self).RunNavigateSteps(action_runner)
    action_runner.WaitForElement(text='Home')

  def RunPageInteractions(self, action_runner):
    with action_runner.CreateGestureInteraction('PinchAction'):
      action_runner.PinchElement(
          selector='[id="110031535020051778989-tab-bar"]')


class YoutubePage(ToughPinchZoomCasesPage):

  """ Why: #3 (Alexa global) """

  def __init__(self, page_set):
    super(YoutubePage, self).__init__(
      url='http://www.youtube.com',
      page_set=page_set)

    self.credentials = 'google'

  def RunNavigateSteps(self, action_runner):
    super(YoutubePage, self).RunNavigateSteps(action_runner)
    action_runner.Wait(2)

class BlogSpotPage(ToughPinchZoomCasesPage):

  """
  Why: #11 (Alexa global), google property; some blogger layouts have infinite
  scroll but more interesting
  """

  def __init__(self, page_set):
    super(BlogSpotPage, self).__init__(
      url='http://googlewebmastercentral.blogspot.com/',
      page_set=page_set, name='Blogger')

  def RunNavigateSteps(self, action_runner):
    super(BlogSpotPage, self).RunNavigateSteps(action_runner)
    action_runner.WaitForElement(text='accessibility')


class FacebookPage(ToughPinchZoomCasesPage):

  """ Why: top social,Public profile """

  def __init__(self, page_set):
    super(FacebookPage, self).__init__(
      url='http://www.facebook.com/barackobama',
      page_set=page_set, name='Facebook')
    self.credentials = 'facebook'

  def RunNavigateSteps(self, action_runner):
    super(FacebookPage, self).RunNavigateSteps(action_runner)
    action_runner.WaitForElement(text='About')


class LinkedinPage(ToughPinchZoomCasesPage):

  """ Why: #12 (Alexa global),Public profile """

  def __init__(self, page_set):
    super(LinkedinPage, self).__init__(
      url='http://www.linkedin.com/in/linustorvalds',
      page_set=page_set, name='LinkedIn')


class WikipediaPage(ToughPinchZoomCasesPage):

  """ Why: #6 (Alexa) most visited worldwide,Picked an interesting page """

  def __init__(self, page_set):
    super(WikipediaPage, self).__init__(
      url='http://en.wikipedia.org/wiki/Wikipedia',
      page_set=page_set, name='Wikipedia (1 tab)')


class TwitterPage(ToughPinchZoomCasesPage):

  """ Why: #8 (Alexa global),Picked an interesting page """

  def __init__(self, page_set):
    super(TwitterPage, self).__init__(
      url='https://twitter.com/katyperry',
      page_set=page_set, name='Twitter')

  def RunNavigateSteps(self, action_runner):
    super(TwitterPage, self).RunNavigateSteps(action_runner)
    action_runner.Wait(2)

class ESPNPage(ToughPinchZoomCasesPage):

  """ Why: #1 sports """

  def __init__(self, page_set):
    super(ESPNPage, self).__init__(
      url='http://espn.go.com/nba',
      page_set=page_set, name='ESPN')


class WeatherDotComPage(ToughPinchZoomCasesPage):

  """ Why: #7 (Alexa news); #27 total time spent,Picked interesting page """

  def __init__(self, page_set):
    super(WeatherDotComPage, self).__init__(
      # pylint: disable=C0301
      url='http://www.weather.com/weather/right-now/Mountain+View+CA+94043',
      page_set=page_set, name='Weather.com')


class YahooGamePage(ToughPinchZoomCasesPage):

  """ Why: #1 games according to Alexa (with actual games in it) """

  def __init__(self, page_set):
    super(YahooGamePage, self).__init__(
      url='http://games.yahoo.com',
      page_set=page_set)

  def RunNavigateSteps(self, action_runner):
    super(YahooGamePage, self).RunNavigateSteps(action_runner)
    action_runner.Wait(2)


class YahooAnswersPage(ToughPinchZoomCasesPage):

  """ Why: #1 Alexa reference """

  def __init__(self, page_set):
    super(YahooAnswersPage, self).__init__(
      url='http://answers.yahoo.com',
      page_set=page_set)

  def RunPageInteractions(self, action_runner):
    with action_runner.CreateGestureInteraction('PinchAction'):
      action_runner.PinchElement(selector='#ya-content-apps')


class ToughPinchZoomCasesPageSet(story.StorySet):

  """ Set of pages that are tricky to pinch-zoom """

  def __init__(self):
    super(ToughPinchZoomCasesPageSet, self).__init__(
      archive_data_file='data/tough_pinch_zoom_cases.json',
      cloud_storage_bucket=story.PARTNER_BUCKET)

    self.AddStory(GoogleSearchPage(self))
    self.AddStory(GmailPage(self))
    self.AddStory(GoogleCalendarPage(self))
    self.AddStory(GoogleImageSearchPage(self))
    self.AddStory(GooglePlusPage(self))
    self.AddStory(YoutubePage(self))
    self.AddStory(BlogSpotPage(self))
    self.AddStory(FacebookPage(self))
    self.AddStory(LinkedinPage(self))
    self.AddStory(WikipediaPage(self))
    self.AddStory(TwitterPage(self))
    self.AddStory(ESPNPage(self))

    # Why: #1 news worldwide (Alexa global)
    self.AddStory(ToughPinchZoomCasesPage('http://news.yahoo.com', self))

    # Why: #2 news worldwide
    self.AddStory(ToughPinchZoomCasesPage('http://www.cnn.com', self))

    self.AddStory(WeatherDotComPage(self))

    # Why: #1 world commerce website by visits; #3 commerce in the US by time
    # spent
    self.AddStory(ToughPinchZoomCasesPage('http://www.amazon.com', self))

    # Why: #1 commerce website by time spent by users in US
    self.AddStory(ToughPinchZoomCasesPage('http://www.ebay.com', self))

    self.AddStory(YahooGamePage(self))

    # Why: #1 Alexa recreation
    self.AddStory(ToughPinchZoomCasesPage('http://booking.com', self))

    self.AddStory(YahooAnswersPage(self))

    # Why: #1 Alexa sports
    self.AddStory(ToughPinchZoomCasesPage('http://sports.yahoo.com/', self))

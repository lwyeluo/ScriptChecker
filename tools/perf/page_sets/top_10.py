# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from telemetry.page import page as page_module
from telemetry.page import shared_page_state
from telemetry import story


class SimplePage(page_module.Page):
  def __init__(self, url, page_set, name=''):
    if name == '':
      name = url
    super(SimplePage, self).__init__(
        url, page_set=page_set, name=name,
        shared_page_state_class=shared_page_state.SharedDesktopPageState)

  def RunPageInteractions(self, action_runner):
    pass


class Google(SimplePage):
  def __init__(self, page_set):
    super(Google, self).__init__(
      url='https://www.google.com/#hl=en&q=barack+obama', page_set=page_set)

  def RunNavigateSteps(self, action_runner):
    super(Google, self).RunNavigateSteps(action_runner)
    action_runner.WaitForElement(text='Next')


class Gmail(SimplePage):
  def __init__(self, page_set):
    super(Gmail, self).__init__(
      url='https://mail.google.com/mail/',
      page_set=page_set)

  def RunNavigateSteps(self, action_runner):
    super(Gmail, self).RunNavigateSteps(action_runner)
    action_runner.WaitForJavaScriptCondition(
        'window.gmonkey !== undefined &&'
        'document.getElementById("gb") !== null')


class GoogleCalendar(SimplePage):
  def __init__(self, page_set):
    super(GoogleCalendar, self).__init__(
      url='https://www.google.com/calendar/',
      page_set=page_set)

  def RunNavigateSteps(self, action_runner):
    super(GoogleCalendar, self).RunNavigateSteps(action_runner)
    action_runner.ExecuteJavaScript('''
        (function() { var elem = document.createElement("meta");
          elem.name="viewport";
          elem.content="initial-scale=1";
          document.body.appendChild(elem);
        })();''')
    action_runner.Wait(2)
    action_runner.WaitForElement('div[class~="navForward"]')


class Youtube(SimplePage):
  def __init__(self, page_set):
    super(Youtube, self).__init__(
      url='https://www.youtube.com',
      page_set=page_set)

  def RunNavigateSteps(self, action_runner):
    super(Youtube, self).RunNavigateSteps(action_runner)
    action_runner.Wait(2)


class Facebook(SimplePage):
  def __init__(self, page_set):
    super(Facebook, self).__init__(
      url='https://www.facebook.com/barackobama',
      page_set=page_set,
      name='Facebook')

  def RunNavigateSteps(self, action_runner):
    super(Facebook, self).RunNavigateSteps(action_runner)
    action_runner.WaitForElement(text='About')

class Economist(SimplePage):
  def __init__(self, page_set):
    super(Economist, self).__init__(
      url='https://www.economist.com/',
      page_set=page_set,
      name='Economist')

  def RunNavigateSteps(self, action_runner):
    super(Economist, self).RunNavigateSteps(action_runner)
    action_runner.Wait(2)
    action_runner.WaitForElement('section[class="layout-economist-today"]')


class Top10PageSet0(story.StorySet):
  """10 Pages chosen from Alexa top sites"""

  def __init__(self):
    super(Top10PageSet0, self).__init__(
      archive_data_file='data/top_10.json',
      cloud_storage_bucket=story.PARTNER_BUCKET)

    #self.AddStory(SimplePage('https://www.google.com/', self, name='Google'))
    #self.AddStory(SimplePage('https://www.facebook.com/', self, name='Facebook'))
    #self.AddStory(SimplePage('https://www.youtube.com/', self, name='Youtube'))
    #self.AddStory(SimplePage('https://www.microsoft.com/zh-cn/', self, name='Microsoft'))
    #self.AddStory(SimplePage('https://www.netflix.com/', self, name='Netflix'))
    #self.AddStory(SimplePage('https://twitter.com/explore', self, name='Twitter'))
    #self.AddStory(SimplePage('https://www.tmall.com/', self, name='TMall'))
    #self.AddStory(SimplePage('https://www.instagram.com/', self, name='Instagram'))
    #self.AddStory(SimplePage('https://www.qq.com/?fromdefault', self, name='QQ'))
    #self.AddStory(SimplePage('https://www.linkedin.com/', self, name='Linkedin'))


    self.AddStory(Google(self))
    self.AddStory(SimplePage('https://www.yelp.com/', self, name='yelp'))
    self.AddStory(SimplePage('https://www.eurosport.com/', self, name='eurosport'))
    self.AddStory(SimplePage('https://www.reddit.com/', self, name='reddit'))
    self.AddStory(SimplePage('https://www.legacy.com/', self, name='leagacy'))
    self.AddStory(SimplePage('https://www.twitch.tv/', self, name='twitch'))
    self.AddStory(SimplePage('https://www.amazon.com/', self, name='amazon'))
    self.AddStory(SimplePage('https://seatguru.com/', self, name='seatguru'))


class Top10PageSet1(story.StorySet):
  """10 Pages chosen from Alexa top sites"""

  def __init__(self):
    super(Top10PageSet1, self).__init__(
      archive_data_file='data/top_10.json',
      cloud_storage_bucket=story.PARTNER_BUCKET)

    self.AddStory(SimplePage('https://www.wowprogress.com/', self, name='wowprogress'))
    self.AddStory(SimplePage('https://www.espn.com/', self, name='espn'))
    self.AddStory(Economist(self))

class Top10PageSet(story.StorySet):
  """10 Pages chosen from Alexa top sites"""

  def __init__(self):
    super(Top10PageSet, self).__init__(
      archive_data_file='data/top_10.json',
      cloud_storage_bucket=story.PARTNER_BUCKET)

    # top google property; a google tab is often open
    #self.AddStory(Google(self))

    # productivity, top google properties
    # TODO(dominikg): fix crbug.com/386152
    #self.AddStory(Gmail(self))

    # productivity, top google properties
    #self.AddStory(GoogleCalendar(self))

    # #3 (Alexa global)
    #self.AddStory(Youtube(self))

    # #3 Tmall
    self.AddStory(SimplePage('https://www.tmall.com/', self, name='Tmall'))

    # top social, Public profile
    #self.AddStory(Facebook(self))

    # #5 Baidu
    self.AddStory(SimplePage('https://www.baidu.com/', self, name='Baidu'))

    # #6 QQ
    self.AddStory(SimplePage('https://www.qq.com/?fromdefault', self, name='Qq'))

    # #7 Sohu
    self.AddStory(SimplePage('http://sohu.com/', self, name='Sohu'))

    # #8 TaoBao
    self.AddStory(SimplePage('https://www.taobao.com/', self, name='Taobao'))

    # #10 (Alexa) most visited worldwide,Picked an interesting page
    #self.AddStory(SimplePage('http://en.wikipedia.org/wiki/Wikipedia',
    #                              self, name='Wikipedia'))

    # #11 world commerce website by visits; #3 commerce in the US by time spent
    self.AddStory(SimplePage('http://www.amazon.com', self, name='Amazon'))

    # #4 Alexa
    # self.AddStory(SimplePage('http://www.yahoo.com/', self))

    # #16 Alexa
    # self.AddStory(SimplePage('http://www.bing.com/', self))

    # #20 Alexa
    # self.AddStory(SimplePage('http://www.ask.com/', self))

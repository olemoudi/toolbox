from scrapy.contrib.spiders import CrawlSpider, Rule
from scrapy.contrib.linkextractors.sgml import SgmlLinkExtractor
from scrapy.selector import HtmlXPathSelector
from chistes.items import ChistesItem

class MySpider(CrawlSpider):
    name = 'mediavida.com'
    allowed_domains = ['mediavida.com']
    start_urls = ['http://www.mediavida.com/foro/90/chistes-malisisisimos-313416']

    rules = (
        Rule(SgmlLinkExtractor(allow=('http://www.mediavida.com/foro/90/chistes-malisisisimos-313416/(\d){1,3}')), callback='parse_page', follow=True),

    )

    def parse_page(self, response):
        hxs = HtmlXPathSelector(response)
        posts = hxs.select('//div[contains(@class, "post ") and not(contains(@id, "postit"))]')

        for post in posts:
            chiste = ChistesItem()
            
            chiste['autor'] = '\n'.join(post.select('.//div[contains(@class, "autor_small")]/strong/a/text()').extract())
            chiste['chiste'] = '\n'.join(post.select('.//div[@class="msg"]//div[@class="cuerpo"]/text()').extract())
            chiste['manitas'] = '\n'.join(post.select('.//*[@title="Usuarios a los que les gusta este mensaje"]/text()').extract())
            chiste['link'] = str(response.url) +  str('\n'.join(post.select('.//div[@class="autor_small clearfix"]/div[@class="info"]/a[@class="qn"]/text()').extract()))
            if len(chiste['autor']) == 0:
                continue
            else:
                yield chiste


